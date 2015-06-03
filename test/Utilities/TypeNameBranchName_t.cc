#include "test/Utilities/TypeNameBranchName_t.h"

#include "art/Utilities/Exception.h"
#include "art/Utilities/FriendlyName.h"
#include "art/Utilities/uniform_type_name.h"
#include "cetlib/demangle.h"
#include "cetlib/map_vector.h"
#include "cetlib/replace_all.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <typeinfo>

namespace {
  bool friendlyNameIsBranchNameType = false;

  class TypeNames {
public:
    TypeNames()
      :
      v_(),
      valid_(false)
      {
      }
    TypeNames(std::vector<std::string> && v)
      :
      v_(std::move(v)),
      valid_((fixV(), v_.size() == expected_size))
      {
        throwIfInvalid_();
      }

    bool isValid() const { return valid_; }

    std::string const & mangled() const { return throwIfInvalid_(), v_[0]; }
    std::string const & abi() const { return throwIfInvalid_(), v_[1]; }
    std::string const & cint() const { return throwIfInvalid_(), v_[2]; }
    std::string const & reflex() const { return throwIfInvalid_(), v_[3]; }
    std::string const & branchNameType() const { return throwIfInvalid_(), v_[4]; }
    std::string const & friendly() const { return throwIfInvalid_(), v_[5]; }

private:
    static constexpr size_t expected_size = 6ul;

    void fixV()
      {
        if ((v_.size() == expected_size -1) &&
            friendlyNameIsBranchNameType) { // Older: no friendlyName.
          v_.push_back("UNKNOWN");
        }
      }

    void throwIfInvalid_() const
      {
        if (!isValid()) {
          std::ostringstream os;
          std::copy(v_.begin(), v_.end(), std::ostream_iterator<std::string>(os, ", "));
          if (os.tellp() > 0) {
            os.seekp(-2, std::ios_base::end); // Rewind.
          }
          throw art::Exception(art::errors::DataCorruption)
            << "Attempt to construct or use a TypeNames object with insufficient input items.\n"
            << "Contents: "
            << os
            << "\n";
        }
      }

    std::vector<std::string> v_;
    bool valid_;
  };

  bool goodMatchOrThrow(std::smatch const & match, std::string const & line)
  {
    if (match.size() != 2) {
      throw art::Exception(art::errors::LogicError)
        << "INTERNAL ERROR: nonsensical regex match for filename on input line:\n"
        << line << "\n";
    }
    return true;
  }

  std::string
  findDataFileName(std::istream & is)
  {
    std::string dataFN; // Result.
    static std::regex const fnReg("INFO: Examining file (.*) ...");
    static std::string const col_headers =
      friendlyNameIsBranchNameType ? "Mangled name : ABI-demangled name : Cint name : Reflex name : friendlyName"
      : "Mangled name : ABI-demangled name : Cint name : Reflex name : branchNameType : friendlyName";
    while (is) {
      std::smatch fnMatch;
      std::string line;
      std::getline(is, line);
      if (std::regex_match(line, fnMatch, fnReg) &&
          goodMatchOrThrow(fnMatch, line) &&
          (dataFN = fnMatch[1],
           std::getline(is, line),
           line == col_headers) &&
          (std::getline(is, line), is)) {
        break;
      }
    }
    return (is ? dataFN : std::string { }); // Stream must still be good.
  }

  bool validate(std::ostream & error, TypeNames tn)
  {
    bool result = false;
    std::string uniform_type_name = art::uniform_type_name(tn.abi());
    // Known differences between Reflex::Type::Name(Reflex::SCOPED) and
    // art::uniform_type_name().
    cet::replace_all(uniform_type_name, "ULong64_t", "unsigned long long");
    cet::replace_all(uniform_type_name, "Long64_t", "long long");
    cet::replace_all(uniform_type_name, "std::string>", "std::basic_string<char> >");
    cet::replace_all(uniform_type_name, "std::string", "std::basic_string<char>");

    if (uniform_type_name != tn.reflex()) {
      error << "ERROR: calculating uniform type name from \"" << tn.abi() << "\":"
            << "       calculated \"" << uniform_type_name << "\"; should be \""
            << tn.reflex() << "\".\n";
    } else {
      std::string friendlyName = art::friendlyname::friendlyName(uniform_type_name);
      std::string compare = tn.friendly();;
      if (friendlyNameIsBranchNameType) { // Older format, less info.
        compare = tn.branchNameType();
        // Known differences between the real friendlyName and that
        // stored as the, "type" part of the branch name.
        if (compare == "RunAuxiliary" ||
            compare == "SubRunAuxiliary" ||
            compare == "EventAuxiliary") {
          compare = std::string("art::") + compare;
        }
      }
      if (friendlyName != compare) {
        error << "ERROR: calculating friendlyName from \"" << uniform_type_name << "\":"
              << "       calculated \"" << friendlyName << "\"; should be \"" << tn.friendly() << "\".\n";
      } else {
        result = true;
      }
    }
    return result;
  }
}

int main(int argc, char **argv)
{
  using namespace arttest;
  using namespace std;

  ostream & out = cout;
  ostream & error = cerr;
  ////////////////////////////////////
  // Accumulators.
  auto total_successes = 0ul;
  auto text_files = 0ul;
  auto data_files = 0ul;
  ////////////////////////////////////
  if (argc == 1) {
    error << "ERROR: expected at least one filename as non-option argument!\n";
    return 1;
  }
  while (++argv, --argc) {
    auto failures_in_file = 0ul;
    auto fn = *argv;
    ifstream ts(fn);
    if (ts) {
      ++text_files;
    } else {
      error << "FATAL ERROR: unable to open file "
            << fn
            << ".\n";
      return 1;
    }
    std::string prog, ver;
    ts >> prog >> ver;
    static std::regex const verRE("v(\\d+)_(\\d+)_(\\d+)(.*)");
    std::smatch vm;
    if ((prog != "art_typetester") ||
        (!std::regex_match(ver, vm, verRE)) ||
        (vm.size() != 5) ||
        vm[1] != "0" ||
        vm[2] != "00") {
      error << "FATAL ERROR: unrecognized data file \n"
            << "       " << fn << ".\n";
      return 1;
    }
    // Can get more fancy when we have more versions.
    if (vm[3] == "02") {
      friendlyNameIsBranchNameType = true;
    } else if (vm[3] == "03") {
    } else {
      error << "FATAL ERROR: unrecognized data file \n"
            << "       " << fn << ".\n";
      return 1;
    }

    while (ts) {
      std::string dataFN = findDataFileName(ts);
      if (ts && !dataFN.empty()) {
        ++data_files;
      } else {
        break;
      }
      std::string line;
      static std::regex const lreg("(.+?)(?: : |$)");
      while (std::getline(ts, line) && (!line.empty())) {
        std::regex_iterator<std::string::iterator>
          it(line.begin(), line.end(), lreg), end;
        std::vector<std::string> typestrings;
        typestrings.reserve(6);
        for (std::size_t count = 0; it != end; ++it, ++count) {
          auto const & match = *it;
          assert(match.size() == 2);
          typestrings.emplace_back(match[1]);
        }
        if (validate(error, std::move(typestrings))) {
          ++total_successes;
        } else {
          ++failures_in_file;
        }
      } // Loop over type lines.
    } // Loop while we have a good stream.
    if (failures_in_file > 0) {
      error << "FATAL ERROR: found " << failures_in_file
            << " validation failures in text file\n"
            << "             " << fn << ".\n";
      return 1;
    }
  } // Loop over text files.
  out << "INFO: successfully validated "
      << total_successes
      << " type"
      << ((total_successes == 1ul) ? "" : "s")
      <<" in "
      << data_files
      << " data file"
      << ((data_files == 1ul) ? "" : "s")
      << " described over "
      << text_files
      << " text file"
      << ((text_files == 1ul) ? "" : "s")
      << "."
      << std::endl;
  return 0;
} // main.
