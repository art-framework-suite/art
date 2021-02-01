#include "art/Framework/IO/Root/EventMergerFileHandler.h"
#include "canvas/Utilities/Exception.h"

#include <fstream>

using namespace art;

EventMergerFileHandler::EventMergerFileHandler(Config const& config)
{
  std::vector<Config::FileNamePack> file_names;
  std::string file_list;

  unsigned parameter_count{};
  parameter_count += config.fileNames(file_names);
  parameter_count += config.fileList(file_list);

  if (parameter_count != 1u) {
    Exception e{errors::Configuration};
    e << "Either the 'fileNames' parameter must be specified or the 'fileList' "
         "parameter.\n";

    if (parameter_count == 0u) {
      e << "Neither was specified.";
    } else if (parameter_count == 2u) {
      e << "Both were specified.  This can happen if the '-S' or '-s' program\n"
           "option was specified at the command line when using the "
           "EventMerger input source.";
    }
    throw e;
  }

  for (auto const& fileNamePack : file_names) {
    fileNames_.emplace_back(fileNamePack.primary(), fileNamePack.secondaries());
  }

  if (not file_list.empty()) {
    std::ifstream file{file_list};
    if (not file) {
      throw Exception{errors::Configuration}
        << "Could not open file with the name '" << file_list << "'\n"
        << "Make sure the file exists.";
    }
    fileNames_ = file_groups(file);
  }
}

bool
EventMergerFileHandler::hasNextFile() const
{
  return nextFilePack_ != fileNames_.cend();
}

bool
EventMergerFileHandler::getNextFile()
{
  if (currentFilePack_ == fileNames_.cend()) {
    return false;
  }

  if (currentFilePack_ == iter_t{}) {
    // First file
    assert(nextFilePack_ == iter_t{});
    currentFilePack_ = fileNames_.cbegin();
    nextFilePack_ =
      fileNames_.empty() ? fileNames_.cend() : currentFilePack_ + 1;
  } else {
    ++currentFilePack_;
    ++nextFilePack_;
  }
  return true;
}
