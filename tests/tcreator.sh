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
    /bin/rm -f tcreator.*.dat
}

function tcreator()
{
    size=4096
    f=tcreator.noC.dat
    fC=tcreator.C.dat
    fC2=tcreator.C2.dat
    /bin/rm -f $f $fC $fC2

    assert_success $GENSTREAM $size $f
    [ -f $f ] || fail "file $f doesn't exist"
    [ `file_size $f` = $size ]  || fail "size of file $f is unexpected"

    assert_success $GENSTREAM -C $size $fC
    [ -f $fC ] || fail "file $fC doesn't exist"
    [ `file_size $fC` = $size ]  || fail "size of file $fC is unexpected"

    assert_success $GENSTREAM -C $size $fC2
    [ -f $fC2 ] || fail "file $fC2 doesn't exist"
    [ `file_size $fC2` = $size ]  || fail "size of file $fC2 is unexpected"

    assert_success $CHECKSTREAM $f
    assert_failure $CHECKSTREAM -C $f

    assert_failure $CHECKSTREAM $fC
    assert_success $CHECKSTREAM -C $fC

    assert_failure $CHECKSTREAM $fC2
    assert_success $CHECKSTREAM -C $fC2

    dd if=$fC of=$fC2 conv=notrunc bs=16 count=4
    assert_failure $CHECKSTREAM -C $fC2
}

while next_subtest -D tearDown tcreator ; do
    :
done
