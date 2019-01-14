find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

art_make_library(
  SOURCE
    ensureTable.cc
    ExceptionMessages.cc
    Globals.cc
    LinuxProcMgr.cc
    MallocOpts.cc
    parent_path.cc
    PluginSuffixes.cc
    RegexMatch.cc
    ScheduleID.cc
    SharedResource.cc
    SharedResourcesRegistry.cc
    unique_filename.cc
    UnixSignalHandlers.cc
    $<$<PLATFORM_ID:Linux>:LinuxProcMgr.cc>
  LIBRARIES
    canvas
    MF_MessageLogger
    fhiclcpp
    cetlib
    cetlib_except
    hep_concurrency
    ${Boost_FILESYSTEM_LIBRARY}
    Threads::Threads)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
