# Set the minimum version of cmake required to build this project
cmake_minimum_required(VERSION 3.25)

# Thanks to @danger89 and @Ilothar for updating the gist.
# Set the name and the supported language of the project
project(app_retometr C CXX)

# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
# pkg_check_modules(GDK_PIXBUF REQUIRED gdk-pixbuf-2.0)
# pkg_check_modules(PUGIXML REQUIRED pugixml)
# pkg_check_modules(GLIB REQUIRED glib-2.0)
# pkg_check_modules(XLNT REQUIRED xlnt)

add_executable(app_retometr main.cpp)


target_link_libraries(app_retometr PRIVATE ${GTK3_LIBRARIES})
# target_link_libraries(app_retometr PRIVATE ${PUGIXML_LIBRARIES})
# target_link_libraries(app_retometr PRIVATE ${GLIB_LIBRARIES})
# target_link_libraries(app_retometr PRIVATE ${XLNT_LIBRARIES})
# target_link_libraries(app_retometr PRIVATE ${GDK_PIXBUF_LIBRARIES})


target_include_directories(app_retometr PRIVATE ${GTK3_INCLUDE_DIRS})
# target_include_directories(app_retometr PRIVATE ${PUGIXML_INCLUDE_DIRS})
# target_include_directories(app_retometr PRIVATE ${GLIB_INCLUDE_DIRS})
# target_include_directories(app_retometr PRIVATE ${XLNT_INCLUDE_DIRS})
# target_include_directories(app_retometr PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS})

# target_link_directories(app_retometr PRIVATE ${GTK3_LIBRARY_DIRS})
# target_link_directories(app_retometr PRIVATE ${PUGIXML_LIBRARY_DIRS})
# target_link_directories(app_retometr PRIVATE ${GLIB_LIBRARY_DIRS})
# target_link_directories(app_retometr PRIVATE ${XLNT_LIBRARY_DIRS})
# target_link_directories(app_retometr PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS})

# target_compile_options(app_retometr PRIVATE ${GTK3_CFLAGS_OTHER})
# target_compile_options(app_retometr PRIVATE ${PUGIXML_CFLAGS_OTHER})
# target_compile_options(app_retometr PRIVATE ${GLIB_CFLAGS_OTHER})
# target_compile_options(app_retometr PRIVATE ${XLNT_CFLAGS_OTHER})
# target_compile_options(app_retometr PRIVATE ${GDK_PIXBU_CFLAGS_OTHER})


# if(UNIX AND NOT APPLE)
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -rdynamic")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
#     target_link_libraries(app_retometr PRIVATE -Wl,--no-console) #убирает окно консоли
# endif()

# if (WIN32)
#     set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-all-symbols")
#     set_target_properties(app_retometr PROPERTIES WIN32_EXECUTABLE TRUE) #убирает окно консоли
# endif()

install(TARGETS app_retometr RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
include(InstallRequiredSystemLibraries)
include(GNUInstallDirs)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
    @ONLY
    )

install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake)

include(CPack)

