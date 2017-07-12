include (CheckFloatPrecision.cmake)
check_float_precision()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xpfpaconfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/xpfpaconfig.h
  @ONLY)

install_headers()
install_source()
