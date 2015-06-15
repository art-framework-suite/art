#ifndef art_Framework_Services_Optional_detail_LinuxProcData_h
#define art_Framework_Services_Optional_detail_LinuxProcData_h

//===================================================================
//
// LinuxProcData
//
//-----------------------------------------------
//
// This is a helper class for storing information that is available
// via 'cat /proc/PID/stat'.  As of SLF6.5 (Ramsey), the values from
// that command correspond to (see 'man 5 proc'):
//
//        Property              type   C++ type
//     ===============================================
//     1. pid                    %d    int
//     2. comm                   %s    char[400]
//     3. state                  %c    char
//     4. ppid                   %d    int
//     5. pgrp                   %d    int
//     6. session                %d    int
//     7. tty_nr                 %d    int
//     8. tpgid                  %d    int
//     9. flags                  %u    unsigned
//    10. minflt                 %lu   unsigned long
//    11. cminflt                %lu   unsigned long
//    12. majflt                 %lu   unsigned long
//    13. cmajflt                %lu   unsigned long
//    14. utime                  %lu   unsigned long
//    15. stime                  %lu   unsigned long
//    16. cutime                 %ld   long
//    17. cstime                 %ld   long
//    18. priority               %ld   long
//    19. nice                   %ld   long
//    20. num_threads            %ld   long
//    21. itrealvalue            %ld   long
//    22. starttime              %llu  unsigned long long
//    23. vsize                  %lu   unsigned long
//    24. rss                    %ld   long
//    25. rsslim                 %lu   unsigned long
//    26. startcode              %lu   unsigned long
//    27. endcode                %lu   unsigned long
//    28. startstack             %lu   unsigned long
//    29. kstkesp                %lu   unsigned long
//    30. kstkeip                %lu   unsigned long
//    31. signal                 %lu   unsigned long
//    32. blocked                %lu   unsigned long
//    33. sigignore              %lu   unsigned long
//    34. sigcatch               %lu   unsigned long
//    35. wchan                  %lu   unsigned long
//    36. nswap                  %lu   unsigned long
//    37. cnswap                 %lu   unsigned long
//    38. exit_signal            %d    int
//    39. processor              %d    int
//    40. rt_priority            %u    unsigned
//    41. policy                 %u    unsigned
//    42. delayacct_blkio_ticks  %llu  unsigned long long
//    43. guest_time             %lu   unsigned long
//    44. cguest_time            %ld   long
//
// Currently, we are interested only in 'vsize' and 'rss'.
//
//===================================================================

#include <array>
#include <sstream>

namespace art {
  namespace detail {

    //================================================================
    struct LinuxProcData {

      // supported procfs types
      enum procfs_type{ VSIZE, RSS, ntypes };

      // aliases
      using vsize_t    = unsigned long;
      using rss_t      = long;

      using proc_array = std::array<double,ntypes>;

      // constants
      static constexpr double kB = 1024.;
      static constexpr double MB = kB*kB;

    };

    // operator overloads for std::array arithmetic
    // ... must type 'using namespace art::detail' to use

    inline bool operator >
    ( LinuxProcData::proc_array const & left,
      LinuxProcData::proc_array const & right ) {
      for ( unsigned i(0) ; i < LinuxProcData::ntypes ; ++i ) {
        if ( left.at(i) > right.at(i) ) return true;
      }
      return false;
    }

    inline
    bool operator <= ( LinuxProcData::proc_array const & left,
                       LinuxProcData::proc_array const & right ) {
      return !( left > right );
    }

    inline
    LinuxProcData::proc_array operator- (LinuxProcData::proc_array const & left,
                                         LinuxProcData::proc_array const & right) {
      LinuxProcData::proc_array tmp = {0.};
      for ( unsigned i(0) ; i < LinuxProcData::ntypes ; ++i ) {
        tmp.at(i) = left.at(i) - right.at(i);
      }
      return tmp;
    }

    inline
    LinuxProcData::proc_array& operator+= (LinuxProcData::proc_array& left,
                                           LinuxProcData::proc_array const & right) {
      for ( unsigned i(0) ; i < LinuxProcData::ntypes ; ++i ) {
        left.at(i) += right.at(i);
      }
      return left;
    }

  }
}
#endif // art_Framework_Services_Optional_detail_LinuxProcData_h

// Local variables:
// mode:c++
// End:
