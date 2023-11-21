cmake_minimum_required(VERSION 3.25)

project(hello)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

add_executable(hello hello.cpp)

target_include_directories(hello PUBLIC ${GTK3_INCLUDE_DIRS})
target_include_directories(hello PUBLIC ${GTKMM_INCLUDE_DIRS})

target_link_directories(hello PUBLIC ${GTK3_LIBRARY_DIRS})
target_link_directories(hello PUBLIC ${GTKMM_LIBRARY_DIRS})

target_link_libraries(hello PUBLIC ${GTK3_LIBRARIES})
target_link_libraries(hello PUBLIC ${GTKMM_LIBRARIES})


