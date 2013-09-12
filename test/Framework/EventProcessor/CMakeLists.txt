art_make_exec(Statemachine_t
  SOURCE statemachine_t.cc MockEventProcessor.cc
  LIBRARIES
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  art_Framework_EventProcessor
  art_Framework_Core
  )

cet_script(Statemachine_t.sh NO_INSTALL)

# Shorthand to avoid writing almost the same thing three times.
macro(statemachine_test i)
  cet_test(Statemachine_t_${i} HANDBUILT
    TEST_EXEC Statemachine_t.sh
    DEPENDENCIES Statemachine_t
    DATAFILES
    unit_test_outputs/statemachine_${i}.txt
    unit_test_outputs/statemachine_output_${i}_ref.txt
    TEST_ARGS
    "statemachine_output_${i}_ref.txt"
    ${ARGN}
    -i "statemachine_${i}.txt"
    -o "statemachine_output_${i}.txt"
    )
endmacro()

# Multiple invocations of Statemachine_t.sh. We could do a couple of
# foreach clauses but then the tests would be out of order.
statemachine_test(01)
statemachine_test(02)
statemachine_test(03 -s)
statemachine_test(04 -s)
statemachine_test(05)
statemachine_test(06)
statemachine_test(07)
statemachine_test(08)
statemachine_test(09)
statemachine_test(10)
statemachine_test(11)
statemachine_test(12 -m)

cet_test(ServiceDirector_t USE_BOOST_UNIT
  LIBRARIES
  art_Framework_EventProcessor
  art_Framework_Services_Registry
  )
