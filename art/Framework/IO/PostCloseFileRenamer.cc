#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "boost/algorithm/string/replace.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"
#include "boost/regex.hpp"

#include <iomanip>
#include <string>
#include <sstream>

art::PostCloseFileRenamer::
PostCloseFileRenamer(std::string const & filePattern,
                     std::string const & moduleLabel,
                     std::string const & processName)
  :
  filePattern_(filePattern),
  moduleLabel_(moduleLabel),
  processName_(processName),
  lowest_(),
  highest_(),
  fo_(),
  fc_()
{
}

std::string
art::PostCloseFileRenamer::
parentPath() const
{
  boost::filesystem::path parent_path(boost::filesystem::path(filePattern_).parent_path());
  if (parent_path.empty()) {
    return std::string(".");
  } else {
    return parent_path.native();
  }
}

void
art::PostCloseFileRenamer::
recordFileOpen()
{
  reset_(); // Reset statistics.
  fo_ = boost::posix_time::second_clock::universal_time();
}

void
art::PostCloseFileRenamer::
recordEvent(EventID const & id)
{
  if (!lowest_.isValid() ||
      id <= lowest_) {
    lowest_ = id;
  }
  if (!lowest_.isValid() ||
      id >= highest_) {
    highest_ = id;
  }
}

void
art::PostCloseFileRenamer::
recordFileClose()
{
  fc_ =  boost::posix_time::second_clock::universal_time();
}

namespace {
  void filled_replace(std::string & target,
                      char c,
                      size_t num) {
    boost::regex const e(std::string("%(\\d*)") + c);
    boost::match_results<std::string::const_iterator> match_results;
    while (boost::regex_search(target, match_results, e)) {
      std::ostringstream num_str;
      if (match_results[1].length() > 0) {
        num_str << std::setfill('0') << std::setw(std::stoul(match_results[1]));
      }
      num_str << num;
      boost::replace_all(target, match_results[0].str(), num_str.str());
    }
  }
}

std::string
art::PostCloseFileRenamer::
applySubstitutions() const {
  auto result = filePattern_;
  using boost::replace_all;
  replace_all(result, "%l", moduleLabel_);
  replace_all(result, "%p", processName_);
  filled_replace(result, 'r', lowest_.run());
  filled_replace(result, 'R', highest_.run());
  filled_replace(result, 's', lowest_.subRun());
  filled_replace(result, 'S', highest_.subRun());
  replace_all(result, "%to", boost::posix_time::to_iso_string(fo_));
  replace_all(result, "%tc", boost::posix_time::to_iso_string(fc_));
  return std::move(result);
}

void
art::PostCloseFileRenamer::
maybeRenameFile(std::string const & inPath) {
  boost::filesystem::rename(inPath, applySubstitutions());
}

void
art::PostCloseFileRenamer::
reset_()
{
  fo_ =
    fc_ = boost::posix_time::ptime();
}
