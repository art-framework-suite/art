include(ArtMake)
include(BuildPlugins)
include(CetMakeCompletions)

find_package(Boost COMPONENTS date_time graph program_options regex REQUIRED PUBLIC)
find_package(Boost PRIVATE COMPONENTS filesystem headers thread unit_test_framework json REQUIRED)
find_package(CLHEP COMPONENTS Random Vector REQUIRED PUBLIC)
find_package(Range PRIVATE REQUIRED)
find_package(TBB REQUIRED PUBLIC)
find_package(canvas REQUIRED PUBLIC)
find_package(cetlib REQUIRED PUBLIC)
find_package(cetlib_except REQUIRED PUBLIC)
find_package(fhiclcpp REQUIRED PUBLIC)
find_package(hep_concurrency REQUIRED PUBLIC)
find_package(messagefacility REQUIRED PUBLIC)

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
