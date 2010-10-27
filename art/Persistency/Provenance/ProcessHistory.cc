#include "art/Persistency/Provenance/ProcessHistory.h"

#include "art/Utilities/Digest.h"
#include "cetlib/container_algorithms.h"
#include <iterator>
#include <ostream>
#include <sstream>


using namespace cet;
using namespace std;


namespace art {
  ProcessHistoryID
  ProcessHistory::id() const {
    if(phid().isValid()) {
      return phid();
    }
    // This implementation is ripe for optimization.
    // We do not use operator<< because it does not write out everything.
    ostringstream oss;
    for (const_iterator i = begin(), e = end(); i != e; ++i) {
      oss << i->processName() << ' '
	  << i->parameterSetID() << ' '
	  << i->releaseVersion() << ' '
	  << i->passID() << ' ';
    }
    string stringrep = oss.str();
    art::Digest md5alg(stringrep);
    ProcessHistoryID tmp(md5alg.digest().toString());
    phid().swap(tmp);
    return phid();
  }

  bool
  ProcessHistory::getConfigurationForProcess(string const& name,
					     ProcessConfiguration& config) const {
    for (const_iterator i = begin(), e = end(); i != e; ++i) {
      if (i->processName() == name) {
	config = *i;
	return true;
      }
    }
    // Name not found!
    return false;
  }

  bool
  isAncestor(ProcessHistory const& a, ProcessHistory const& b) {
    if (a.size() >= b.size()) return false;
    typedef ProcessHistory::collection_type::const_iterator const_iterator;
    for (const_iterator itA = a.data().begin(), itB = b.data().begin(),
         itAEnd = a.data().end(); itA != itAEnd; ++itA, ++itB) {
      if (*itA != *itB) return false;
    }
    return true;
  }

  ostream&
  operator<<(ostream& ost, ProcessHistory const& ph) {
    ost << "Process History = ";
    copy_all(ph, ostream_iterator<ProcessHistory::value_type>(ost,";"));
    return ost;
  }
}
