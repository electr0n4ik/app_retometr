cmake_minimum_required(VERSION 3.25)

project(app_retometr)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(PUGIXML REQUIRED pugixml)
pkg_check_modules(GLIB REQUIRED glib-2.0)
#pkg_check_modules(XLSXWRITER REQUIRED xlsxwriter)
pkg_check_modules(XLNT REQUIRED xlnt)

set(CMAKE_CXX_STANDARD 17)

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
endif()

#if (WIN32)
#    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-all-symbols")
#endif()

#if (WIN32)
#    set_target_properties(app_retometr PROPERTIES WIN32_EXECUTABLE TRUE) #убирает окно консоли
#endif()

#if (UNIX)
#    target_link_libraries(app_retometr PRIVATE -Wl,--no-console) #убирает окно консоли
#endif()

add_executable(app_retometr main.cpp)

target_include_directories(app_retometr PUBLIC ${GTK3_INCLUDE_DIRS})
target_include_directories(app_retometr PUBLIC ${PUGIXML_INCLUDE_DIRS})
target_include_directories(app_retometr PUBLIC ${GLIB_INCLUDE_DIRS})
target_include_directories(app_retometr PUBLIC ${XLNT_INCLUDE_DIRS})
#target_include_directories(app_retometr PUBLIC ${XLSXWRITER_INCLUDE_DIRS})

target_link_directories(app_retometr PUBLIC ${GTK3_LIBRARY_DIRS})
target_link_directories(app_retometr PUBLIC ${PUGIXML_LIBRARY_DIRS})
target_link_directories(app_retometr PUBLIC ${GLIB_LIBRARY_DIRS})
target_link_directories(app_retometr PUBLIC ${XLNT_LIBRARY_DIRS})
#target_link_directories(app_retometr PUBLIC ${XLSXWRITER_LIBRARY_DIRS})

target_link_libraries(app_retometr PUBLIC ${GTK3_LIBRARIES})
target_link_libraries(app_retometr PUBLIC ${PUGIXML_LIBRARIES})
target_link_libraries(app_retometr PUBLIC ${GLIB_LIBRARIES})
target_link_libraries(app_retometr PUBLIC ${XLNT_LIBRARIES})
target_link_libraries(app_retometr PUBLIC xlnt)
target_link_libraries(app_retometr PUBLIC stdc++)
#target_link_libraries(app_retometr PUBLIC ${XLSXWRITER_LIBRARIES})
