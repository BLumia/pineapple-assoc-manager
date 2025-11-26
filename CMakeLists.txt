# SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.23)

project(pineapple-assoc-manager
    VERSION 0.1
    HOMEPAGE_URL "https://github.com/BLumia/pineapple-assoc-manager"
)

include(FetchContent)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 "6.8" REQUIRED COMPONENTS Widgets LinguistTools)

set(EXE_NAME passoc)

qt_add_executable(${EXE_NAME}
    app/main.cpp
    app/mainwindow.cpp
    app/mainwindow.h
    app/mainwindow.ui
    app/association_manager.cpp
    app/association_manager.h
)

qt_add_resources(${EXE_NAME} resources.qrc
    PREFIX "/"
    BASE "app/"
    FILES "app/default-assoc.pacfg"
)

target_link_libraries(${EXE_NAME} PRIVATE Qt6::Widgets)

set (INSTALL_TARGETS_DEFAULT_ARGS
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Devel
)

install (
    TARGETS ${EXE_NAME}
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
