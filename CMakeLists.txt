cmake_minimum_required(VERSION 3.16)
project(cutienetworking VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick DBus)
qt_standard_project_setup()

set(lib_SOURCES
    src/cutienetworking.h
    src/mobilenetwork/mobilenetwork.cpp
    src/wifi/cutienetworkconnection.cpp
    src/wifi/wifiaccesspoint.cpp
    src/wifi/wifisettings.cpp)

set(plugin_SOURCES
    src/plugin.cpp src/plugin.h)

qt6_add_library(cutienetworking
    SHARED ${lib_SOURCES}
)

qt6_add_qml_module(qmlcutienetworkingplugin
    URI Cutie.Networking
    VERSION ${PROJECT_VERSION}
    PLUGIN_TARGET qmlcutienetworkingplugin
    NO_GENERATE_PLUGIN_SOURCE
    SOURCES
        ${plugin_SOURCES}
)

set_target_properties(cutienetworking PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(cutienetworking PROPERTIES SOVERSION 0)
set_target_properties(cutienetworking PROPERTIES PUBLIC_HEADER src/cutienetworking.h)

target_link_libraries(cutienetworking PUBLIC
    Qt::Core
    Qt::Quick
    Qt::DBus
)

target_link_libraries(qmlcutienetworkingplugin PUBLIC
    Qt::Core
    Qt::Quick
    cutienetworking
)

configure_file(src/cutienetworking.pc.in cutienetworking.pc @ONLY)

install(TARGETS cutienetworking
    RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    BUNDLE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(FILES ${CMAKE_BINARY_DIR}/cutienetworking.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

install(TARGETS qmlcutienetworkingplugin
    RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt6/qml/Cutie/Networking"
    BUNDLE DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt6/qml/Cutie/Networking"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt6/qml/Cutie/Networking"
)

install(DIRECTORY src/qml/Cutie/Networking DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt6/qml/Cutie")

