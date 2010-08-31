#!/bin/bash


# ======================================================================
# Prepare:

TOP=art_v0_0_0 # adjust to be the top of your hierarchy


# ======================================================================
# Create initial editor script to remove CVS-isms & trailing whitespace:

EDFILE=pop.ed

for F in `find $TOP/ -name "*.h" -o -name "*.cc" -o -name "*.cpp"`; do
  echo $F
  ed $F < $EDFILE > /dev/null 2>&1
done
