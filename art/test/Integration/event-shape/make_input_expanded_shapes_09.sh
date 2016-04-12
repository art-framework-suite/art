#!/bin/bash

# Creates 'a.root' and 'b.root' with the following shapes, where the
# 'b.root' shape is expanded wrt 'a.root', by doing a simple
# pass-though process, followed by a process that creates another
# product.
#
#
#          a.root   b.root
#          ======   ======
# Runs   : p1: r1   p1: r1
#          p2: r2   p2: r2
# SubRuns: p1: s1   p1: s1
#          p2: s2   p2: s2
# Events : p1: b1   p1: b1
#          p2: b2   p2: b2
#                   p3: [empty] - simple pass through
#                   p4: b4

# process 1
art -c make_input_producer.fcl --process-name=p1 -o "a1.root" -n 10
art -c make_input_producer.fcl --process-name=p1 -o "b1.root" -n 10

# process 2
art -c make_input_producer.fcl --process-name=p2 -o "a.root" -s "a1.root" -n 10
art -c make_input_producer.fcl --process-name=p2 -o "b2.root" -s "b1.root" -n 10

# process 3
art -c empty.fcl --process-name=p3 -o "b3.root" -s "b2.root" -n 10

# process 4
art -c make_input_producer.fcl --process-name=p4 -o "b.root" -s "b3.root" -n 10

rm b{1,2,3}.root a1.root
