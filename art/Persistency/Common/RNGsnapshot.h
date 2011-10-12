#ifndef art_Persistency_Common_RNGsnapshot_h
#define art_Persistency_Common_RNGsnapshot_h

// ======================================================================
//
// RNGsnapshot - a data product holding saved state from/for the
//               RandomNumberGenerator
//
// ======================================================================
//
// Notes:
// ------
//
// CLHEP specifies that the state of any of its engines will be returned
// as a vector<unsigned long>.  However, the size of such a long is
// machine-dependent.  If unsigned long is an 8 byte variable, only the
// least significant 4 bytes are filled and the most significant 4 bytes
// are zero.  We choose to store only the useful 32 bits, making the
// assumption (verified at compile-time) that unsigned int is always a
// 32-bit type.
//
// We would prefer to be more explicit about this by using a standard
// typedef such as saved_t32_t.  However, ROOT would have issues with this
// since ROOT sees only the underlying type, and not the typedef proper.
// Thus values written on one system and read on another would be
// problematic whenever the two systems did not agree on the underlying
// type.
//
// ======================================================================

#include "boost/static_assert.hpp"
#include <limits>
#include <string>
#include <vector>

// ======================================================================

namespace art {

  class RNGsnapshot {
  public:
    // --- CLHEP engine state characteristics:
    typedef  unsigned long         CLHEP_t;
    typedef  std::vector<CLHEP_t>  engine_state_t;

    // --- Our state characteristics:
    typedef  unsigned int          saved_t;
    typedef  std::vector<saved_t>  snapshot_state_t;
    typedef  std::string           label_t;

    BOOST_STATIC_ASSERT(std::numeric_limits<saved_t>::digits == 32);
    BOOST_STATIC_ASSERT(sizeof(saved_t) <= sizeof(CLHEP_t));

    // --- C'tor:
    RNGsnapshot();

    // --- Use compiler-generated copy c'tor, copy assignment, and d'tor

    // --- Access:
    std::string      const  & ekind() const  { return engine_kind_; }
    label_t          const  & label() const  { return label_; }
    snapshot_state_t const  & state() const  { return state_; }

    // --- Save/restore:
    void  saveFrom(std::string const &
                   , label_t const &
                   , engine_state_t const &);
    void  restoreTo(engine_state_t &) const;

  private:
    std::string       engine_kind_;
    label_t           label_;
    snapshot_state_t  state_;

  };  // RNGsnapshot

}  // art

// ======================================================================

#endif /* art_Persistency_Common_RNGsnapshot_h */

// Local Variables:
// mode: c++
// End:
