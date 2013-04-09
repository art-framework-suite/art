#!/bin/bash

out=BlockingPrescaler_t-out.txt
ref=${out%-out.txt}-ref.txt

art -c "BlockingPrescaler_t.fcl" | \
  grep -v -e '^Begin' -e '^TimeReport' -e '^%MSG-i MF_INIT_OK' > "${out}" || \
  { exit $?; }

if [[ -n "$1" ]]; then # Produce ref
  cp "${out}" "${ref}"
else
  diff -u "${ref}" "${out}"
fi
exit $?
