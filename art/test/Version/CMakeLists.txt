configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_GetReleaseVersion.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/test_GetReleaseVersion.cpp @ONLY
  )

cet_test(GetReleaseVersion
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/test_GetReleaseVersion.cpp
  LIBRARIES art_Version
  )
