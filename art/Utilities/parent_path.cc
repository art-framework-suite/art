////////////////////////////////////////////////////////////////////////
// parent_path
//
// Return the parent of the supplied path. If this was a filename
// without path, return "." otherwise return either the containing
// directory if the path represents a file; or the parent directory if
// the path represents a directory.
//
// This is a thin wrapper around boost::filesystem::path::parent_path().
////////////////////////////////////////////////////////////////////////
#include "art/Utilities/parent_path.h"
#include "boost/filesystem.hpp"

std::string
art::parent_path(std::string const& in_path)
{
  using namespace std::string_literals;
  std::string result;
  boost::filesystem::path parent_path(
    boost::filesystem::path(in_path).parent_path());
  if (parent_path.empty()) {
    result = "."s;
  } else {
    result = parent_path.native();
  }
  return result;
}
