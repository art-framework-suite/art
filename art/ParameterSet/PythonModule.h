#error "Using obsolete ParameterSet/PythonModule.h"


#if 0

#include "art/ParameterSet/PythonParameterSet.h"
#include "art/ParameterSet/PythonProcessDesc.h"
#include "art/Persistency/Provenance/EventID.h"
#include <boost/python.hpp>
#include <boost/cstdint.hpp>

using namespace boost::python;



BOOST_PYTHON_MODULE(libFWCoreParameterSet)
{
  class_<art::InputTag>("InputTag", init<std::string>())
      .def(init<std::string, std::string, std::string>())
     .def(init<std::string, std::string>())
      .def("label",    &art::InputTag::label, return_value_policy<copy_const_reference>())
      .def("instance", &art::InputTag::instance, return_value_policy<copy_const_reference>())
      .def("process",  &art::InputTag::process, return_value_policy<copy_const_reference>())
  ;

  class_<art::EventID>("EventID", init<unsigned int, unsigned int>())
      .def("run",   &art::EventID::run)
      .def("event", &art::EventID::event)
  ;

  class_<art::SubRunID>("SubRunID", init<unsigned int, unsigned int>())
      .def("run",    &art::SubRunID::run)
      .def("subRun", &art::SubRunID::subRun)
  ;

  class_<art::FileInPath>("FileInPath", init<std::string>())
      .def("fullPath",     &art::FileInPath::fullPath)
      .def("relativePath", &art::FileInPath::relativePath)
      .def("isLocal",      &art::FileInPath::isLocal)
  ;



  class_<PythonParameterSet>("ParameterSet")
    .def("addInt32", &PythonParameterSet::addParameter<int>)
    .def("getInt32", &PythonParameterSet::getInt)
    .def("addVInt32", &PythonParameterSet::addParameters<int>)
    .def("getVInt32", &PythonParameterSet::getParameters<int>)
    .def("addUInt32", &PythonParameterSet::addParameter<unsigned int>)
    .def("getUInt32", &PythonParameterSet::getParameter<unsigned int>)
    .def("addVUInt32", &PythonParameterSet::addParameters<unsigned int>)
    .def("getVUInt32", &PythonParameterSet::getParameters<unsigned int>)
    .def("addInt64", &PythonParameterSet::addParameter<boost::int64_t>)
    .def("getInt64", &PythonParameterSet::getParameter<boost::int64_t>)
    .def("addUInt64", &PythonParameterSet::addParameter<boost::uint64_t>)
    .def("getUInt64", &PythonParameterSet::getParameter<boost::uint64_t>)
    .def("addVInt64", &PythonParameterSet::addParameters<boost::int64_t>)
    .def("getVInt64", &PythonParameterSet::getParameters<boost::int64_t>)
    .def("addVUInt64", &PythonParameterSet::addParameters<boost::uint64_t>)
    .def("getVUInt64", &PythonParameterSet::getParameters<boost::uint64_t>)
    .def("addDouble", &PythonParameterSet::addParameter<double>)
    .def("getDouble", &PythonParameterSet::getParameter<double>)
    .def("addVDouble", &PythonParameterSet::addParameters<double>)
    .def("getVDouble", &PythonParameterSet::getParameters<double>)
    .def("addBool", &PythonParameterSet::addParameter<bool>)
    .def("getBool", &PythonParameterSet::getParameter<bool>)
    .def("addString", &PythonParameterSet::addParameter<std::string>)
    .def("getString", &PythonParameterSet::getString)
    .def("addVString", &PythonParameterSet::addParameters<std::string>)
    .def("getVString", &PythonParameterSet::getParameters<std::string>)
    .def("addInputTag", &PythonParameterSet::addParameter<art::InputTag>)
    .def("getInputTag", &PythonParameterSet::getParameter<art::InputTag>)
    .def("addVInputTag", &PythonParameterSet::addParameters<art::InputTag>)
    .def("getVInputTag", &PythonParameterSet::getParameters<art::InputTag>)
    .def("addEventID", &PythonParameterSet::addParameter<art::EventID>)
    .def("getEventID", &PythonParameterSet::getParameter<art::EventID>)
    .def("addVEventID", &PythonParameterSet::addParameters<art::EventID>)
    .def("getVEventID", &PythonParameterSet::getParameters<art::EventID>)
    .def("addSubRunID", &PythonParameterSet::addParameter<art::SubRunID>)
    .def("getSubRunID", &PythonParameterSet::getParameter<art::SubRunID>)
    .def("addVSubRunID", &PythonParameterSet::addParameters<art::SubRunID>)
    .def("getVSubRunID", &PythonParameterSet::getParameters<art::SubRunID>)
    .def("addPSet", &PythonParameterSet::addPSet)
    .def("getPSet", &PythonParameterSet::getPSet)
    .def("addVPSet", &PythonParameterSet::addVPSet)
    .def("getVPSet", &PythonParameterSet::getVPSet)
    .def("addFileInPath", &PythonParameterSet::addParameter<art::FileInPath>)
    .def("getFileInPath", &PythonParameterSet::getParameter<art::FileInPath>)
    .def("newInputTag", &PythonParameterSet::newInputTag)
    .def("newEventID", &PythonParameterSet::newEventID)
    .def("newSubRunID", &PythonParameterSet::newSubRunID)
    .def("addNewFileInPath", &PythonParameterSet::addNewFileInPath)
    .def("newPSet", &PythonParameterSet::newPSet)
    .def("dump", &PythonParameterSet::dump)
  ;


  class_<PythonProcessDesc>("ProcessDesc", init<>())
    .def(init<std::string>())
    .def("addService", &PythonProcessDesc::addService)
    .def("newPSet", &PythonProcessDesc::newPSet)
    .def("dump", &PythonProcessDesc::dump)
  ;

}

#endif  // 0
