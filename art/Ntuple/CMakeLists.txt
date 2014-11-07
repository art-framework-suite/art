include(BuildPlugins)

art_make(
  art_Ntuple
  LIB_LIBRARIES
  ${SQLITE3}
  EXCLUDE sqlite_mfPlugin.cc
  )

simple_plugin( sqlite mfPlugin
               art_Ntuple
               ${SQLITE3}
               ${MF_MESSAGELOGGER}
               ${MF_UTILITIES}
               ${FHICLCPP}
               ${CETLIB}
               )

install_headers()
install_source()
