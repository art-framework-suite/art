#include "art/Persistency/Provenance/SubRunID.h"
#include <ostream>
#include <limits>



namespace art {

  static unsigned int const shift = 8 * sizeof(unsigned int);

  SubRunID::SubRunID(boost::uint64_t id) :
   run_(static_cast<RunNumber_t>(id >> shift)),
   subRun_(static_cast<SubRunNumber_t>(std::numeric_limits<unsigned int>::max() & id))
  {
  }

  boost::uint64_t
  SubRunID::value() const {
   boost::uint64_t id = run_;
   id = id << shift;
   id += subRun_;
   return id;
  }

  std::ostream& operator<<(std::ostream& oStream, SubRunID const& iID) {
    oStream<< "run: " << iID.run() << " subRun: " << iID.subRun();
    return oStream;
  }
}
