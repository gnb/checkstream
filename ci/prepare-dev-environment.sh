#!/bin/bash

set -x
host_os=$(uname -s)

case "$host_os" in
Linux)
    if [ -f /etc/ubuntu-release ] ; then
        sudo apt-get install \
            autoconf automake make gcc
    elif [ -f /etc/redhat-release ] ; then
        sudo yum install -y \
            autoconf automake make
    fi
    ;;
Darwin)
    echo "nothing to see here"
    ;;
esac
