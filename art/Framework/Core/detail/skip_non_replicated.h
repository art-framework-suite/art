#ifndef art_Framework_Core_detail_skip_non_replicated_h
#define art_Framework_Core_detail_skip_non_replicated_h

namespace art {
  class Worker;
  namespace detail {
    bool skip_non_replicated(Worker const&);
  }
}

#endif
