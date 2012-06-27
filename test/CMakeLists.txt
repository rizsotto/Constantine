# Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  @ONLY)

add_custom_target(check
  COMMAND ${LLVM_LIT} .
  COMMENT "Running regression tests")
add_dependencies(check
  Medve)
