#include <cassert>
#include <string>
#include <vector>

#include "art/Framework/PluginManager/FileOperations.h"

using namespace std;

int main()
{
  vector<string> results;
  plugin::get_map_list(results);
  assert(!results.empty());
}
