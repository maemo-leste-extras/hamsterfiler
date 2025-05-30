cmake_minimum_required(VERSION 3.13)
project(hamsterfiler)
message(STATUS "Initiating compile using CMake ${CMAKE_VERSION}")

set(VERSION_MAJOR "0")
set(VERSION_MINOR "8")
set(VERSION_REVISION "0")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}")

# add `cmake` directory as cmake module path
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")

set(THREADS_PREFER_PTHREAD_FLAG ON)
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)  # 'signals' in `gdbusintrospection.h` conflicts with Qt's 'signals'
set(HAMSTERFILER_DESKTOP_DIR /usr/share/applications/hildon)

configure_file("cmake/config-hamsterfiler.h.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/src/config-hamsterfiler.h")

set(CMAKE_CXX_STANDARD 14)
include(FindCcache)
include(CheckIncludeFiles)
include(CheckFunctionExists)
#include(CheckSymbolExists)

find_package(PkgConfig REQUIRED)
find_package(GLIB REQUIRED COMPONENTS gio)

pkg_search_module(LIBHILDON hildon-1)
pkg_search_module(LIBHILDONMIME libhildonmime)
pkg_search_module(LIBHILDONTHUMB hildon-thumbnail)
pkg_search_module(LIBDBUS REQUIRED dbus-1)
pkg_check_modules(LIBDBUS_GLIB REQUIRED dbus-glib-1)
pkg_search_module(LIBGDKPIXBUF REQUIRED gdk-pixbuf-2.0)

if(DEFINED LIBHILDON_LIBRARIES)
    set(MAEMO ON)
endif()

# Find X11
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_SKIP_RPATH ON)
endif()

find_package(X11 REQUIRED)
message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

include_directories(${X11_INCLUDE_DIR})
link_directories(${X11_LIBRARIES})

# Find pthread
find_package(Threads REQUIRED)

add_subdirectory(src)
add_subdirectory(desktop)
