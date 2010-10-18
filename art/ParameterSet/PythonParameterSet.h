#error "Using obsolete ParameterSet/PythonParameterSet.h"


#if 0
#ifndef PythonParameterSet_h
#define PythonParameterSet_h

#include "fhiclcpp/ParameterSet.h"
#include <boost/python.hpp>
#include "art/ParameterSet/PythonWrapper.h"

class PythonParameterSet
{
public:
  PythonParameterSet();

  PythonParameterSet(const fhicl::ParameterSet & p)
  : theParameterSet(p) {}

  template <class T>
  T
  getParameter(bool tracked, std::string const& name) const
  {
    T result;
    if(tracked)
    {
      result = theParameterSet.template getParameter<T>(name);
    }
    else
    {
      result = theParameterSet.template getUntrackedParameter<T>(name, result);
    }
    return result;
  }


  template <class T>
  void
  addParameter(bool tracked, std::string const& name, T value)
  {
   if(tracked)
   {
     theParameterSet.template addParameter<T>(name, value);
   }
   else
   {
     theParameterSet.template addUntrackedParameter<T>(name, value);
   }
  }


  /// templated on the type of the contained object
  template <class T>
  boost::python::list
  getParameters(bool tracked, const std::string & name) const
  {
    std::vector<T> v = getParameter<std::vector<T> >(tracked, name);
    return art::toPythonList(v);
  }

  /// unfortunate side effect: destroys the original list!
  template <class T>
  void
  addParameters(bool tracked, std::string const& name,
                boost::python::list  value)
  {
    std::vector<T> v = art::toVector<T>(value);
    addParameter(tracked, name, v);
  }


  /// these custom classes do seem to be a hassle
  /// to wrap, compared to, say, InputTag
  /// maybe we will need to template these someday
  void addPSet(bool tracked, std::string const& name,
               const PythonParameterSet & ppset)
  {
    addParameter(tracked, name, ppset.theParameterSet);
  }


  PythonParameterSet getPSet(bool tracked, std::string const& name) const
  {
    return PythonParameterSet(getParameter<fhicl::ParameterSet>(tracked, name));
  }


  void addVPSet(bool tracked, std::string const& name,
                boost::python::list  value);

  boost::python::list getVPSet(bool tracked, std::string const& name);

  // no way to interface straight into the other python InputTag
  art::InputTag newInputTag(const std::string& label,
                            const std::string& instance,
                            const std::string& process)
  {
    return art::InputTag(label, instance, process);
  }

  art::EventID newEventID(unsigned int run, unsigned int event)
  {
    return art::EventID(run, event);
  }

  art::SubRunID newSubRunID(unsigned int run, unsigned int subRun)
  {
    return art::SubRunID(run, subRun);
  }

  void addNewFileInPath(bool tracked, std::string const & name, std::string const & value);

  PythonParameterSet newPSet() const {return PythonParameterSet();}

  const fhicl::ParameterSet & pset() const {return theParameterSet;}

  std::string dump() const {return theParameterSet.dump();}

private:
  fhicl::ParameterSet theParameterSet;
};

#endif
#endif  // 0
