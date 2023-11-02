#ifndef art_Framework_Services_Optional_detail_LinuxMallInfo_h
#define art_Framework_Services_Optional_detail_LinuxMallInfo_h

//===================================================================
//
// LinuxMallInfo
//
//-----------------------------------------------
//
// This is a helper class for storing information that is available
// through a call to mallinfo, the contents of which are:
//
//  struct mallinfo {
//
//    MALLINFO_FIELD_TYPE arena;    /* non-mmapped space allocated from system
//    */ MALLINFO_FIELD_TYPE ordblks;  /* number of free chunks */
//    MALLINFO_FIELD_TYPE smblks;   /* always 0 */
//    MALLINFO_FIELD_TYPE hblks;    /* always 0 */
//    MALLINFO_FIELD_TYPE hblkhd;   /* space in mmapped regions */
//    MALLINFO_FIELD_TYPE usmblks;  /* maximum total allocated space */
//    MALLINFO_FIELD_TYPE fsmblks;  /* always 0 */
//    MALLINFO_FIELD_TYPE uordblks; /* total allocated space */
//    MALLINFO_FIELD_TYPE fordblks; /* total free space */
//    MALLINFO_FIELD_TYPE keepcost; /* releasable (via malloc_trim) space */
//
//  };
//
//===================================================================

extern "C" {
#include <malloc.h>
}

namespace art {
  namespace detail {

    class LinuxMallInfo {
    public:

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
      // FIXME: The 'mallinfo()' function is deprecated for AL9.
      LinuxMallInfo() : minfo_(mallinfo()) {}
#pragma GCC diagnostic pop

      struct mallinfo
      get() const
      {
        return minfo_;
      }

    private:
      struct mallinfo minfo_;

      friend std::ostream& operator<<(std::ostream& os,
                                      LinuxMallInfo const& info);
    };

    inline std::ostream&
    operator<<(std::ostream& os, LinuxMallInfo const& info)
    {
      auto const& minfo = info.minfo_;
      os << " HEAP-ARENA [ SIZE-BYTES " << minfo.arena << " N-UNUSED-CHUNKS "
         << minfo.ordblks << " TOP-FREE-BYTES " << minfo.keepcost << " ]"
         << " HEAP-MAPPED [ SIZE-BYTES " << minfo.hblkhd << " N-CHUNKS "
         << minfo.hblks << " ]"
         << " HEAP-USED-BYTES " << minfo.uordblks << " HEAP-UNUSED-BYTES "
         << minfo.fordblks;
      return os;
    }

  } // namespace detail

} // namespace art
#endif /* art_Framework_Services_Optional_detail_LinuxMallInfo_h */

// Local variables:
// mode:c++
// End:
