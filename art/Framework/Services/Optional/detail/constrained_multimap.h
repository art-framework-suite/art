#ifndef art_Framework_Services_Optional_detail_constrained_map_h
#define art_Framework_Services_Optional_detail_constrained_map_h

// ====================================================
//
// constrained_multimap
//
// Class that constrains the number of unique keys to be no larger
// than the value of SIZE_LIMIT -- this enforced by the user calling
// 'maybe_emplace'.
//
// ====================================================

#include <map>

namespace art {
  namespace detail {

    template< unsigned SIZE_LIMIT, typename ... ARGS >
    class constrained_multimap {
    public:

      constrained_multimap() : nUniqueKeys_(0) {}

      using multimap_type = std::multimap<ARGS...>;

      auto begin  () const { return map_.begin  (); }
      auto end    () const { return map_.end    (); }
      auto cbegin () const { return map_.cbegin (); }
      auto cend   () const { return map_.cend   (); }
      auto crbegin() const { return map_.crbegin(); }
      auto crend  () const { return map_.crend  (); }

      auto empty() const { return map_.empty(); }
      auto size () const { return map_.size (); }
      auto unique_key_limit() const { return SIZE_LIMIT; }
      void erase( typename multimap_type::const_iterator iter ) { map_.erase(iter); }

      using key_type    = typename multimap_type::key_type;
      using mapped_type = typename multimap_type::mapped_type;
      void maybe_emplace( key_type const &, mapped_type const & );

    private:
      multimap_type map_;
      std::size_t   nUniqueKeys_;

      void computeNoUniqueKeys_();

    };

    //===========================================================================
    // Implementation below

    template< unsigned SIZE_LIMIT, typename ... ARGS >
    void
    constrained_multimap<SIZE_LIMIT, ARGS...>::
    maybe_emplace( key_type const & key, mapped_type const & value ) {

      if ( nUniqueKeys_ < SIZE_LIMIT ){
        map_.emplace( key, value );
        computeNoUniqueKeys_();
        return;
      }

      auto position = map_.upper_bound( key );

      if ( position != map_.cbegin() && nUniqueKeys_ == SIZE_LIMIT ) {
        map_.emplace_hint( position, key, value );
        computeNoUniqueKeys_();
        if ( nUniqueKeys_ == SIZE_LIMIT+1 ) {
          auto iters = map_.equal_range( cbegin()->first );
          map_.erase( iters.first, iters.second );
          computeNoUniqueKeys_();
        }
      }

    }

    //===========================================================================
    template< unsigned SIZE_LIMIT, typename ... ARGS >
    void
    constrained_multimap<SIZE_LIMIT, ARGS...>::
    computeNoUniqueKeys_(){
      std::size_t nkeys(0);
      for( auto it = map_.begin() ; it != map_.end(); ++nkeys) {
        it = map_.equal_range( it->first ).second;
      }
      nUniqueKeys_ = nkeys;
    }

  }
}

#endif // art_Framework_Services_Optional_detail_constrained_multimap_h

// Local variables:
// mode: c++
// End:
