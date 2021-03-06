cmake_minimum_required (VERSION 2.6)
project (Contact)

# Path extra package
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Packages
find_package(Doxygen)
find_package(cmocka)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Options
option(DEVEL "Enable devel mode" OFF)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

# Set flags
if(DEVEL)
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer")
    else()
        message( FATAL_ERROR "In devel mode, you must use clang." )
    endif()
endif()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Weverything")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall --pedantic -pedantic-errors --std=c11 -Werror")


# Gtk
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})


# Build shared libraries.
add_library(contact SHARED src/main.c)
target_link_libraries(contact ${GTK3_LIBRARIES})

# Build
add_executable(contact-bin src/main.c)
target_link_libraries(contact-bin contact)

###
# TEST
###
enable_testing()
add_executable(test-main src/main_test.c)
target_link_libraries(test-main contact ${CMOCKA_LIBRARY})
add_test(test-main test-main)


###
# DOCUMENTATION
###
if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_result "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()
