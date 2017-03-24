#include "art/Framework/Services/Optional/detail/LinuxProcData.h"

using art::detail::LinuxProcData;
using proc_tuple = LinuxProcData::proc_tuple;
using vsize_t = LinuxProcData::vsize_t;
using rss_t = LinuxProcData::rss_t;

namespace {

  template <typename T>
  typename T::value_type
  getValue(proc_tuple const& t)
  {
    return std::get<T>(t).value;
  }

  template <typename T>
  typename T::value_type&
  getValue(proc_tuple& t)
  {
    return std::get<T>(t).value;
  }

}


LinuxProcData::proc_tuple
art::detail::operator-(LinuxProcData::proc_tuple const& left,
                       LinuxProcData::proc_tuple const& right)
{
  auto const vsize = getValue<vsize_t>(left)-getValue<vsize_t>(right);
  auto const rss = getValue<rss_t>(left)-getValue<rss_t>(right);
  return LinuxProcData::make_proc_tuple(vsize, rss);
}

LinuxProcData::proc_tuple&
art::detail::operator+=(LinuxProcData::proc_tuple& left,
                        LinuxProcData::proc_tuple const& right)
{
  getValue<vsize_t>(left) += getValue<vsize_t>(right);
  getValue<rss_t>(left) += getValue<rss_t>(right);
  return left;
}
