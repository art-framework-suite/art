#include "art/Framework/IO/PostCloseFileRenamer.h"

#include "art/Framework/IO/FileStatsCollector.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"
#include "boost/regex.hpp"
#include "canvas/Utilities/Exception.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace bfs = boost::filesystem;

art::PostCloseFileRenamer::PostCloseFileRenamer(FileStatsCollector const& stats)
  : stats_{stats}
{}

std::string
art::PostCloseFileRenamer::applySubstitutions(
  std::string const& filePattern) const
{
  std::string result; // Empty
  using namespace boost::regex_constants;
  using boost::regex;
  using boost::regex_match;
  using boost::regex_search;
  using boost::smatch;
  smatch match;
  auto sb = filePattern.cbegin(), si = sb, se = filePattern.cend();
  while (regex_search(si,
                      se,
                      match,
                      regex("%[lp]|%(\\d+)?([#rRsS])|%t([oc])|%if([bnedp])|%"
                            "ifs%([^%]*)%([^%]*)%([ig]*)%|%.",
                            ECMAScript))) {
    // Precondition: that the regex creates the sub-matches we think it
    // should.
    assert(match.size() == 8);
    // Subexpressions:
    //   0: Entire matched expression.
    //   1. Possible fill format digits for numeric substitution.
    //   2. Numeric substitution specifier.
    //   3. Timestamp substitution specifier.
    //   4. Input file name substitution specifier.
    //   5. Input file name regex match clause.
    //   6. Input file name regex substitution clause.
    //   7. Input file name regex flags.
    //
    // Note that we're not using named capture groups because that is
    // not supported by C++11 to which we will eventually migrate when
    // GCC supports the full C++11 regex functionality (reportedly
    // 4.9.0).
    //
    // Add the bit before the next substitution pattern to the result.
    result += match.prefix();
    // Decide what we're going to add to the result instead of the
    // substitution pattern:
    switch (*(match[0].first + 1)) {
      case 'l':
        result += stats_.moduleLabel();
        break;
      case 'p':
        result += stats_.processName();
        break;
      case 'i':
        result += subInputFileName_(match);
        break;
      case 't':
        result += subTimestamp_(match);
        break;
      default:
        if (match[2].matched) { // [#rRsS]
          result += subFilledNumeric_(match);
        } else {
          throw Exception(errors::Configuration)
            << "Unrecognized substitution %" << *(match[0].first + 1)
            << " in pattern " << filePattern
            << " -- typo or misconstructed regex?\n";
        }
        break;
    }
    si = match[0].second; // Set position for next match start.
  }
  result.append(si, se); // Append unmatched text at end.
  return result;
}

std::string
art::PostCloseFileRenamer::subInputFileName_(boost::smatch const& match) const
{
  using namespace boost::regex_constants;
  using bfs::canonical;
  using bfs::path;
  using boost::regex;
  using boost::regex_replace;
  std::string result;
  // If the filename is empty, substitute "-". If it is merely the
  // required substitution that is empty, substitute "".
  if (!stats_.lastOpenedInputFile().empty()) {
    bfs::path const ifp{stats_.lastOpenedInputFile()};
    if (match[4].matched) { // %if[bdenp]
      switch (*(match[4].first)) {
        case 'b': {
          // Base name without extension.
          result = ifp.stem().native();
        } break;
        case 'd': {
          // Fully resolved path without filename.
          result = canonical(ifp.parent_path()).native();
        } break;
        case 'e': {
          // Extension.
          result = ifp.extension().native();
        } break;
        case 'n': {
          // Base name with extension.
          result = ifp.filename().native();
        } break;
        case 'p': {
          // Fully-resolved path with filename.
          result = (canonical(ifp.parent_path()) / ifp.filename()).native();
        } break;
        default: // INTERNAL_ERROR.
          Exception(errors::LogicError)
            << "Internal error: subInputFileName_() did not recognize "
               "substitution code %if"
            << *(match[4].first) << ".\n";
          break;
      }
    } else if (match[5].matched) { // Regex substitution.
      // Decompose the regex;
      syntax_option_type sflags{ECMAScript};
      match_flag_type mflags{match_default};
      bool global{false};
      if (match[7].matched) { // Options.
        for (auto c : match[7].str()) {
          switch (c) {
            case 'i':
              sflags |= icase;
              break;
            case 'g':
              global = true;
              break;
            default: // INTERNAL ERROR.
              throw Exception(errors::LogicError)
                << "Internal error: subInputFileName_() did not recognize "
                   "regex flag '"
                << c << "'.\n";
              break;
          }
        }
      }
      if (!global) {
        mflags |= format_first_only;
      }
      regex const dsub{match[5].str(), sflags};
      result = regex_replace(
        stats_.lastOpenedInputFile(), dsub, match[6].str(), mflags);
    } else { // INTERNAL ERROR.
      throw Exception(errors::LogicError)
        << "Internal error: subInputFileName_() called for unknown reasons "
           "with pattern "
        << match[0].str() << ".\n";
    }
  } else {
    result = "-";
  }
  return result;
}

std::string
art::PostCloseFileRenamer::subTimestamp_(boost::smatch const& match) const
{
  std::string result;
  switch (*(match[3].first)) {
    case 'o': // Open.
      result = boost::posix_time::to_iso_string(stats_.outputFileOpenTime());
      break;
    case 'c': // Close.
      result = boost::posix_time::to_iso_string(stats_.outputFileCloseTime());
      break;
    default: // INTERNAL ERROR.
      throw Exception(errors::LogicError)
        << "Internal error: subTimestamp_() did not recognize substitution "
           "code %t"
        << *(match[3].first) << ".\n";
      break;
  }
  return result;
}

std::string
art::PostCloseFileRenamer::subFilledNumeric_(boost::smatch const& match) const
{
  std::string result;
  std::ostringstream num_str;
  if (match[1].matched) { // Zero-filling.
    num_str << std::setfill('0') << std::setw(std::stoul(match[1].str()));
  }
  switch (*(match[2].first)) {
    case '#':
      num_str << stats_.sequenceNum();
      break;
    case 'r':
      if (stats_.lowestSubRunID().runID().isValid()) {
        num_str << stats_.lowestSubRunID().run();
      }
      break;
    case 'R':
      if (stats_.highestSubRunID().runID().isValid()) {
        num_str << stats_.highestSubRunID().run();
      }
      break;
    case 's':
      if (stats_.lowestSubRunID().isValid()) {
        num_str << stats_.lowestSubRunID().subRun();
      }
      break;
    case 'S':
      if (stats_.highestSubRunID().isValid()) {
        num_str << stats_.highestSubRunID().subRun();
      }
      break;
    default: // INTERNAL ERROR.
      break;
  }
  result = num_str.str();
  if (result.empty()) {
    result = "-";
  }
  return result;
}

std::string
art::PostCloseFileRenamer::maybeRenameFile(std::string const& inPath,
                                           std::string const& toPattern)
{
  std::string const& newFile = applySubstitutions(toPattern);
  boost::system::error_code ec;
  bfs::rename(inPath, newFile, ec);
  if (ec) {
    // Fail (different filesystems? Try copy / delete instead).
    // This attempt will throw on failure.
    bfs::copy_file(inPath, newFile, bfs::copy_option::overwrite_if_exists);
    bfs::remove(inPath);
  }
  return newFile;
}
