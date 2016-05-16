#!/bin/bash

# Creates 'a.root' and 'b.root' with the following shapes, where the
# 'b.root' shape is expanded wrt 'a.root', but only by doing
# drop/simple pass-though processes.
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
#                   p3: [empty] - drop some products
#                   p4: [empty] - simple pass through

# process 1
art -c make_input_producer.fcl --process-name=p1 -o "a1.root" -n 10
art -c make_input_producer.fcl --process-name=p1 -o "b1.root" -n 10

# process 2
art -c make_input_producer.fcl --process-name=p2 -o "a.root" -s "a1.root" -n 10
art -c make_input_producer.fcl --process-name=p2 -o "b2.root" -s "b1.root" -n 10

# process 3
art -c drop_products.fcl --process-name=p3 -o "b3.root" -s "b2.root" -n 10

# process 4
art -c empty.fcl --process-name=p4 -o "b.root" -s "b3.root" -n 10

rm b{1,2,3}.root a1.root
