#ifndef art_Ntuple_sqlite_stringstream_h
#define art_Ntuple_sqlite_stringstream_h

// =================================================================
//
// sqlite stringstream
//
// To be used to bind the result of an sqlite query to objects.
//
// This class has been designed to behave like an std::stringstream
// object.  It is more flexible in that it supports move operations,
// which gcc 4.9 does not currently support.  The gcc 5 series will
// include move operations for iostream objects, but until we get
// there, something hard-coded needs to be done.
//
// For insertion operations, the following conversions are supported:
//
// .. std::string ==> std::string
//
//   ( as explicit specializations )
//
// .. std::string ==> int
// .. std::string ==> long
// .. std::string ==> long long
// .. std::string ==> unsigned
// .. std::string ==> unsigned long
// .. std::string ==> unsigned long long
// .. std::string ==> float
// .. std::string ==> double
// .. std::string ==> long double
//
// =================================================================

#include <deque>
#include <string>
#include <type_traits>
#include <vector>

#include "canvas/Utilities/Exception.h"

namespace sqlite {

  class stringstream {
  public:

    bool empty() const { return data_.empty(); }
    std::size_t size() const { return data_.size(); }

    std::string const& operator[](int const index) const { return data_[index]; }

    stringstream& operator<< (char const* str) &
    {
      data_.push_back(str);
      return *this;
    }

    stringstream& operator<< (char* str) &
    {
      data_.push_back(str);
      return *this;
    }

    stringstream& operator<< (std::string const& str) &
    {
      data_.push_back(str);
      return *this;
    }

    template <typename T>
    stringstream& operator<< (T arg) &
    {
      data_.push_back(std::to_string(std::move(arg)));
      return *this;
    }

    // Inserters

    template<typename T>
    auto convertTo(std::string const& arg)
    {
      return arg;
    }

    template <typename T>
    stringstream& operator>> (T& arg)
    {
      check_size_();
      arg = convertTo<T>(data_.front());
      data_.pop_front();
      return *this;
    }

    template <typename T>
    stringstream& operator>> (std::vector<T>& arg)
    {
      check_size_();
      arg.assign(data_.begin(), data_.end());
      data_.clear();
      return *this;
    }

    stringstream() = default;

    using data_container_t = std::deque<std::string>;

    stringstream(stringstream&&) noexcept(std::is_nothrow_move_constructible<data_container_t>::value)= default;
    stringstream& operator=(stringstream&&) noexcept(std::is_nothrow_move_assignable<data_container_t>::value)= default;

    // Disable copy c'tor/assignment
    stringstream(stringstream const&) = delete;
    stringstream& operator=(stringstream const&) = delete;

  private:

    data_container_t data_;

    void check_size_()
    {
      if (data_.empty()) {
        throw art::Exception(art::errors::LogicError)
          << "sqlite::stringstream is empty.  Cannot pop front.";
      }
    }

  };

  template<> inline auto stringstream::convertTo<int               >(std::string const& arg) { return std::stoi  (arg); }
  template<> inline auto stringstream::convertTo<long              >(std::string const& arg) { return std::stol  (arg); }
  template<> inline auto stringstream::convertTo<long long         >(std::string const& arg) { return std::stoll (arg); }
  template<> inline auto stringstream::convertTo<unsigned          >(std::string const& arg) { return std::stoul (arg); }
  template<> inline auto stringstream::convertTo<unsigned long     >(std::string const& arg) { return std::stoul (arg); }
  template<> inline auto stringstream::convertTo<unsigned long long>(std::string const& arg) { return std::stoull(arg); }
  template<> inline auto stringstream::convertTo<float             >(std::string const& arg) { return std::stof  (arg); }
  template<> inline auto stringstream::convertTo<double            >(std::string const& arg) { return std::stod  (arg); }
  template<> inline auto stringstream::convertTo<long double       >(std::string const& arg) { return std::stold (arg); }

} // namespace sqlite

#endif /* art_Ntuple_sqlite_stringstream_h */

// Local variables:
// mode: c++
// End:
