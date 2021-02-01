#ifndef art_Framework_IO_Root_EventMergerFileHandler_h
#define art_Framework_IO_Root_EventMergerFileHandler_h
// vim: set sw=2:

#include "art/Framework/IO/Root/file_groups.h"

#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/OptionalSequence.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/Table.h"

#include <string>
#include <vector>

namespace art {

  class EventMergerFileHandler {
  public:
    struct Config {
      using Comment = fhicl::Comment;
      using Name = fhicl::Name;
      struct FileNamePack {
        fhicl::Atom<std::string> primary{Name("primary")};
        fhicl::Sequence<std::string> secondaries{Name("secondaries")};
      };
      fhicl::OptionalSequence<fhicl::Table<FileNamePack>> fileNames{
        Name("fileNames"),
        Comment(
          "The 'fileNames' and 'fileList' parameters are mutually exclusive.\n"
          "The 'fileList' argument is the name of a file that contains all\n"
          "files to be read by the EventMerger input source.")};
      fhicl::OptionalAtom<std::string> fileList{
        Name("fileList"),
        Comment("The form of the entries in the 'fileList' is:\n\n"
                "  Inputfilex1| inputfilex2, ..., inputfilexN;\n"
                "  Inputfiley1| inputfiley2, ..., inputfileyM;\n"
                "  ...\n"
                "  Inputfilez1| inputfilez2, ..., inputfilezP;\n\n"
                "Whitespace is optional, as is the semicolon in the last line\n"
                "of the file.  Comments are prefixed with the '#' character\n"
                "ignoring all characters following it until the end of the "
                "line.\n\n"
                "The semicolon delimits each group of primary and secondary\n"
                "files, allowing users to split file specifications across\n"
                "multiple lines:\n\n"
                "  Inputfilex1|\n"
                "    inputfilex2,\n"
                "    # inputfilex3 <- Bad file\n"
                "    inputfilex4;")};
    };

    explicit EventMergerFileHandler(Config const& config);

    bool hasNextFile() const;
    bool getNextFile();

    std::string const&
    currentFileName() const
    {
      assert(currentFilePack_ != fileNames_.cend());
      return std::get<0>(*currentFilePack_);
    }

    std::vector<std::string> const&
    currentSecondaryFileNames() const
    {
      assert(currentFilePack_ != fileNames_.cend());
      return std::get<1>(*currentFilePack_);
    }

  private:
    using iter_t = collection_t::const_iterator;
    collection_t fileNames_;
    iter_t currentFilePack_{};
    iter_t nextFilePack_{};
  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_EventMergerFileHandler_h */
