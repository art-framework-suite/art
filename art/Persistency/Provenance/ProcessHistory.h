#ifndef art_Persistency_Provenance_ProcessHistory_h
#define art_Persistency_Provenance_ProcessHistory_h

#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/Transient.h"

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace art {
  class ProcessHistory;

  typedef std::map<ProcessHistoryID const, ProcessHistory> ProcessHistoryMap;

  void swap(art::ProcessHistory & a, art::ProcessHistory & b);

  bool operator == (art::ProcessHistory const& a, art::ProcessHistory const& b);

  bool operator!=(art::ProcessHistory const& a, art::ProcessHistory const& b);

  bool isAncestor(art::ProcessHistory const& a, art::ProcessHistory const& b);

  bool
  isDescendant(art::ProcessHistory const& a, art::ProcessHistory const& b);

  std::ostream& operator<<(std::ostream& ost, art::ProcessHistory const& ph);
}

class art::ProcessHistory {
public:
  typedef ProcessConfiguration    value_type;
  typedef std::vector<value_type> collection_type;

  typedef collection_type::iterator       iterator;
  typedef collection_type::const_iterator const_iterator;

  typedef collection_type::reverse_iterator       reverse_iterator;
  typedef collection_type::const_reverse_iterator const_reverse_iterator;

  typedef collection_type::reference       reference;
  typedef collection_type::const_reference const_reference;

  typedef collection_type::size_type size_type;

  ProcessHistory();
  explicit ProcessHistory(size_type n);
  explicit ProcessHistory(collection_type const& vec);

  void swap(ProcessHistory& other);

  // Put the given ProcessConfiguration into the history. This makes
  // our ProcessHistoryID become invalid.
  void push_back(const_reference t);

  bool empty() const;
  size_type size() const;
  size_type capacity() const;
  void reserve(size_type n);

  reference operator[](size_type i) {return data_[i];}
  const_reference operator[](size_type i) const {return data_[i];}

  reference at(size_type i) {return data_.at(i);}
  const_reference at(size_type i) const {return data_.at(i);}

  const_iterator begin() const {return data_.begin();}
  const_iterator end() const {return data_.end();}

#ifndef __GCCXML__
  const_iterator cbegin() const {return data_.cbegin();}
  const_iterator cend() const {return data_.cend();}
#endif // #ifndef __GCCXML__

  const_reverse_iterator rbegin() const {return data_.rbegin();}
  const_reverse_iterator rend() const {return data_.rend();}

#ifndef __GCCXML__
  const_reverse_iterator crbegin() const {return data_.crbegin();}
  const_reverse_iterator crend() const {return data_.crend();}
#endif // #ifndef __GCCXML__

  collection_type const& data() const {return data_;}
  ProcessHistoryID id() const;


  // Return true, and fill in config appropriately, if the a process
  // with the given name is recorded in this ProcessHistory. Return
  // false, and do not modify config, if process with the given name
  // is found.
  bool getConfigurationForProcess(std::string const& name, ProcessConfiguration& config) const;

  struct Transients {
    ProcessHistoryID phid_;
  };

private:
  collection_type data_;
  mutable Transient<Transients> transients_;

  void invalidateProcessHistoryID_() { transients_.get().phid_ = ProcessHistoryID(); }

  ProcessHistoryID & phid() const {return transients_.get().phid_;}
};

// Free swap function
inline
void
art::swap(art::ProcessHistory& a, art::ProcessHistory& b) {
  a.swap(b);
}

inline
bool
art::operator==(art::ProcessHistory const& a, art::ProcessHistory const& b) {
  return a.data() == b.data();
}

inline
bool
art::operator!=(art::ProcessHistory const& a, art::ProcessHistory const& b) {
  return !(a==b);
}

inline
bool
art::isDescendant(art::ProcessHistory const& a, art::ProcessHistory const& b) {
  return isAncestor(b, a);
}

//--------------------------------------------------------------------
// Implementation of ProcessHistory

inline
art::ProcessHistory::ProcessHistory() :
  data_(),
  transients_()
{ }

inline
art::ProcessHistory::ProcessHistory(size_type n) :
  data_(n),
  transients_()
{ }

inline
art::ProcessHistory::ProcessHistory(collection_type const& vec) :
  data_(vec),
  transients_()
{ }

inline
void art::ProcessHistory::swap(ProcessHistory& other)
{
  data_.swap(other.data_);
  phid().swap(other.phid());
}

inline
void art::ProcessHistory::push_back(const_reference t)
{
  data_.push_back(t);
  invalidateProcessHistoryID_();
}

inline
bool art::ProcessHistory::empty() const
{
  return data_.empty();
}

inline
art::ProcessHistory::size_type
art::ProcessHistory::size() const
{
  return data_.size();
}

inline
art::ProcessHistory::size_type
art::ProcessHistory::capacity() const
{
  return data_.capacity();
}

inline
void
art::ProcessHistory::reserve(size_type n)
{
  data_.reserve(n);
}

#endif /* art_Persistency_Provenance_ProcessHistory_h */

// Local Variables:
// mode: c++
// End:
