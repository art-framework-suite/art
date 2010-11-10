#include "art/Framework/Core/LibraryManager.h"

art::Librarymanager::LibraryManager(std::string const& suffix)
{
  // TODO: We could also consider searching the ld.so.conf list, if
  // anyone asks for it.

  // If LD_LIBRARY_PATH is undefined, we have no libraries to find.
  const char* ld_library_path = getenv("LD_LIBRARY_PATH");
  if (ld_library_path == 0) return;


  const char* lid_library_path_end = ld_library_path + strlen(ld_library_path);
  std::vector<std::string> directories;
  cet::split(ld_library_path, ld_library_path_end, ":", directories);

  std::vector<std::string> matches;
  std::string pattern("lib.*_");
  pattern += suffix;
  pattern += "\.so"; // TODO: Make this OS-aware; Mac OS wants "\.dylib"


  
  const char* end = 
  cet::find_matching_files(pattern, dirname, matches);
  
}

art::LibraryManager::~LibraryManager()
{
  
}
