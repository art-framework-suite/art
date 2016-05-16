#!/bin/bash

cat << EOF
Modules:
`art --print-available-modules  | grep "art/art"`
Services:
`art --print-available-services | grep "art/art"`
EOF
