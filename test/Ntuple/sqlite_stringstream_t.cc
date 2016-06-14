#include "art/Ntuple/sqlite_stringstream.h"
#include "art/Utilities/Exception.h"

#include <string>

using namespace std::string_literals;
using namespace sqlite;

int main() {

  stringstream ss;

  // put a few things in
  ss << "const char*"
     << "std::string"s
     << 1
     << 2.4
     << 3.6f;

  std::string first;
  std::string second;
  int one;
  double next;
  float last;

  if ( !ss.empty() ) {
    ss >> first
       >> second
       >> one
       >> next
       >> last;
  }

  try {
    int x;
    ss >> x;
    throw art::Exception( art::errors::Unknown,"shouldn't get here");
  }
  catch ( art::Exception const & e ) {
    if ( e.categoryCode() == art::errors::Unknown ) {
      throw art::Exception( art::errors::Unknown, "this is really bad" );
    }
  }
  catch (...){}

}
