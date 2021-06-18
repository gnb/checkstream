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

function tearDown()
{
    /bin/rm -f tcheckerrors.*.dat
}

function testShort()
{
    f8=tcheckerrors.short8.dat
    f7=tcheckerrors.short7.dat

    assert_success $GENSTREAM 8 $f8
    [ -f $f8 ] || fail "file $f8 doesn't exist"
    [ `file_size $f8` = 8 ]  || fail "size of file $f8 is unexpected"

    dd if=$f8 of=$f7 bs=1 count=7

    assert_success $CHECKSTREAM $f8
    assert_failure $CHECKSTREAM $f7
    assert_logged "file too short: must be at least 8 bytes long"
}


function testUnaligned()
{
    fA=tcheckerrors.aligned.dat
    fU=tcheckerrors.unaligned.dat

    assert_success $GENSTREAM 16 $fA
    [ -f $fA ] || fail "file $fA doesn't exist"
    [ `file_size $fA` = 16 ]  || fail "size of file $fA is unexpected"

    dd if=$fA of=$fU bs=1 count=15

    assert_success $CHECKSTREAM $fA
    assert_success $CHECKSTREAM $fU
    assert_logged "warning: unaligned file length (will not check last 7 bytes)"
}


function testZeroRecord()
{
    f=tcheckerrors.zerorecord.dat

    for offset in 0 8 16 128 1016 ; do

        assert_success $GENSTREAM 1024 $f
        [ -f $f ] || fail "file $f doesn't exist"
        [ `file_size $f` = 1024 ]  || fail "size of file $f is unexpected"

        assert_success $CHECKSTREAM $f

        dd if=/dev/zero of=$f bs=1 count=8 conv=notrunc seek=$offset

        assert_failure $CHECKSTREAM $f
        assert_logged "zero data for 8 bytes at offset $offset"
    done
}

run_subtests
