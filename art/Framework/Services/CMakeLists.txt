add_subdirectory(Registry)
add_subdirectory(FileServiceInterfaces)

# Need to switch to a different export set so services can depend on
# art_plugin_types::service(Declaration|Definition).
cet_register_export_set(SET_NAME Services SET_DEFAULT)
add_subdirectory(System)
add_subdirectory(Optional)
