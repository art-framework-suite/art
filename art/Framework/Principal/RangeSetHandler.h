#ifndef art_Framework_Principal_RangeSetHandler_h
#define art_Framework_Principal_RangeSetHandler_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/RangeSet.h"
#include <memory>

namespace art {

class EventID;
class SubRunID;

class RangeSetHandler {

public: // TYPES

  enum class HandlerType {
      Open = 1
    , Closed // 2
  };

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~RangeSetHandler() noexcept;

public: // MEMBER FUNCTIONS -- API for the user

  HandlerType
  type() const;

  RangeSet
  seenRanges() const;

  void
  update(EventID const& id, bool const lastInSubRun);

  void
  flushRanges();

  void
  maybeSplitRange();

  void
  rebase();

  RangeSetHandler*
  clone() const;

private: // MEMBER FUNCTIONS -- API subclasses are required to provide

  virtual
  HandlerType
  do_type() const = 0;

  virtual
  RangeSet
  do_getSeenRanges() const = 0;

  virtual
  void
  do_update(EventID const&, bool lastInSubRun) = 0;

  virtual
  void
  do_flushRanges() = 0;

  virtual
  void
  do_maybeSplitRange() = 0;

  virtual
  void
  do_rebase() = 0;

  virtual
  RangeSetHandler*
  do_clone() const = 0;

};

} // namespace art

#endif /* art_Framework_Principal_RangeSetHandler_h */

// Local variables:
// mode: c++
// End:
