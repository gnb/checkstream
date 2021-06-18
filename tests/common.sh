#
# Tests Copyright (c) 2021 Greg Banks <gnb@fmeh.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# Shell functions for test scripts
#

SUBTESTS=
_RESULT=
SUBTEST_INDEX=
SUBTEST=
_IN_SUBSHELL=no
_EXIT_SKIP=77
_EXIT_HARDFAIL=99
_EXIT_PASS=0
_SUBTEST_LOG="subtest.log"

_HOST_OS=$(uname -s)

# automake allows us to mark a test as hard failed using exit code 99
function hardfail()
{
    echo "$0: $*" 1>&2
    exit $_EXIT_HARDFAIL
}

function skip()
{
    [ -n "$SUBTEST_INDEX" ] || hardfail "skip() called without a subtest"
    echo "ok $SUBTEST_INDEX $SUBTEST # SKIP $*"
    [ $_IN_SUBSHELL = yes ] && exit $_EXIT_SKIP
}

function fail()
{
    [ -n "$SUBTEST_INDEX" ] || hardfail "fail() called without a subtest"
    echo "not ok $SUBTEST_INDEX - $SUBTEST # $*"
    [ $_IN_SUBSHELL = yes ] && exit 1
}

function pass()
{
    [ -n "$SUBTEST_INDEX" ] || hardfail "pass() called without a subtest"
    echo "ok $SUBTEST_INDEX - $SUBTEST"
    [ $_IN_SUBSHELL = yes ] && exit $_EXIT_PASS
}

function file_size()
{
    local file="$1"
    case "$_HOST_OS" in
    Linux) stat -c %s "$file" ;;
    Darwin) stat -f %z "$file" ;;
    *) hardfail "no implementation of file_size for this platform" ;;
    esac
}

function init_subtests()
{
    SUBTESTS="$*"
    echo "1..$#"
}

function _list_shell_functions()
{
    declare -F | awk '{print $3}' | LANG=C sort
}


function _run_subtest()
{
    local setupfunc="$1"
    local testfunc="$2"
    local teardownfunc="$3"

    [ -n "$setupfunc" ] && eval "$setupfunc"
    _RESULT=
    ( 
        _IN_SUBSHELL=yes
        eval "$testfunc"
        pass
    )
    case "$?" in
    $_EXIT_PASS) _RESULT=pass ;;
    $_EXIT_SKIP) _RESULT=skip ;;
    $_EXIT_HARDFAIL) exit $_EXIT_HARDFAIL ;;
    *) RESULT=fail ;;
    esac
    [ -n "$teardownfunc" ] && eval "$teardownfunc"
}

function next_subtest()
{
    local testfunc=
    local setupfunc=
    local teardownfunc=

    while [ $# -gt 0 ] ; do
        case "$1" in
        -U) setupfunc="$2" ; shift ;;
        -D) teardownfunc="$2" ; shift ;;
        -*) hardfail "Unknown option $1 to next_subtest" ;;
        *)
            [ -z "$testfunc" ] || hardfail "multiple test functions in next_subtest"
            testfunc="$1"
            ;;
        esac
        shift
    done

    set - $SUBTESTS
    [ $# -gt 0 ] || return 1
    SUBTEST="$1"
    if [ -z "$SUBTEST_INDEX" ] ; then
        SUBTEST_INDEX=1
    else
        SUBTEST_INDEX=$[SUBTEST_INDEX+1]
    fi
    shift
    SUBTESTS="$*"

    _run_subtest "$setupfunc" "$testfunc" "$teardownfunc"

    return 0
}

function run_subtests()
{
    local setupfunc=$(_list_shell_functions | egrep '^(setup|setUp)' | head -1)
    local teardownfunc=$(_list_shell_functions | egrep '^(teardown|tearDown)' | head -1)

    SUBTESTS=$(_list_shell_functions | grep '^test')

    # count the subtests and emit the TAP header
    n=0
    for t in $SUBTESTS ; do
        n=$[n+1]
    done
    echo "1..$n"

    # run all the tests in order
    SUBTEST_INDEX=1
    for SUBTEST in $SUBTESTS ; do
        _run_subtest "$setupfunc" $SUBTEST "$teardownfunc"
        SUBTEST_INDEX=$[SUBTEST_INDEX+1]
    done
}

function assert_success()
{
    set -o pipefail
    eval "$@ 2>&1 | tee $_SUBTEST_LOG" || fail "$* failed unexpectedly"
}

function assert_failure()
{
    set -o pipefail
    eval "$@ 2>&1 | tee $_SUBTEST_LOG" && fail "$* succeeded unexpectedly"
}

function assert_logged()
{
    local msg="$*"

    fgrep "$msg" $_SUBTEST_LOG > /dev/null || fail "log doesn't contain string \"$msg\""
}

GENSTREAM="../genstream"
CHECKSTREAM="../checkstream"

