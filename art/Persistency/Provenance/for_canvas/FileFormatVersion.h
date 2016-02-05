#ifndef art_Persistency_Provenance_FileFormatVersion_h
#define art_Persistency_Provenance_FileFormatVersion_h

#include <iosfwd>
#include <string>

namespace art {
   struct FileFormatVersion;
}

struct art::FileFormatVersion {
   FileFormatVersion() : value_(-1), era_() { }
   explicit FileFormatVersion(int vers) : value_(vers), era_() { }
   FileFormatVersion(int vers, std::string const &era) :
      value_(vers), era_(era) {}
   bool isValid() const { return value_ >= 0; }

   bool fastCopyPossible() const { return true; }

   int value_;
   std::string era_;
};

// Note for backward compatibility and efficiency reasons, comparison
// operators do not take era into account. This must be checked explicitly.
inline
bool operator== (art::FileFormatVersion const& a,
                 art::FileFormatVersion const& b)
{
   return a.value_ == b.value_;
}

inline
bool operator!= (art::FileFormatVersion const& a,
                 art::FileFormatVersion const& b)
{
   return !(a==b);
}

std::ostream&
operator<< (std::ostream& os,
            art::FileFormatVersion const& ff);

#endif /* art_Persistency_Provenance_FileFormatVersion_h */

// Local Variables:
// mode: c++
// End:
