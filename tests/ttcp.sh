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

PORTFILE=ttcp.port
PIDFILE=ttcp.pid

function tearDown()
{
    local pid=$(cat $PIDFILE 2>/dev/null)
    if [ -n "$pid" ]; then
        local x=$(ps -lf -p "$pid" | grep checkstream)
        if [ -n "$x" ] ; then
            echo "Killing leftover checkstream process $pid"
            kill -TERM "$pid"
        fi
    fi
    /bin/rm -f $PORTFILE $PIDFILE
}

function wait_for_file()
{
    local file="$1"
    local timeout_secs="${2:-10}"
    local timeout_ms=$[ timeout_secs * 1000 ]
    local delay_ms=10
    local total_delay_ms=0
    echo "Waiting up to $timeout_secs seconds for $file to appear"
    while (( total_delay_ms < timeout_ms )) ; do
        if [ -f "$file" ] ; then
            printf "...file $file exists after %d.%03d seconds\n" $[ total_delay_ms / 1000 ] $[total_delay_ms % 1000 ]
            return 0
        fi
        echo "...sleeping $delay_ms milliseconds"
        sleep $(printf "%d.%03d" $[ delay_ms / 1000 ] $[ delay_ms % 1000 ])
        total_delay_ms=$[ total_delay_ms + delay_ms ]
        delay_ms=$[ delay_ms * 2 ]
    done
    return 1
}

#function testWaitForFile()
#{
#    /bin/rm -f $PORTFILE
#    /bin/rm -f $PIDFILE
#
#    ( sleep 5 ; echo "now!" ; echo foo > $PORTFILE ) &
#    echo $! > $PIDFILE
#    wait_for_file $PORTFILE
#    wait $(cat $PIDFILE)
#}

function testTCP()
{
    size=4096
    /bin/rm -f $PORTFILE
    /bin/rm -f $PIDFILE

    echo "Starting checkstream in server mode"
    ( $CHECKSTREAM --protocol tcp --length $size --port dynamic --port-filename $PORTFILE ) &
    echo $! > $PIDFILE
    echo "Wrote \""$(cat $PIDFILE)"\" to pid file $PIDFILE"
    wait_for_file $PORTFILE

    echo "Starting genstream in client mode"
    assert_success $GENSTREAM --protocol=tcp --port=$(cat $PORTFILE) $size localhost

    echo "Waiting for checkstream process"
    wait $(cat $PIDFILE)
    [ $? = 0 ] || fail "checkstream failed"
}

run_subtests
