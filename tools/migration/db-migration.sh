#!/bin/bash

function set_file_names() {
    file=$1
    base=${file%.db}
    new_file=${base}_new.db
    if ! [ -z "$2" ]
    then
        new_file=$2
    fi
    temp_file=$(mktemp /tmp/timeTracker-migration-script.XXXXXX)
}

function validate_new_file() {
    if [ -e $1 ]
    then
        echo "New file (\"$1\") already exists."
        echo "Please supply different new-file name or remove it."
        exit 2
    fi
}
