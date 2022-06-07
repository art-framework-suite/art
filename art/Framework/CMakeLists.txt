add_subdirectory(Services)
add_subdirectory(Principal)

# Need to switch to a different export set so we can use services.
cet_register_export_set(SET_NAME ServiceUsers SET_DEFAULT)
cet_register_export_set(SET_NAME PluginTypes NAMESPACE art_plugin_types)

add_subdirectory(Core)
add_subdirectory(IO)
add_subdirectory(EventProcessor)
add_subdirectory(Art)
add_subdirectory(Modules)
