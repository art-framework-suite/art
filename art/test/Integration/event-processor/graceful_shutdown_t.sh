#!/bin/bash

function usage() {
    echo "usage: ${0##*/} -c <art-config-file>" 1>&2
}

while getopts :c: OPT; do
    case $OPT in
        c)
            config="$OPTARG"
            ;;
        *)
            usage
            exit 2
    esac
done
shift $(( OPTIND - 1 ))
OPTIND=1

[[ -n "$config" ]] || { echo "Config file required." 1>&2; usage; exit 1; }

TMP=`mktemp -t graceful_shutdown_t.sh.XXXXXXXXXX`
trap "[[ -n \"$TMP\" ]] && rm $TMP* 2>/dev/null" EXIT

# Start art
art --rethrow-all -c "$config" --trace -n 10 --timing >"$TMP"
cat "$TMP"
