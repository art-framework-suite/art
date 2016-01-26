#!/bin/bash

# This test takes two input files from a primary process and dumps
# their event numbers.  It then runs a no-merge job where the products
# in the primary files are persisted to some output files (the
# secondary files) that correspond exactly to the events in each file.
# The event IDs are dumped for the secondary files.  The event
# printout is diffed between primary and secondary files to ensure
# that there is a one-to-one correspondence of output files to input
# files.

# Dump info from first file
art -c eventIDPrinter.fcl -s ../ToyProductProducer_t_01.d/ToyProductProducer_t_01.root > primary_01.txt || exit 1
art -c eventIDPrinter.fcl -s ../ToyProductProducer_t_02.d/ToyProductProducer_t_02.root > primary_02.txt || exit 2

# Run no-merging process
art -c ToyProductNoMerger_t.fcl --rethrow-all || exit 3

art -c eventIDPrinter.fcl -s ToyProductNoMerger_t_01.root > secondary_01.txt || exit 4
art -c eventIDPrinter.fcl -s ToyProductNoMerger_t_02.root > secondary_02.txt || exit 5

diff {primary,secondary}_01.txt || exit 6
diff {primary,secondary}_02.txt || exit 7
