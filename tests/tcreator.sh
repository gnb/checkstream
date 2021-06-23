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

. $PWD/common.sh

function tearDown()
{
    /bin/rm -f tcreator.*.dat
}

function testCreator()
{
    size=4096
    f=tcreator.noC.dat
    fC=tcreator.C.dat
    fC2=tcreator.C2.dat
    /bin/rm -f $f $fC $fC2

    assert_success $GENSTREAM $size $f
    assert_file_exists $f
    assert_file_size_equals $f $size

    assert_success $GENSTREAM -C $size $fC
    assert_file_exists $fC
    assert_file_size_equals $fC $size

    assert_success $GENSTREAM -C $size $fC2
    assert_file_exists $fC2
    assert_file_size_equals $fC2 $size

    assert_success $CHECKSTREAM $f
    assert_failure $CHECKSTREAM -C $f

    assert_failure $CHECKSTREAM $fC
    assert_success $CHECKSTREAM -C $fC

    assert_failure $CHECKSTREAM $fC2
    assert_success $CHECKSTREAM -C $fC2

    dd if=$fC of=$fC2 conv=notrunc bs=16 count=4
    assert_failure $CHECKSTREAM -C $fC2
}

run_subtests
