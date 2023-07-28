cmake_minimum_required(VERSION 3.25)
project(app_retometr)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(PUGIXML REQUIRED pugixml)
pkg_check_modules(GLIB REQUIRED glib-2.0)

include_directories(${GTK3_INCLUDE_DIRS} ${PUGIXML_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS} ${PUGIXML_LIBRARY_DIRS} ${GLIB_INCLUDE_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER} ${PUGIXML_CFLAGS_OTHER} ${GLIB_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-rdynamic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")

add_executable(app_retometr main.cpp)

target_link_libraries(app_retometr ${GTK3_LIBRARIES} ${PUGIXML_LIBRARIES} ${GLIB_LIBRARIES} stdc++)

add_custom_target(run_app_retometr
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/app_retometr
        DEPENDS app_retometr
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
        )

add_custom_target(clean_app_retometr
        COMMAND rm -f ${CMAKE_CURRENT_BINARY_DIR}/app_retometr
        DEPENDS app_retometr
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
        )

