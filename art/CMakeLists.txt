# project name
project(artCode)

# cmake will automatically order the library builds according to declared dependencies
subdirs (Version)
subdirs (Utilities)
subdirs (ParameterSet)
subdirs (Framework)
subdirs (Persistency)
