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

#include <limits>
#include <string>
#include <type_traits>
#include <vector>

// ======================================================================

namespace art {

  class RNGsnapshot {
  public:
    // --- CLHEP engine state characteristics:
    using CLHEP_t        = unsigned long;
    using engine_state_t = std::vector<CLHEP_t>;

    // --- Our state characteristics:
    using saved_t          = unsigned int;
    using snapshot_state_t = std::vector<saved_t>;
    using label_t          = std::string;

    static_assert( std::numeric_limits<saved_t>::digits == 32,
                   "std::numeric_limits<saved_t>::digits != 32");
    static_assert( sizeof(saved_t) <= sizeof(CLHEP_t),
                   "sizeof(saved_t) > sizeof(CLHEP_t)" );

    // --- Access:
    std::string      const &  ekind( ) const  { return engine_kind_; }
    label_t          const &  label( ) const  { return label_; }
    snapshot_state_t const &  state( ) const  { return state_; }

    // --- Save/restore:
    void  saveFrom( std::string const &,
                    label_t const &,
                    engine_state_t const & );
    void  restoreTo( engine_state_t & ) const;

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
