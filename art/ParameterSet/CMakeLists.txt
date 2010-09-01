# project name
project(artParameterSet)

set( artParameterSet_sources Entry.cc
				FileInPath.cc
				MakeParameterSets.cc
				ParameterDescription.cc
				ParameterDescriptionTemplate.cc
				ParameterSet.cc
				ParameterSetDescription.cc
				ParameterSetDescriptionFillerBase.cc
				ParameterSetEntry.cc
				ProcessDesc.cc
				pythonFileToConfigure.cc
				PythonParameterSet.cc
				PythonProcessDesc.cc
				PythonWrapper.cc
				Registry.cc
				types.cc )

link_libraries( artUtilities
		artMessageLogger
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_PYTHON_LIBRARY}
		${SIGC}
		${PYTHON} )

add_library(artParameterSet SHARED ${artParameterSet_sources} )

install ( TARGETS artParameterSet DESTINATION lib )
install(FILES Entry.h
	      FileInPath.h
	      InputTag.h
	      MakeParameterSets.h
	      ParameterDescription.h
	      ParameterDescriptionTemplate.h
	      ParameterSetDescriptionFillerBase.h
	      ParameterSetDescriptionFiller.h
	      ParameterSetDescriptionFillerPluginFactory.h
	      ParameterSetDescription.h
	      ParameterSetEntry.h
	      ParameterSetfwd.h
	      ParameterSet.h
	      ProcessDesc.h
	      pythonFileToConfigure.h
	      PythonParameterSet.h
	      PythonProcessDesc.h
	      Registry.h
	      split.h
	      types.h
	DESTINATION include/art/ParameterSet )
