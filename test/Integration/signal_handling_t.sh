#!/bin/bash

function usage() {
    echo "usage: ${0##*/} -c <art-config-file> [--] signal..." 1>&2
}

function art_running() {
  [[ -n "$art_pid" ]] || { echo "art never started!" 1>&2; return 1; }
  kill -0 $art_pid
  return $?
}

function print_and_finish() {
  [[ -n "$TMP" ]] && [[ -f "$TMP" ]] && cat "$TMP"
  exit $1
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

sig="$1"

[[ -n "$config" ]] || { echo "Config file required.\n" 1>&2; usage; exit 1; }

[[ -n "$sig" ]] || { echo "Signal spec required.\n" 1>&2; usage; exit 1; }

TMP=`mktemp -t signal_handling_t.sh.XXXXXXXXXX`
trap "[[ -n \"$TMP\" ]] && rm $TMP* 2>/dev/null" EXIT

# Start art
art -c "$config" 2>"$TMP" &

(( art_pid = $! ))

# Pause for init
while art_running; do
  grep -e ">> Pausing for" "$TMP" >/dev/null 2>&1 && break
  sleep 1;
done

# Check we're still running
cat $TMP
if art_running && grep -e ">> Pausing for" "$TMP" >/dev/null 2>&1; then
  kill -$sig $art_pid
  wait $art_pid
  print_and_finish $?
else 
  wait $art_pid
  print_and_finish 1
fi
