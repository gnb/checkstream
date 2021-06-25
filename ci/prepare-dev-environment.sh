#!/bin/bash

set -x
host_os=$(uname -s)

function fatal()
{
    echo "$0: $*" 1>&2
    exit 1
}

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
    sudo apt-get update || fatal "apt-get update failed"
    sudo apt-get install -y autoconf automake make gcc  || fatal "apt-get install failed"
}

function setup_redhat()
{
    sudo yum install -y autoconf automake make gcc
}

case "$host_os" in
Linux)
    if is_ubuntu ; then
        setup_ubuntu
    elif is_redhat ; then
        setup_redhat
    else
        fatal "Unknown Linux distro"
    fi
    ;;
Darwin)
    echo "nothing to see here"
    ;;
esac
