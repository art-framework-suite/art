////////////////////////////////////////////////////////////////////////
// tbb_preduce_01_t
//
// Demonstrate trivial use of parallel_reduce over a vector with
// functors to ascertain the mean value in a vector, and the location
// and value of the minimum value in same.
//
////////////////////////////////////////////////////////////////////////

#include "tbb/tbb.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

typedef tbb::blocked_range<typename std::vector<double>::const_iterator> br_t;

// Calculate the mean.
class Meanie {
public:
  Meanie();
  Meanie(Meanie const &, tbb::split);
  void operator()(br_t const & r);
  void join(Meanie const & other);

  size_t count() const;
  double result() const;

private:
  size_t running_count_;
  double running_sum_;
};

// Calculate the min.
class Minnie {
public:
  Minnie();
  Minnie(br_t::const_iterator min);
  Minnie(Minnie const & other, tbb::split);
  void operator()(br_t const & r);
  void join(Minnie const & other);

  br_t::const_iterator min() const;
  bool valid() const;

private:
  br_t::const_iterator min_;
  bool valid_;
};

// Main program.
int main()
{
  // Setup.
  size_t const n = 500000;
  double const val = 27.125; // Exactly representable as a double in IEEE.
  std::vector<double> v(n, val);
  // Mean.
  Meanie m;
  tbb::parallel_reduce(br_t(v.cbegin(), v.cend()), m);
  assert(m.count() == n);
  assert(m.result() == val);
  // Setup for min.
  std::vector<double>::difference_type const loc = 47856;
  double const minval = 22.3;
  v[loc] = minval;
  // Min.
  Minnie mincalc;
  tbb::parallel_reduce(br_t(v.cbegin(), v.cend()), mincalc);
  assert(mincalc.valid() && std::distance(v.cbegin(), mincalc.min()) == loc);
  assert(*mincalc.min() == minval);
}

////////////////////////////////////////////////////////////////////////
// Member function implementations.

////////////////////////////////////
// Meanie

inline
Meanie::
Meanie()
  :
  running_count_(0),
  running_sum_(0.0)
{
}

inline
Meanie::
Meanie(Meanie const &, tbb::split)
  :
  Meanie()
{
}

inline
void
Meanie::
operator()(br_t const & r)
{
  running_count_ += r.size();
  running_sum_ = std::accumulate(r.begin(), r.end(), running_sum_);
}

inline
void
Meanie::
join(Meanie const & other)
{
  running_count_ += other.running_count_;
  running_sum_ += other.running_sum_;
}

inline
size_t
Meanie::
count() const
{
  return running_count_;
}

inline
double
Meanie::
result() const
{
  return running_count_ ? (running_sum_ / running_count_) : 0.0;
}

////////////////////////////////////
// Minnie

inline
Minnie::
Minnie()
  :
  min_(),
  valid_(false)
{
}

inline
Minnie::
Minnie(br_t::const_iterator min)
  :
  min_(min),
  valid_(true)
{
}

inline
Minnie::
Minnie(Minnie const & other, tbb::split)
  :
  min_(other.min_),
  valid_(other.valid_)
{
}

inline
void
Minnie::
operator()(br_t const & r)
{
  for (br_t::const_iterator i = r.begin(),
       e = r.end();
       i != e;
       ++i) {
    if (!valid_ || *i < *min_) {
      min_ = i;
      valid_ = true;
    }
  }
}

inline
void
Minnie::
join(Minnie const & other)
{
  if (other.valid_ &&
      (!valid_ || (*other.min_ < *min_))) {
    min_ = other.min_;
    valid_ = true;
  }
}

inline
br_t::const_iterator
Minnie::
min() const
{
  return min_;
}

inline
bool
Minnie::
valid() const
{
  return valid_;
}
