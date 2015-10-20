#!/bin/bash

# Creates 'a.root' and 'b.root' with the following shapes, where the
# 'b.root' shape is a subset of 'a.root':
#
#          a.root  b.root
#          ======  ======
# Runs   : p1: r1  p1: r1 
#                  p2: r2
# SubRuns: p1: s1  p1: s1
#                  p2: s2
# Events : p1: b1  p1: b1
#                  p2: b2
#

# process 1
art -c make_input_producer.fcl --process-name=p1 -o "a.root" -n 10
art -c make_input_producer.fcl --process-name=p1 -o "b1.root" -n 10

# process 2
art -c make_input_producer.fcl --process-name=p2 -o "b.root" -s "b1.root" -n 10

rm b1.root
