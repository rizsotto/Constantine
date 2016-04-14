cmake_minimum_required(VERSION 2.8)

project(constantine CXX)
set(CONSTANTINE_VERSION "3.7")

set(CPACK_PACKAGE_NAME "constantine")
set(CPACK_PACKAGE_CONTACT "László Nagy")
set(CPACK_PACKAGE_VENDOR ${CPACK_PACKAGE_CONTACT})
set(CPACK_PACKAGE_VERSION ${CONSTANTINE_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A plugin for Clang compiler")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_RPM_PACKAGE_RELEASE "1%{?dist}")
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
set(CPACK_RPM_PACKAGE_GROUP "Development/Tools")
set(CPACK_RPM_PACKAGE_URL "http://github.com/rizsotto/Constantine")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Constantine implements pseudo const analysis.")
include(CPack)

include(GNUInstallDirs)
install(FILES COPYING README.md
  DESTINATION ${CMAKE_INSTALL_DOCDIR})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(Clang REQUIRED)
find_package(Lit)

add_subdirectory(sources)
add_subdirectory(test)
