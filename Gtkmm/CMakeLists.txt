# Set the minimum version of cmake required to build this project
cmake_minimum_required(VERSION 3.10)

# Thanks to @danger89 and @Ilothar for updating the gist.
# Set the name and the supported language of the project
project(Gtkmm C CXX)

# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)


add_executable(Gtkmm WIN32 Gtkmm.cpp)
target_link_libraries(Gtkmm PRIVATE ${GTKMM_LIBRARIES})


# Add other flags to the compiler
target_compile_definitions(Gtkmm PRIVATE ${GTKMM_CFLAGS_OTHER})

# target_link_options(Gtkmm PRIVATE -mwindows)


# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
target_include_directories(Gtkmm PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(Gtkmm PRIVATE ${GTKMM_LIBRARY_DIRS})

install(TARGETS Gtkmm RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
include(InstallRequiredSystemLibraries)
include(GNUInstallDirs)

# To install other/non-system 3rd party required libraries:
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
  @ONLY
  )

install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake)

include(CPack)