if (LIT_FOUND)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
    ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
    @ONLY)

  add_custom_target(check
    COMMAND ${LIT_EXECUTABLE} -v .
    COMMENT "Running regression tests")
  add_dependencies(check constantine debug)
else()
  message(STATUS "Lit was not found, skip to run tests")
endif()
