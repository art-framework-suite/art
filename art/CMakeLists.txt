include(ArtMake)
include(BuildPlugins)
include(CetMakeCompletions)

cet_find_package(Boost COMPONENTS date_time graph program_options regex REQUIRED)
cet_find_package(Boost PRIVATE COMPONENTS filesystem headers thread unit_test_framework REQUIRED)
cet_find_package(CLHEP COMPONENTS Random Vector REQUIRED)
cet_find_package(Range PRIVATE REQUIRED)
cet_find_package(RapidJSON PRIVATE REQUIRED)
cet_find_package(TBB REQUIRED)
cet_find_package(canvas REQUIRED)
cet_find_package(cetlib REQUIRED)
cet_find_package(cetlib_except REQUIRED)
cet_find_package(fhiclcpp REQUIRED)
cet_find_package(hep_concurrency REQUIRED)
cet_find_package(messagefacility REQUIRED)

# Register export sets here in dependency order to avoid issues.
cet_register_export_set(SET_NAME ServicePluginType NAMESPACE art_plugin_types)
cet_register_export_set(SET_NAME Services NAMESPACE art)
cet_register_export_set(SET_NAME ServiceUsers NAMESPACE art)
cet_register_export_set(SET_NAME PluginSupport NAMESPACE art_plugin_support)
cet_register_export_set(SET_NAME PluginTypes NAMESPACE art_plugin_types)

add_subdirectory(Version)
add_subdirectory(Utilities)
add_subdirectory(Persistency)
add_subdirectory(Framework)

# Testing
add_subdirectory(test)
