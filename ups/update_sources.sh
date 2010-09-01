#!/bin/bash


# ======================================================================
# Prepare:

TOP=${1:-ART}


# ======================================================================
# Run scripts to update

for F in `find $TOP \( -type d \( -name .git -o -name .svn -o -name CVS \) \
                       -prune \) -o -type f -print`; do
  echo "$F"
  ed "$F" < pop.ed > /dev/null 2>&1
  ed "$F" < movedfile_20100901_112607.ed > /dev/null 2>&1
  perl -wapi\~ -f fix-icc-includes.pl "${F}" >/dev/null 2>&1 && rm -f "${F}~"
done
