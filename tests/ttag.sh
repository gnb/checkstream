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
    /bin/rm -f ttag.*.dat
}

function testTag()
{
    size=4096
    f=ttag.notag.dat
    f1=ttag.T1.dat
    f2=ttag.T2.dat
    /bin/rm -f $f $f1 $f2

    assert_success $GENSTREAM $size $f
    assert_file_exists $f
    assert_file_size_equals $f $size

    assert_success $GENSTREAM -T1 $size $f1
    assert_file_exists $f1
    assert_file_size_equals $f1 $size

    assert_success $GENSTREAM -T2 $size $f2
    assert_file_exists $f2
    assert_file_size_equals $f2 $size

    assert_success $CHECKSTREAM $f
    assert_logged "valid data for $size bytes at offset 0"
    assert_failure $CHECKSTREAM -T1 $f
    assert_logged "bad tag for $size bytes at offset 0"
    assert_failure $CHECKSTREAM -T2 $f
    assert_logged "bad tag for $size bytes at offset 0"

    assert_failure $CHECKSTREAM $f1
    assert_logged "bad offset for $size bytes at offset 0"
    assert_success $CHECKSTREAM -T1 $f1
    assert_logged "valid data for $size bytes at offset 0"
    assert_failure $CHECKSTREAM -T2 $f1
    assert_logged "bad tag for $size bytes at offset 0"

    assert_failure $CHECKSTREAM $f2
    assert_logged "bad offset for $size bytes at offset 0"
    assert_failure $CHECKSTREAM -T1 $f2
    assert_logged "bad tag for $size bytes at offset 0"
    assert_success $CHECKSTREAM -T2 $f2
    assert_logged "valid data for $size bytes at offset 0"
}

run_subtests
