# project name
project(artParameterSet)

set( artParameterSet_sources Entry.cc
                             FileInPath.cc
                             MakeParameterSets.cc
                             ParameterSetEntry.cc
                             ProcessDesc.cc
                             pythonFileToConfigure.cc
#                             PythonParameterSet.cc
#                             PythonProcessDesc.cc
#                             PythonWrapper.cc
                             Registry.cc
                             types.cc )

#link_libraries( artUtilities
#                artMessageLogger
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${Boost_PYTHON_LIBRARY}
#                ${SIGC}
#                ${PYTHON} )

add_library(artParameterSet SHARED ${artParameterSet_sources} )

install ( TARGETS artParameterSet DESTINATION ${flavorqual_dir}/lib )
install(FILES Entry.h
              FileInPath.h
              InputTag.h
              MakeParameterSets.h
              ParameterSetEntry.h
              ParameterSetfwd.h
              ProcessDesc.h
              pythonFileToConfigure.h
#              PythonParameterSet.h
#              PythonProcessDesc.h
              Registry.h
              split.h
              types.h
         DESTINATION  ${product}/${version}/include/art/ParameterSet )
