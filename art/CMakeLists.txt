include(ArtMake)
include(CetMakeCompletions)

# cmake will automatically order the library builds according to declared dependencies
add_subdirectory(Version)
add_subdirectory(Utilities)
add_subdirectory(Framework)
add_subdirectory(Persistency)

# Testing
add_subdirectory(test)
