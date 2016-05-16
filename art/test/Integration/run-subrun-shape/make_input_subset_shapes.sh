#!/bin/bash

# Creates 'a.root' and 'b.root' with the following shapes, where the
# 'b.root' shape is a subset of 'a.root':
#
#          a.root  b.root
#          ======  ======
# Runs   : p1: r1  p1: r1 
#          p2: r2
# SubRuns: p1: s1  p1: s1
#          p2: s2
# Events : ------  ------
#

# process 1
art -c make_input_producer_noevents.fcl --process-name=p1 -o "a1.root" -n 10
art -c make_input_producer_noevents.fcl --process-name=p1 -o "b.root" -n 10

# process 2
art -c make_input_producer_noevents.fcl --process-name=p2 -o "a.root" -s "a1.root" -n 10

rm a1.root
