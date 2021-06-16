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

# 8 bytes is the smallest working length for a stream
init_subtests \
    8 16 128 512 1K 4K 32K 128K 1M

function size_to_bytes()
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

function tearDown()
{
    /bin/rm -f tbasic.$SUBTEST.dat
}

function tbasic()
{
    f=tbasic.$SUBTEST.dat
    /bin/rm -f $f
    $GENSTREAM $SUBTEST $f || fail "genstream failed"
    [ -f $f ] || fail "file $f doesn't exist"
    [ `file_size $f` = `size_to_bytes $SUBTEST` ]  || fail "size of file $f is unexpected"
    $CHECKSTREAM $f || fail "checkstream failed"
}

while next_subtest -D tearDown tbasic ; do
    :
done
