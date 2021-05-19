#ifndef art_Framework_Principal_ProcessTag_h
#define art_Framework_Principal_ProcessTag_h

#include <string>

namespace art {
  class ProcessTag {
  public:
    enum class allowed_search {
      all_processes,
      current_process,
      input_source,
      invalid
    };

    explicit ProcessTag();
    explicit ProcessTag(std::string const& specified_process_name);

    // Only c'tor that creates valid ProcessTag
    explicit ProcessTag(std::string const& specified_process_name,
                        std::string const& current_process_name);
    auto const&
    name() const
    {
      return name_;
    }
    bool current_process_search_allowed() const;
    bool input_source_search_allowed() const;

  private:
    allowed_search search_{allowed_search::invalid};
    std::string name_{};
  };
}

#endif /* art_Framework_Principal_ProcessTag_h */

// Local Variables:
// mode: c++
// End:
