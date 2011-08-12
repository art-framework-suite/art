# project name
project(artCode)

# cmake will automatically order the library builds according to declared dependencies
subdirs (Version)
subdirs (Utilities)
subdirs (Framework)
subdirs (Persistency)
