#!/bin/bash
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

function _params_for_subtest()
{
    local subtest="$1"
    local var=$(declare | grep "^param_$subtest=" | cut -d= -f1)
    if [ -n "$var" ] ; then
        eval 'echo $'$var
    fi
}

function run_subtests()
{
    local setupfunc=$(_list_shell_functions | egrep '^(setup|setUp)' | head -1)
    local teardownfunc=$(_list_shell_functions | egrep '^(teardown|tearDown)' | head -1)
    local params=

    SUBTESTS=$(_list_shell_functions | grep '^test')

    # count the subtest instances and emit the TAP header
    n=0
    for st in $SUBTESTS ; do
        params=$(_params_for_subtest $st)
        if [ -z "$params" ] ; then
            n=$[n+1]
        else
            for p in $params ; do
                n=$[n+1]
            done
        fi
    done
    echo "1..$n"

    # run all the subtests in order
    SUBTEST_INDEX=1
    for testfunc in $SUBTESTS ; do
        params=$(_params_for_subtest $testfunc)
        if [ -z "$params" ] ; then
            SUBTEST=$testfunc
            _run_subtest "$setupfunc" "$testfunc" "$teardownfunc"
            SUBTEST_INDEX=$[SUBTEST_INDEX+1]
        else
            for p in $params ; do
                SUBTEST="$testfunc.$p"
                _run_subtest "$setupfunc" "$testfunc $p" "$teardownfunc"
                SUBTEST_INDEX=$[SUBTEST_INDEX+1]
            done
        fi
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

function assert_file_exists()
{
    local f="$1"
    [ -f "$f" ] || fail "file $f doesn't exist"
}

function _size_to_bytes()
{
    case "$1" in
    *[kK]) echo $[ ${1%[kK]} << 10 ] ;;
    *[mM]) echo $[ ${1%[mM]} << 20 ] ;;
    *[gG]) echo $[ ${1%[gG]} << 30 ] ;;
    *[tT]) echo $[ ${1%[tT]} << 40 ] ;;
    *[pP]) echo $[ ${1%[pP]} << 50 ] ;;
    *) echo "$1" ;;
    esac
}

function assert_file_size_equals()
{
    local f="$1"
    local expected_size=$(_size_to_bytes "$2")
    local actual_size=$(file_size "$f")
    [ "$actual_size" = "$expected_size" ]  || fail "size of file $f is $actual_size, expecting $expected_size"
}

function assert_logged()
{
    local msg="$*"

    fgrep "$msg" $_SUBTEST_LOG > /dev/null || fail "log doesn't contain string \"$msg\""
}

GENSTREAM="../genstream"
CHECKSTREAM="../checkstream"

