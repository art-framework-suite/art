#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Utilities/Exception.h"
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
  fc_(),
  seqNo_(0ul),
  lastOpenedInputFile_()
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
  // Don't care about the event number at the moment.
  recordSubRun(id.subRunID());
}

void
art::PostCloseFileRenamer::
recordRun(RunID const & id)
{
  if ((!lowest_.runID().isValid()) ||
      id < lowest_.runID()) {
    lowest_ = SubRunID::invalidSubRun(id);
  }
  if ((!highest_.runID().isValid()) ||
      id > highest_.runID()) {
    highest_ = SubRunID::invalidSubRun(id);
  }
}

void
art::PostCloseFileRenamer::
recordInputFile(std::string const & inputFileName)
{
  lastOpenedInputFile_ = inputFileName;
}

void
art::PostCloseFileRenamer::
recordSubRun(SubRunID const & id)
{
  if ((!lowest_.runID().isValid()) || // No lowest run yet.
      id.runID() < lowest_.runID() || // New lower run.
      (id.runID() == lowest_.runID() &&
       (id.isValid() &&
        ((!lowest_.isValid()) || // No valid subrun yet.
         id < lowest_)))) {
    lowest_ = id;
  }
  if (id > highest_) { // Sort-invalid-first gives the correct answer.
    highest_ = id;
  }
}

void
art::PostCloseFileRenamer::
recordFileClose()
{
  fc_ =  boost::posix_time::second_clock::universal_time();
  ++seqNo_;
}

std::string
art::PostCloseFileRenamer::
applySubstitutions() const {
  std::string result; // Empty
  using namespace boost::regex_constants;
  using boost::regex;
  using boost::regex_match;
  using boost::regex_search;
  using boost::smatch;
  smatch match;
  auto sb = filePattern_.cbegin(),
       si = sb,
       se = filePattern_.cend();
  while (regex_search(si,
                      se,
                      match,
                      regex("%[lp]|%(\\d+)?([#rRsS])|%t([oc])|%if([bnedp])|%ifs%([^%]*)%([^%]*)%([ig]*)%|%.",
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
      result += moduleLabel_;
      break;
    case 'p':
      result += processName_;
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
          << "Unrecognized substitution %"
          << *(match[0].first + 1)
          << " in pattern "
          << filePattern_
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
art::PostCloseFileRenamer::
subInputFileName_(boost::smatch const & match) const
{
  using namespace boost::regex_constants;
  using boost::regex;
  using boost::regex_replace;
  using boost::filesystem::path;
  using boost::filesystem::canonical;
  std::string result;
  // If the filename is empty, substitute "-". If it is merely the
  // required substitution that is empty, substitute "".
  if (!lastOpenedInputFile_.empty()) {
    boost::filesystem::path const ifp(lastOpenedInputFile_);
    if (match[4].matched) { // %if[bdenp]
      switch (*(match[4].first)) {
      case 'b':
      {
        // Base name without extension.
        result = ifp.stem().native();
      }
      break;
      case 'd':
      {
        // Fully resolved path without filename.
        result = canonical(ifp.parent_path()).native();
      }
      break;
      case 'e':
      {
        // Extension.
        result = ifp.extension().native();
      }
      break;
      case 'n':
      {
        // Base name with extension.
        result = ifp.filename().native();
      }
      break;
      case 'p':
      {
        // Fully-resolved path with filename.
        result = (canonical(ifp.parent_path()) / ifp.filename()).native();
      }
      break;
      default: // INTERNAL_ERROR.
        Exception(errors::LogicError)
          << "Internal error: subInputFileName_() did not recognize substitution code %if"
          << *(match[4].first)
          << ".\n";
        break;
      }
    } else if (match[5].matched) { // Regex substitution.
      // Decompose the regex;
      syntax_option_type sflags { ECMAScript };
      match_flag_type mflags { match_default };
      bool global { false };
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
              << "Internal error: subInputFileName_() did not recognize regex flag '"
              << c
              << "'.\n";
            break;
          }
        }
      }
      if (!global) {
        mflags |= format_first_only;
      }
      regex dsub(match[5].str(), sflags);
      result = regex_replace(lastOpenedInputFile_, dsub, match[6].str(), mflags);
    } else { // INTERNAL ERROR.
        throw Exception(errors::LogicError)
          << "Internal error: subInputFileName_() called for unknown reasons with pattern "
          << filePattern_
          << ".\n";
    }
  } else {
    result = "-";
  }
  return result;
}

std::string
art::PostCloseFileRenamer::
subTimestamp_(boost::smatch const & match) const
{
  std::string result;
  switch (*(match[3].first)) {
  case 'o': // Open.
    result = boost::posix_time::to_iso_string(fo_);
    break;
  case 'c': // Close.
    result = boost::posix_time::to_iso_string(fc_);
    break;
  default: // INTERNAL ERROR.
    throw Exception(errors::LogicError)
      << "Internal error: subTimestamp_() did not recognize substitution code %t"
      << *(match[3].first)
      << ".\n";
    break;
  }
  return result;
}

std::string
art::PostCloseFileRenamer::
subFilledNumeric_(boost::smatch const & match) const
{
  std::string result;
  std::ostringstream num_str;
  if (match[1].matched) { // Zero-filling.
    num_str << std::setfill('0') << std::setw(std::stoul(match[1].str()));
  }
  switch (*(match[2].first)) {
  case '#':
    num_str << seqNo_;
    break;
  case 'r':
    if (lowest_.runID().isValid()) {
      num_str << lowest_.run();
    }
    break;
  case 'R':
    if (highest_.runID().isValid()) {
      num_str << highest_.run();
    }
    break;
  case 's':
    if (lowest_.isValid()) {
      num_str << lowest_.subRun();
    }
    break;
  case 'S':
    if (highest_.isValid()) {
      num_str << highest_.subRun();
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

void
art::PostCloseFileRenamer::
maybeRenameFile(std::string const & inPath) {
  auto newFile = applySubstitutions();
  boost::system::error_code ec;
  boost::filesystem::rename(inPath, newFile, ec);
  if (ec) { // Fail (different flesystems? Try copy / delete instead).
    boost::filesystem::
      copy_file(inPath,
                newFile,
                boost::filesystem::copy_option::overwrite_if_exists);
    (void) boost::filesystem::remove(inPath);
  }
}

void
art::PostCloseFileRenamer::
reset_()
{
  fo_ =
    fc_ = boost::posix_time::ptime();
  lowest_ = SubRunID();
  highest_ = SubRunID();
}
