#!/bin/bash

set -x
host_os=$(uname -s)

case "$host_os" in
Linux)
    case "$(lsb_release -i)" in
    Ubuntu*)
        sudo apt-get install autoconf automake make
        ;;
    Fedora*)
        sudo yum install -y autoconf automake make
        ;;
    esac
    ;;
Darwin)
    echo "nothing to see here"
    ;;
esac
