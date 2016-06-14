#ifndef art_Framework_Services_Registry_detail_SignalResponseType_h
#define art_Framework_Services_Registry_detail_SignalResponseType_h

// Definition for ActivityRegistry and friends of the two available
// signal response types and a suitable overload set for a function to
// connect a slot to a signal in the desired way.

#include "cpp0x/type_traits"

namespace art {
  namespace detail {
    enum class SignalResponseType { FIFO, LIFO };

    template <SignalResponseType STYPE, typename SIGNAL, typename FUNC>
    typename std::enable_if<STYPE == SignalResponseType::FIFO>::type
    connect_to_signal(SIGNAL & s, FUNC f) { s.emplace_back(f); }

    template <SignalResponseType STYPE, typename SIGNAL, typename FUNC>
    typename std::enable_if<STYPE == SignalResponseType::LIFO>::type
    connect_to_signal(SIGNAL & s, FUNC f) { s.emplace_front(f); }
  }
}

#endif /* art_Framework_Services_Registry_detail_SignalResponseType_h */

// Local Variables:
// mode: c++
// End:
