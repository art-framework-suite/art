# Compile the exec (don't run). Yes, add_executable /
# target_link_libraries is also fine but this is one command with a
# uniform interface.
cet_test(Statemachine_t NO_AUTO
  SOURCES statemachine_t.cc MockEventProcessor.cc
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

# Multiple invocations of Statemachine_t.sh.
foreach(i RANGE 1 12)
  execute_process(COMMAND printf "%02d" ${i}
    OUTPUT_VARIABLE j
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  statemachine_test(${j})
endforeach()