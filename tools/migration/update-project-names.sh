#!/bin/bash
# Program name
prog=${0##*/}
# ======================================================================
function usage() {
    cat 1>&2 <<EOF
usage: $prog [--one-file <file>] <top-dir>
EOF
}

function one_file() {
  local F=$1
  printf "\n $F ... "
  # Fix Project Names
  #perl -wapi\~ -f fix-project-names.pl "${F}" >/dev/null 2>&1 && rm -f "${F}~"
  perl -wapi\~ -f fix-project-names.pl "${F}" 
}

# ======================================================================
# Prepare:
TEMP=`getopt -n "$prog" -o a --long all-lumi-cases --long one-file: -- "${@}"`
eval set -- "$TEMP"
while true; do
  case $1 in
    --one-file)
      file=$2
      shift 2
      ;;
    --)
      shift
      break
      ;;
    *)
      echo "Bad argument \"$OPT\"" 1>&2
      usage
      exit 1
    esac
done

TOP=${1:-ART}

# ======================================================================
# Run scripts to update

TMP=`mktemp -t update_sources.sh.XXXXXX`
trap "rm $TMP* 2>/dev/null" EXIT

if [[ -n "${file}" ]]; then
  if ! [[ -r "${file}" ]]; then
    echo "ERROR: ${file} does not exist or is not readable." 1>&2
    exit 1
  else
    one_file "$file"
  fi
else
  for F in `find $TOP -name CMakeLists.txt -print`; do
    one_file "$F"
  done
fi
