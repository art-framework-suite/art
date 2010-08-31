#include <cstdlib>
#include <sstream>


#include "boost/filesystem.hpp"
#include "boost/regex.hpp"

#include "art/Framework/PluginManager/FileOperations.h"
#include "art/Utilities/Exception.h"

using std::vector;
using std::string;
using std::ostringstream;
using std::istringstream;
using namespace boost;
using namespace boost::filesystem;

namespace plugin
{
  void get_map_list(vector<string>& result)
  {
    // load all the _map_ files now
    // JBK - hack for now - just look in one fixed spot for the
    // *_map_*.so files
    // This code really needs to go through every path in
    // LD_LIBRARY_PATH and find all the *_map_*.so files

    // This code fails if a non-existent directory is in LD_LIBRARY_PATH

    // we expect that the base library directory will be in LD_LIBRARY_PATH
    const char* ldlib_env = getenv("LD_LIBRARY_PATH");
    // right now, FW_HOME points to the source code directory,
    // which is not where cmake installs libraries
    // also, FW_HOME was not used in this code
    //const char* home_env = getenv("FW_HOME");
    // LOCAL_PLUGIN_DIR is the test library directory
    // this environmental variable may not exist
    const char* plug_env = getenv("LOCAL_PLUGIN_DIR");

    //if(!home_env)
    //  {
	//throw cms::Exception("env missing")
	//  << "cannot find environment variable FW_HOME\n";
    //  }
    if(!ldlib_env)
      {
	throw cms::Exception("env missing")
	  << "cannot find environment variable LD_LIBRARY_PATH\n";
      }

    string ldlib_str = ldlib_env;
    char tmp_str[500];
    istringstream ist(ldlib_str);
    typedef vector<string> strings;
    strings paths;
    bool stop=false;

    if(plug_env) {
        paths.push_back(std::string(plug_env));
      }
    while(!stop)
      {
	ist.getline(&tmp_str[0],sizeof(tmp_str),':');
	if(ist.eof()) stop=true;
	paths.push_back(tmp_str);
      }

    regex libpattern(".*_map_plugin.so$");
    for(strings::iterator cur(paths.begin()),end(paths.end());cur!=end;++cur)
      {
	//ostringstream ost;
	//ost << *cur << "/tmp/lib";
	path full_path( *cur );
	for (directory_iterator
	       i = directory_iterator(full_path),
	       e = directory_iterator();
	     i != e;
	     ++i)
	  {
	    string filename(i->path().leaf());
	    if (regex_match(filename, libpattern)) result.push_back(filename);
	  }	
      }
  }
}

