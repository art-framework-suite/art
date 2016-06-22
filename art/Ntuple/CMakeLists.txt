include(BuildPlugins)

art_make(
  art_Ntuple
  LIB_LIBRARIES
  art_Utilities canvas_Utilities
  ${SQLITE3}
  EXCLUDE sqlite_mfPlugin.cc
  )

simple_plugin( sqlite mfPlugin
               art_Ntuple
               ${SQLITE3}
               MF_MessageService
               MF_Utilities
               cetlib
               fhiclcpp
               )

install_headers()
install_source()
