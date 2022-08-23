cet_register_export_set(SET_NAME ServicePluginType NAMESPACE art_plugin_types)

add_subdirectory(Registry)
add_subdirectory(FileServiceInterfaces)

# Need to switch to a different default export set so services can depend on
# art_plugin_types::service(Declaration|Definition).
cet_register_export_set(SET_NAME Services SET_DEFAULT)

# Make service building available.
include(art::service)

add_subdirectory(System)
add_subdirectory(Optional)
