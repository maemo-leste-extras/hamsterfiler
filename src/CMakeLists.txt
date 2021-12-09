set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

# the source files
file(GLOB SOURCE_FILES
        "*.h"
        "*.cpp"
        )

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui Svg DBus)
if(MAEMO)
    find_package(Qt5 REQUIRED COMPONENTS Maemo5)
endif()

qt5_add_resources(RESOURCES
        assets.qrc
        )

if(NOT MAEMO)
    qt5_add_resources(RESOURCES assets_localdev.qrc)
endif()

set(EXECUTABLE_FLAG)

add_executable(hamsterfiler ${EXECUTABLE_FLAG} main.cpp
        ${SOURCE_FILES}
        ${RESOURCES}
        )

set_property(TARGET hamsterfiler PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_include_directories(hamsterfiler PUBLIC ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

if(MAEMO)
    # @TODO: should not be necessary, but definition `Q_WS_MAEMO` is *not* available via CMake
    target_compile_definitions(hamsterfiler PRIVATE MAEMO=1)
endif()

target_include_directories(hamsterfiler PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${X11_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Gui_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${Qt5Svg_INCLUDE_DIRS}
        ${Qt5DBus_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
        ${GLib_INCLUDE_DIRS}
        ${LIBDBUS_INCLUDE_DIRS}
        ${LIBDBUS_GLIB_INCLUDE_DIRS}
        ${LIBGDKPIXBUF_INCLUDE_DIRS}
        ${LIBHILDON_INCLUDE_DIRS}
        ${LIBHILDONMIME_INCLUDE_DIRS}
        ${LIBHILDONTHUMB_INCLUDE_DIRS}
        )

add_definitions(${QT_DEFINITIONS})
if(NOT "${CMAKE_BUILD_TYPE}" EQUAL "Debug")
    add_definitions(-DQT_NO_DEBUG)
endif()

target_compile_definitions(hamsterfiler PRIVATE HAS_QML=1)

target_compile_definitions(hamsterfiler
        PUBLIC
        ${Qt5Core_DEFINITIONS}
        ${Qt5Widgets_DEFINITIONS}
        ${Qt5Gui_DEFINITIONS}
        ${Qt5Network_DEFINITIONS}
        ${Qt5Svg_DEFINITIONS}
        ${Qt5DBus_DEFINITIONS}
        ${Qt5Xml_DEFINITIONS}
        ${GLIB_DEFINITIONS}
        )

set(CMAKE_CXX_FLAGS
        ${CMAKE_CXX_FLAGS}
        ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS})

target_link_libraries(hamsterfiler PUBLIC
        ${CMAKE_DL_LIBS}
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::Svg
        Qt5::DBus
        Qt5::QSvgIconPlugin
        Qt5::QSvgPlugin
        Threads::Threads
        ${GLIB_LIBRARY}
        ${GLIB_GIO_LIBRARIES}
        ${LIBDBUS_LIBRARIES}
        ${LIBDBUS_GLIB_LIBRARIES}
        ${LIBGDKPIXBUF_LIBRARIES}
        ${LIBHILDON_LIBRARIES}
        ${LIBHILDONMIME_LIBRARIES}
        ${LIBHILDONTHUMB_LIBRARIES}
        )

if(MAEMO)
    target_link_libraries(hamsterfiler PUBLIC Qt5::Maemo5)
endif()

if(X11_FOUND)
    target_link_libraries(hamsterfiler PUBLIC ${X11_LIBRARIES})
endif()

install(TARGETS hamsterfiler
        DESTINATION bin
        )


message(STATUS "=============================================")
message(STATUS "VERSION_MAJOR: ${VERSION_MAJOR}")
message(STATUS "VERSION_MINOR: ${VERSION_MINOR}")
message(STATUS "VERSION_REVISION: ${VERSION_REVISION}")
message(STATUS "VERSION: ${VERSION}")
message(STATUS "GLib: ${GLIB_VERSION}")
message(STATUS "Hildon: ${LIBHILDON_VERSION}")
message(STATUS "HildonMime: ${LIBHILDONMIME_VERSION}")
message(STATUS "This build is for Maemo: ${MAEMO}")
message(STATUS "=============================================")
