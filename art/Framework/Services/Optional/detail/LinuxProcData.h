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

#include <sstream>
#include <tuple>
#include <type_traits>

namespace art {
  namespace detail {

    //================================================================
    struct LinuxProcData {

      // supported procfs types
      enum procfs_type{VSIZE, RSS, ntypes};

      // aliases
      struct proc_type {};
      struct vsize_t : proc_type {
        using value_type = unsigned long;
        explicit vsize_t(value_type const v) : value{v} {}
        value_type value;
      };

      struct rss_t : proc_type {
        using value_type = long;
        explicit rss_t(value_type const v) : value{v} {}
        value_type value;
      };

      using proc_tuple = std::tuple<vsize_t,rss_t>;

      static auto make_proc_tuple(vsize_t::value_type const vsize = {}, rss_t::value_type const rss = {})
      {
        return proc_tuple{vsize_t{vsize}, rss_t{rss}};
      }

      template <typename T>
      static
      std::enable_if_t<std::is_base_of<proc_type,T>::value, double>
      getValueInMB(proc_tuple const& t)
      {
        // Info from proc is in bytes; convert to base-10 MB.
        return std::get<T>(t).value/MB;
      }

      // constants
      static constexpr double KB {1000.};
      static constexpr double KiB {1.024*KB};
      static constexpr double MB {KB*KB};
      static constexpr double MiB {KiB*KiB};
    };

  }
}
#endif /* art_Framework_Services_Optional_detail_LinuxProcData_h */

// Local variables:
// mode:c++
// End:
