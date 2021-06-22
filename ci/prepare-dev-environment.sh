#!/bin/bash

set -x
host_os=$(uname -s)

function is_ubuntu()
{
    grep Ubuntu /etc/issue > /dev/null 2>&1
}

function is_redhat()
{
    [ -f /etc/redhat-release ]
}

case "$host_os" in
Linux)
    if is_ubuntu ; then
        sudo apt-get install \
            autoconf automake make gcc
    elif is_redhat ; then
        sudo yum install -y \
            autoconf automake make
    fi
    ;;
Darwin)
    echo "nothing to see here"
    ;;
esac
