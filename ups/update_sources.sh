#!/bin/bash


# ======================================================================
# Prepare:

TOP=ART # adjust to the top of your hierarchy


# ======================================================================
# Run scripts to update

for F in `find $TOP/ -name "*.h" -o -name "*.cc" -o -name "*.cpp"`; do
  echo $F
  ed $F < pop.ed > /dev/null 2>&1
  ed $F < movedfile_20100901_112607.ed > /dev/null 2>&1
done
