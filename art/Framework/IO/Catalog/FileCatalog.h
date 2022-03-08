#ifndef art_Framework_IO_Catalog_FileCatalog_h
#define art_Framework_IO_Catalog_FileCatalog_h

// ======================================================================
//
// Class FileCatalog. Common services to manage File catalog
//
// ======================================================================

#include <string>

namespace art {

  class FileCatalogItem {
  public:
    FileCatalogItem() = default;

    FileCatalogItem(std::string const& pfn,
                    std::string const& lfn,
                    std::string const& uri)
      : pfn_{pfn}, lfn_{lfn}, uri_{uri}
    {}

    std::string const&
    fileName() const noexcept
    {
      return pfn_;
    }
    std::string const&
    logicalFileName() const noexcept
    {
      return lfn_;
    }
    std::string const&
    uri() const noexcept
    {
      return uri_;
    }
    bool
    skipped() const noexcept
    {
      return skipped_;
    }
    bool
    consumed() const noexcept
    {
      return consumed_;
    }

    void
    fileName(std::string const& pfn)
    {
      pfn_ = pfn;
    }
    void
    logicalFileName(std::string const& lfn)
    {
      lfn_ = lfn;
    }
    void
    uri(std::string const& uri)
    {
      uri_ = uri;
    }
    void
    skip()
    {
      skipped_ = true;
    }
    void
    consume()
    {
      consumed_ = true;
    }

  private:
    std::string pfn_{};
    std::string lfn_{};
    std::string uri_{};
    bool skipped_{false};
    bool consumed_{false};
  }; // FileCatalogItem

  // ----------------------------------------------------------------------

  class FileCatalog {
  public:
    virtual ~FileCatalog() = default;
    void
    commitCatalog()
    {}
    static bool
    isPhysical(std::string const& name) noexcept
    {
      return (name.empty() || name.find(':') != std::string::npos);
    }
    std::string&
    url() noexcept
    {
      return url_;
    }
    std::string const&
    url() const noexcept
    {
      return url_;
    }
    void
    setActive() noexcept
    {
      active_ = true;
    }
    bool
    active() const noexcept
    {
      return active_;
    }

  private:
    std::string url_{};
    bool active_{false};
  }; // FileCatalog

} // namespace art

// ======================================================================

#endif /* art_Framework_IO_Catalog_FileCatalog_h */

// Local Variables:
// mode: c++
// End:
