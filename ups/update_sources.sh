#!/bin/bash
# Program name
prog=${0##*/}
# ======================================================================
function usage() {
    cat 1>&2 <<EOF
usage: $prog [-a|--all-lumi-cases] [--one-file <file>] <top-dir>
EOF
}

function one_file() {
  local F=$1
  printf "$F ... "
  # Fix most includes
  ed "$F" < pop.ed > /dev/null 2>&1
  # Account for one additional moved file
  ed "$F" < movedfile_20100901_112607.ed > /dev/null 2>&1
  # Fix includes in and of .icc files
  perl -wapi\~ -f fix-icc-includes.pl "${F}" >/dev/null 2>&1 && rm -f "${F}~"
  # "lumi|luminosty|luminosityblock" -> subrun
  grep -Il subrun "${F}" && { echo "OK"; return; } # Already done
  cp -p "${F}" "$TMP" # Make sure permissions are correct on temporary file
  err=$(perl -wp -f fix-lumi.pl "${F}" 2>&1 >"$TMP" ) # Yes, the redirections are in the right order.
  status=$?
  if (( $status )); then # Oops
    echo "PROBLEM: fix manually ($(echo \"$err\" | sed -e 's/.*FATAL: //' ))!" 1>&2
    rm -f "$TMP"
    return 1
  else
      # Success!
      mv -f "$TMP" "$F"
  fi
  
  # Lumi fix for file name
  local Fnew=$(echo "$F" | perl -wp -f fix-lumi.pl 2>/dev/null) 
  if [[ -n "$Fnew" ]] && [[ "$Fnew" != "$F" ]]; then
      if [[ -e "$Fnew" ]]; then # Oops
          echo "Unable to rename \"$F\" to already-existing file \"$Fnew\"" 1>&2
      else
          mv "$F" "$Fnew"
      fi
  fi
  echo "OK"
}

# ======================================================================
# Prepare:
TEMP=`getopt -n "$prog" -o a --long all-lumi-cases --long one-file: -- "${@}"`
eval set -- "$TEMP"
while true; do
  case $1 in
    -a|--all-lumi-cases)
      all_lumi=1
      shift
      ;;
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

(( ${all_lumi:-0} )) || export FIX_LUMI_CLASSES_ONLY=1
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
  for F in `find $TOP \( -type d \( -name .git -o -name .svn -o -name CVS -o -name ups \) \
                       -prune \) -o -type f \! \( -name '*~' -o -name '*.bak' -o -name '*.new' \) -print`; do
    one_file "$F"
  done
fi
