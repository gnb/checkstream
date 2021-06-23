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
    /bin/rm -f tmmap.*.dat
}

function testMmap()
{
    size=16384
    f=tmmap.noM.dat
    fM=tmmap.M.dat
    /bin/rm -f $f $fM

    assert_success $GENSTREAM $size $f
    assert_file_exists $f
    assert_file_size_equals $f $size

    assert_success $GENSTREAM -M $size $fM
    assert_file_exists $fM
    assert_file_size_equals $fM $size

    # Writing with mmap makes no difference to the format

    assert_success $CHECKSTREAM $f
    assert_success $CHECKSTREAM -M $f

    assert_success $CHECKSTREAM $fM
    assert_success $CHECKSTREAM -M $fM
}

run_subtests
