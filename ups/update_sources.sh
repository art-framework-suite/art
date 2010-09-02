#!/bin/bash


# ======================================================================
# Prepare:

TOP=${1:-ART}


# ======================================================================
# Run scripts to update

for F in `find $TOP \( -type d \( -name .git -o -name .svn -o -name CVS -o -name ups \) \
                       -prune \) -o -type f \! \( -name '*~' -o -name '*.bak' -o -name '*.new' \) -print`; do
  printf "$F ... "
  # Fix most includes
  ed "$F" < pop.ed > /dev/null 2>&1
  # Account for one additional moved file
  ed "$F" < movedfile_20100901_112607.ed > /dev/null 2>&1
  # Fix includes in and of .icc files
  perl -wapi\~ -f fix-icc-includes.pl "${F}" >/dev/null 2>&1 && rm -f "${F}~"
  # "lumi|luminosty|luminosityblock" -> subrun
  grep -Iil subrun "${F}" && { echo "OK"; continue; } # Already done
  err=$(perl -wpi\~ -f fix-lumi.pl "${F}" 2>&1 >/dev/null)
  if (( $? )); then # Oops
    echo "PROBLEM: fix manually!" 1>&2
    continue
  else
    # Remove backup
    rm -f "${F}~"
  fi
  
  # Lumifix for file name
  Fnew=$(echo "$F" | perl -wp -f fix-lumi.pl 2>/dev/null) 
  if [[ -n "$Fnew" ]] && [[ "$Fnew" != "$F" ]]; then
      if [[ -e "$Fnew" ]]; then # Oops
          echo "Unable to rename \"$F\" to already-existing file \"$Fnew\"" 1>&2
      fi
  fi
  echo "OK"
done
