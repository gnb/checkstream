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

function sudo()
{
    local binsudo=
    [ -x /bin/sudo ] && binsudo=/bin/sudo
    $binsudo "$@"
}

function setup_ubuntu()
{
    set -x
    sudo apt-get install autoconf automake make gcc
}

function setup_redhat()
{
    set -x
    sudo yum install -y autoconf automake make gcc
}

case "$host_os" in
Linux)
    if is_ubuntu ; then
        setup_ubuntu
    elif is_redhat ; then
        setup_redhat
    fi
    ;;
Darwin)
    echo "nothing to see here"
    ;;
esac
