#ifndef art_Persistency_Provenance_ParameterSetBlob_h
#define art_Persistency_Provenance_ParameterSetBlob_h

/*----------------------------------------------------------------------

ParameterSetBlob: A string in which to store a parameter set so that it can be made persistent.

The ParameterSetBlob is a concatenation of the names and values of the
tracked parameters within a ParameterSet,

----------------------------------------------------------------------*/

#include <iosfwd>
#include <string>

// ----------------------------------------------------------------------

namespace art {

  struct ParameterSetBlob
  {
    typedef std::string value_t;

    ParameterSetBlob() : pset_() {}
    explicit ParameterSetBlob(value_t const& v) : pset_(v) {}

    value_t pset_;
  };

  std::ostream&
  operator<<(std::ostream& os, ParameterSetBlob const& blob);

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_ParameterSetBlob_h */

// Local Variables:
// mode: c++
// End:
