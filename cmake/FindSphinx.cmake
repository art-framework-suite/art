#find_program(SPHINX_EXECUTABLE NAMES sphinx-build-2.7
find_program(SPHINX_EXECUTABLE NAMES sphinx-build
    HINTS
    ENV PATH
    #$ENV{SPHINX_DIR}
    PATH_SUFFIXES bin
    DOC "Sphinx documentation generator"
)

include(FindPackageHandleStandardArgs)

find_package(PythonInterp)

find_package_handle_standard_args(Sphinx DEFAULT_MSG
    SPHINX_EXECUTABLE
)

mark_as_advanced(SPHINX_EXECUTABLE)
