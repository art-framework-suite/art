#!/bin/bash

# Creates 'b.root' and 'c.root' with the following shapes:
#
#          b.root   d.root 
#          ======   ======
# Runs   : p1: r1   p1: r1
#          p2: r2   p2: r2
# SubRuns: p1: s1   p1: s1
#          p2: s2   p2: s2
# Events : p1: b1   p1: b1
#          p2: b2   p2: b3 <--- different here

# process 1
art -c make_input_producer.fcl --process-name=p1 -o "b1.root" -n 10
art -c make_input_producer.fcl --process-name=p1 -o "d1.root" -n 10

# process 2
art -c make_input_producer_different_shapes_06b.fcl --process-name=p2 -o "b.root" -s "b1.root" -n 10
art -c make_input_producer_different_shapes_06d.fcl --process-name=p2 -o "d.root" -s "d1.root" -n 10

rm b1.root d1.root
