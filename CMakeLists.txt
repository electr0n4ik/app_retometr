Acmake_minimum_required(VERSION 3.25)

project(app_retometr)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(GDK_PIXBUF REQUIRED gdk-pixbuf-2.0)
pkg_check_modules(PUGIXML REQUIRED pugixml)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(XLNT REQUIRED xlnt)

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
    target_link_libraries(app_retometr PRIVATE -Wl,--no-console) #убирает окно консоли
endif()

add_executable(app_retometr main.cpp)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-all-symbols")
    #set_target_properties(app_retometr PROPERTIES WIN32_EXECUTABLE TRUE) #убирает окно консоли
endif()

target_compile_options(app_retometr PRIVATE -g)

target_link_libraries(app_retometr PRIVATE ${GTK3_LIBRARIES})
target_link_libraries(app_retometr PRIVATE ${PUGIXML_LIBRARIES})
target_link_libraries(app_retometr PRIVATE ${GLIB_LIBRARIES})
target_link_libraries(app_retometr PRIVATE ${XLNT_LIBRARIES})
target_link_libraries(app_retometr PRIVATE ${GDK_PIXBUF_LIBRARIES})

target_include_directories(app_retometr PRIVATE ${GTK3_INCLUDE_DIRS})
target_include_directories(app_retometr PRIVATE ${PUGIXML_INCLUDE_DIRS})
target_include_directories(app_retometr PRIVATE ${GLIB_INCLUDE_DIRS})
target_include_directories(app_retometr PRIVATE ${XLNT_INCLUDE_DIRS})
target_include_directories(app_retometr PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/glade.glade
    ${CMAKE_CURRENT_BINARY_DIR}/glade.glade
    @ONLY
    )

include(CPack)
