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
    FileCatalogItem() : pfn_(), lfn_(), uri_(), skipped_(false), consumed_(false) {}
    FileCatalogItem(std::string const& pfn, std::string const& lfn, std::string const & uri)
      : pfn_(pfn), lfn_(lfn), uri_(uri), skipped_(false), consumed_(false) {}
    std::string const& fileName()        const {return pfn_;}
    std::string const& logicalFileName() const {return lfn_;}
    std::string const& uri()             const {return uri_;}
    void fileName       (std::string const & pfn) { pfn_ = pfn; }
    void logicalFileName(std::string const & lfn) { lfn_ = lfn; }
    void uri            (std::string const & uri) { uri_ = uri; }
    void skip()           { skipped_ = true; }
    bool skipped()  const { return skipped_; }
    void consume()        { consumed_ = true; }
    bool consumed() const { return consumed_; }
  private:
    std::string pfn_;
    std::string lfn_;
    std::string uri_;
    bool skipped_;
    bool consumed_;
  };  // FileCatalogItem

// ----------------------------------------------------------------------

  class FileCatalog {
  public:
    FileCatalog();
    virtual ~FileCatalog() = 0;
    void commitCatalog();
    static bool isPhysical(std::string const& name) {
      return (name.empty() || name.find(':') != std::string::npos);
    }
    std::string & url() {return url_;}
    std::string const& url() const {return url_;}
    void setActive() {active_ = true;}
    bool active() const {return active_;}
  private:
    std::string url_;
    bool active_;
  };  // FileCatalog

}  // art

// ======================================================================

#endif /* art_Framework_IO_Catalog_FileCatalog_h */

// Local Variables:
// mode: c++
// End:
