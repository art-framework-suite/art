#!/bin/bash

# Creates 'b.root' and 'c.root' with the following shapes by inverting
# the process names for the "b" and "c" files.
#
#          b.root   c.root 
#          ======   ======
# Runs   : p1: r1   p1: r2
#          p2: r2   p2: r1
# SubRuns: p1: s1   p1: s2
#          p2: s2   p2: s1
# Events : p1: b1   p1: b2
#          p2: b2   p2: b1

# process 1
art -c make_input_producer.fcl --process-name=p1 -o "b1.root" -n 10
art -c make_input_producer.fcl --process-name=p2 -o "c2.root" -n 10

# process 2
art -c make_input_producer.fcl --process-name=p2 -o "b.root" -s "b1.root" -n 10
art -c make_input_producer.fcl --process-name=p1 -o "c.root" -s "c2.root" -n 10

rm b1.root c2.root
