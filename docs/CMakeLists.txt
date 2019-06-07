find_package(Sphinx REQUIRED)
find_ups_product(python)# v3_7_2)
 
if(NOT DEFINED SPHINX_THEME)
    set(SPHINX_THEME default)
endif()
 
if(NOT DEFINED SPHINX_THEME_DIR)
    set(SPHINX_THEME_DIR)
endif()

# configured documentation tools and intermediate build results
#set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")

# Sphinx cache with pickled ReST documents
set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

# HTML output directory
# --- doing a bit of extra work here. Why?
#        + we want to see what directory are people putting their docs
#          do we want them to make and environement variable for that?
#        + once we know that this dir exists, we want to check does the 
#          product dir exist there (e.g. all_docs/art exists)
#          if not then we should probably make it ourselves
#        + and then we do the same for the version being developed
#          the goal is for the all_docs dir to look similar to scisoft
#          in directory structure.

#set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")
#set(SPHINX_HTML_DIR "$ENV{MRB_TOP}/all_docs/art")
#set(SPHINX_HTML_DIR "$ENV{MRB_TOP}/all_docs/${PROJECT_NAME}/$ENV{}")
#message("\n\n\n\n\n\n")

set(TOP_DOCS_DIR "$ENV{MRB_TOP}/all_docs")

# looking for top level dir
if(EXISTS "${TOP_DOCS_DIR}")
  #message("I can use exists this way. all_docs is there.")
  set(PROJ_DOCS_DIR "${TOP_DOCS_DIR}/${PROJECT_NAME}")
  

  # looking for project dir
  if(EXISTS "${PROJ_DOCS_DIR}")
    #message("art dir is here")
    string(TOUPPER ${PROJECT_NAME} ENV_PROJ_NAME)
    set(VERSION_DOCS_DIR "${PROJ_DOCS_DIR}/$ENV{${ENV_PROJ_NAME}_VERSION}")
    set(VERSION_NUMBER $ENV{${ENV_PROJ_NAME}_VERSION})
    #message("looking for.. ${DUMB_TEMP_VAR}")
    #if(EXISTS "$ENV{MRB_TOP}/all_docs/${PROJECT_NAME}/$ENV{${ENV_PROJ_NAME}_VERSION}")

    if(EXISTS "${VERSION_DOCS_DIR}")
      #set(DUMB_TEMP_VAR "$ENV{MRB_TOP}/all_docs/${PROJECT_NAME}/$ENV{${ENV_PROJ_NAME}_VERSION}")
      #message("version dir is here")
    else()
      #message("You don't have a version dir")
      add_custom_target(build_art_version_dir ALL COMMAND 
                        ${CMAKE_COMMAND} -E make_directory 
                        ${VERSION_DOCS_DIR})
    endif()
  else()
    #message("You don't have a art dir")
    add_custom_target(build_art_proj_dir ALL COMMAND 
                      ${CMAKE_COMMAND} -E make_directory 
                      ${PROJ_DOCS_DIR})
    
    # Going back through now assuming it exists ! 
    #message("art dir is here")
    string(TOUPPER ${PROJECT_NAME} ENV_PROJ_NAME)
    set(VERSION_DOCS_DIR "${PROJ_DOCS_DIR}/$ENV{${ENV_PROJ_NAME}_VERSION}")
    set(VERSION_NUMBER $ENV{${ENV_PROJ_NAME}_VERSION})
    #message("looking for.. ${DUMB_TEMP_VAR}")
    #if(EXISTS "$ENV{MRB_TOP}/all_docs/${PROJECT_NAME}/$ENV{${ENV_PROJ_NAME}_VERSION}")

    if(EXISTS "${VERSION_DOCS_DIR}")
      #set(DUMB_TEMP_VAR "$ENV{MRB_TOP}/all_docs/${PROJECT_NAME}/$ENV{${ENV_PROJ_NAME}_VERSION}")
      #message("version dir is here")
    else()
      #message("You don't have a version dir")
      add_custom_target(build_art_version_dir ALL COMMAND 
                        ${CMAKE_COMMAND} -E make_directory 
                        ${VERSION_DOCS_DIR})
    endif() 
  endif()
else()
  #message("You don't a spot to put your docs!")
endif()
  


#message("\n\n\n\n\n\n")

set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${VERSION_NUMBER}")
message("BINARY_BUILD_DIR: ${BINARY_BUILD_DIR}")
set(SPHINX_HTML_DIR "${BINARY_BUILD_DIR}")




file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/_static/ DESTINATION ${BINARY_BUILD_DIR}/_static)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/_templates/ DESTINATION ${BINARY_BUILD_DIR}/_templates)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/_extensions/ DESTINATION ${BINARY_BUILD_DIR}/_extensions)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
    "${BINARY_BUILD_DIR}/conf.py"
    @ONLY)

#message(STATUS "SPHINX_EXECUTABLE: ${SPHINX_EXECUTABLE}")

add_custom_target(art_docs ALL
  ${SPHINX_EXECUTABLE}
  -q -b html
  -c "${BINARY_BUILD_DIR}"
  -d "${SPHINX_CACHE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${SPHINX_HTML_DIR}"
  COMMENT "Building HTML documentation with Sphinx"
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/_extensions/redmineRoles.py)

# Install the built html files to an instal dir (HTML DIR)

install(DIRECTORY "${BINARY_BUILD_DIR}" 
        #DESTINATION "${VERSION_DOCS_DIR}"
        DESTINATION "${PROJ_DOCS_DIR}"
        PATTERN "${BINARY_BUILD_DIR}/*")


# EOF
