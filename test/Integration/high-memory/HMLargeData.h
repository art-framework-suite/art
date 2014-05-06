#ifndef test_Integration_high_memory_HMLargeData_h
#define test_Integration_high_memory_HMLargeData_h

#include <vector>

namespace arttest {
  class HMLargeData;
}

#ifndef __GCCXML__
#include <cstddef>
namespace arttest {
  constexpr unsigned short N_BLOCKS = 28;
}
#endif

class arttest::HMLargeData {
public:
  HMLargeData & operator += (HMLargeData const & other);
#ifndef __GCCXML__
  static constexpr size_t size();
  float * data();
  float const * data() const;
#endif

private:
  static const int data_size_ = 32 * 12 * 32 * 100 * 3 * 5;
  std::vector<float> data_;
  // float data_[data_size_];
};

#ifndef __GCCXML__

#include <iterator>

arttest::HMLargeData &
arttest::HMLargeData::
operator += (HMLargeData const & other)
{
  auto o = std::begin(other.data_); // Should be cbegin in C++2014.
  for (auto i = std::begin(data_),
            e = std::begin(data_);
       i != e;
       ++i, ++o) {
    *i += *o;
  }
  return *this;
}

constexpr size_t
arttest::HMLargeData::
size()
{
  return data_size_;
}

float *
arttest::HMLargeData::
data()
{
  if (data_.size() < data_size_) {
    data_.resize(data_size_);
  }
  return data_.data();
}

float const *
arttest::HMLargeData::
data() const
{
  assert(!data_.size() < data_size_);
  return data_.data();
}

#endif /* __GCCXML__ */

#endif /* test_Integration_high_memory_HMLargeData_h */


// Local Variables:
// mode: c++
// End:
