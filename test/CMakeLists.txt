# This file is distributed under MIT-LICENSE. See COPYING for details.

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  @ONLY)

add_custom_target(check
  COMMAND ${LLVM_LIT} -v .
  COMMENT "Running regression tests")
add_dependencies(check
  constantine)
