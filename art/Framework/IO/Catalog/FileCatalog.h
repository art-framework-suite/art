#ifndef art_Framework_IO_Catalog_FileCatalog_h
#define art_Framework_IO_Catalog_FileCatalog_h

// ======================================================================
//
// Class FileCatalog. Common services to manage File catalog
//
// ======================================================================

#include <string>

// ----------------------------------------------------------------------

namespace pool {
  class IFileCatalog
  {
  public:
    void commit() { }
    void start() { }
    void disconnect() { }
    int nReadCatalogs() { }
  };  // IFileCatalog
}  // pool

// ----------------------------------------------------------------------

namespace art {

  class FileCatalogItem {
  public:
    FileCatalogItem() : pfn_(), lfn_() {}
    FileCatalogItem(std::string const& pfn, std::string const& lfn) : pfn_(pfn), lfn_(lfn) {}
    std::string const& fileName() const {return pfn_;}
    std::string const& logicalFileName() const {return lfn_;}
  private:
    std::string pfn_;
    std::string lfn_;
  };  // FileCatalogItem

// ----------------------------------------------------------------------

  struct PoolCatalog {
    PoolCatalog() : catalog_() {}
    pool::IFileCatalog catalog_;
  };  // PoolCatalog

// ----------------------------------------------------------------------

  class FileCatalog {
  public:
    explicit FileCatalog(PoolCatalog & poolcat);
    virtual ~FileCatalog() = 0;
    void commitCatalog();
    static bool const isPhysical(std::string const& name) {
      return (name.empty() || name.find(':') != std::string::npos);
    }
    pool::IFileCatalog& catalog() {return catalog_;}
    std::string & url() {return url_;}
    std::string const& url() const {return url_;}
    void setActive() {active_ = true;}
    bool active() const {return active_;}
  private:
    pool::IFileCatalog& catalog_;
    std::string url_;
    bool active_;
  };  // FileCatalog

}  // art

// ======================================================================

#endif /* art_Framework_IO_Catalog_FileCatalog_h */

// Local Variables:
// mode: c++
// End:
