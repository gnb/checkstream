#/bin/bash
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

. common.sh

init_subtests 1

function tearDown()
{
    /bin/rm -f ttag.*.dat
}

function ttag()
{
    size=4096
    f=ttag.notag.dat
    f1=ttag.T1.dat
    f2=ttag.T2.dat
    /bin/rm -f $f $f1 $f2

    assert_success $GENSTREAM $size $f
    [ -f $f ] || fail "file $f doesn't exist"
    [ `file_size $f` = $size ]  || fail "size of file $f is unexpected"

    assert_success $GENSTREAM -T1 $size $f1
    [ -f $f1 ] || fail "file $f1 doesn't exist"
    [ `file_size $f1` = $size ]  || fail "size of file $f1 is unexpected"

    assert_success $GENSTREAM -T2 $size $f2
    [ -f $f2 ] || fail "file $f2 doesn't exist"
    [ `file_size $f2` = $size ]  || fail "size of file $f2 is unexpected"

    assert_success $CHECKSTREAM $f
    assert_failure $CHECKSTREAM -T1 $f
    assert_failure $CHECKSTREAM -T2 $f

    assert_failure $CHECKSTREAM $f1
    assert_success $CHECKSTREAM -T1 $f1
    assert_failure $CHECKSTREAM -T2 $f1

    assert_failure $CHECKSTREAM $f2
    assert_failure $CHECKSTREAM -T1 $f2
    assert_success $CHECKSTREAM -T2 $f2
}

while next_subtest -D tearDown ttag ; do
    :
done
