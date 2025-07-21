#!/bin/bash

if [[ "$1" == "show" || "$1" == "tree" ]]; then

    pattern='^\.+ (engine|src|include)'

    gpp_command=$(make tree=y -n | grep -m 1 '^g++')
    if [[ -z "$gpp_command" ]]; then
        echo "Error: Could not find the g++ command in Makefile output." >&2
        exit 1
    fi

    echo -e "\n[Running]: $gpp_command"
    eval "$gpp_command" 2>&1 | grep -E "$pattern"

    rm -f *.o

elif [[ "$1" == "debug" ]]; then
    make debug=y
else
    make
fi