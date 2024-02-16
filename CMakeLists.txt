project(qlivestyleeditor)
cmake_minimum_required(VERSION 3.5.0)

find_package(
    Qt5
    COMPONENTS
        Widgets
    REQUIRED
)

set(CMAKE_AUTOMOC ON)
set(MOCUIC_DIR ${CMAKE_CURRENT_BINARY_DIR}/.mocuic)

add_executable(
    qlivestyleeditor
        demo/demo.cpp
        demo/demomainwindow.cpp
        src/qlivestyleeditor.cpp
)

target_include_directories(
    qlivestyleeditor
    PRIVATE
        src/
)

set_target_properties(
    qlivestyleeditor
    PROPERTIES
        AUTOGEN_BUILD_DIR
            ${MOCUIC_DIR}
)

target_link_libraries(
    qlivestyleeditor
        Qt5::Widgets
)
