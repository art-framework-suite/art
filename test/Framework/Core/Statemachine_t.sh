#!/bin/bash

function usage() {
cat 1>&2 <<EOF
usage: ${0##*/} <reference-output> <passthrough-args>"
<passthrough-args>
  [-s|-m]
  -i <input>
  -o <output>
EOF
}

# Preserve arguments for sourcing functions.
declare -a args=("$@")
set --

# Source functions.
. cet_test_functions.sh

# Restore arguments.
set -- "${args[@]}"

# Process arguments.
reference="${1}"
shift

# Check the reference file exists.
[[ -r "$reference" ]] || { echo "Reference \"$1\" not readable." 1>&2; usage; exit 1; }

# Save the arguments again for passing to the test exec.
declare -a args=("$@")

# Process possible arguments -- all we want is the output file for
# comparison with the reference.
while getopts :i:o:ms OPT; do
    case $OPT in
        i)
        # Passthrough
        ;;
        o)
        output="$OPTARG"
        ;;
        m)
        # Passthrough
        ;;
        s)
        # Passthrough
        ;;
        *)
        usage
        exit 2
    esac
done
shift $[ OPTIND - 1 ]

check_exit Statemachine_t "${args[@]}"
check_exit cmp -s "$reference" "$output"
