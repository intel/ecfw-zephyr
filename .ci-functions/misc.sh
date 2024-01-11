# SPDX-License-Identifier: Apache-2.0
#!/bin/bash

end="\033[0m"
black="\033[0;30m"
blackb="\033[1;30m"
white="\033[0;37m"
whiteb="\033[1;37m"
red="\033[0;31m"
redb="\033[1;31m"
green="\033[0;32m"
greenb="\033[1;32m"
yellow="\033[0;33m"
yellowb="\033[1;33m"
blue="\033[0;34m"
blueb="\033[1;34m"
purple="\033[0;35m"
purpleb="\033[1;35m"
lightblue="\033[0;36m"
lightblueb="\033[1;36m"

function info() {
    echo -e "${lightblue}${1}${end}"
}

function warning() {
    echo -e "${yellow}${1}${end}"
}

function error() {
    echo -e "${red}${1}${end}"
}

function debug() {
    echo -e "${green}${1}${end}"
}
