# project name
project(IOPool)

# version number
set (IOPool_VERSION_MAJOR 1)
set (IOPool_VERSION_MINOR 0)

# build IOPool libraries
subdirs (Common/src)
subdirs (Input/src)
subdirs (Output/src)

# build IOPool tests
subdirs (Common/test)
subdirs (Input/test)
subdirs (Output/test)
