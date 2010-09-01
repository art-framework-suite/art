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
	DESTINATION include/FWCore/ParameterSet/interface )
>>><<<
# project name
project(FWCoreParameterSet)

# version number
set (FWCoreParameterSet_VERSION_MAJOR 1)
set (FWCoreParameterSet_VERSION_MINOR 0)

set( FWCoreParameterSet_sources Entry.cc
				FileInPath.cc
				MakeParameterSets.cc
				ParameterDescription.cc
				ParameterDescriptionTemplate.cc
				ParameterSet.cc
				ParameterSetDescription.cc
				ParameterSetDescriptionFillerBase.cc
				ParameterSetDescriptionFillerPluginFactory.cc
				ParameterSetEntry.cc
				ProcessDesc.cc
				pythonFileToConfigure.cc
				PythonParameterSet.cc
				PythonProcessDesc.cc
				PythonWrapper.cc
				Registry.cc
				types.cc )

link_libraries( FWCoreUtilities
		FWCoreMessageLogger
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_PYTHON_LIBRARY}
		${SIGC}
		${PYTHON} )

add_library(FWCoreParameterSet SHARED ${FWCoreParameterSet_sources} )

install ( TARGETS FWCoreParameterSet DESTINATION lib )
