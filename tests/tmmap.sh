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
    /bin/rm -f tmmap.*.dat
}

function tmmap()
{
    size=16384
    f=tmmap.noM.dat
    fM=tmmap.M.dat
    /bin/rm -f $f $fM

    assert_success $GENSTREAM $size $f
    [ -f $f ] || fail "file $f doesn't exist"
    [ `file_size $f` = $size ]  || fail "size of file $f is unexpected"

    assert_success $GENSTREAM -M $size $fM
    [ -f $fM ] || fail "file $fM doesn't exist"
    [ `file_size $fM` = $size ]  || fail "size of file $fM is unexpected"

    # Writing with mmap makes no difference to the format

    assert_success $CHECKSTREAM $f
    assert_success $CHECKSTREAM -M $f

    assert_success $CHECKSTREAM $fM
    assert_success $CHECKSTREAM -M $fM
}

while next_subtest -D tearDown tmmap ; do
    :
done
