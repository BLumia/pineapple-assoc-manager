# SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.23)

project(pineapple-assoc-manager
    VERSION 1.0.0
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

# Enable Qt tools
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en zh_CN ko ru sl tr vi)

configure_file(assets/pineapple-assoc-manager.rc.in pineapple-assoc-manager.rc @ONLY)

qt_add_executable(${EXE_NAME} WIN32
    app/main.cpp
    app/mainwindow.cpp
    app/mainwindow.h
    app/mainwindow.ui
    app/association_manager.cpp
    app/association_manager.h
    ${CMAKE_CURRENT_BINARY_DIR}/pineapple-assoc-manager.rc
)

qt_add_resources(${EXE_NAME} resources.qrc
    PREFIX "/"
    BASE "app/"
    FILES "app/default-assoc.pacfg"
)

# Add translations
qt_add_translations(${EXE_NAME}
    TS_FILE_DIR translations # Before Qt 6.9, the argument TS_OUTPUT_DIRECTORY was called TS_FILE_DIR. That name is still an alias for TS_OUTPUT_DIRECTORY to keep older project files working.
    RESOURCE_PREFIX "/i18n"
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

install(
    FILES assets/icons/pacfg.ico
    DESTINATION "${CMAKE_INSTALL_BINDIR}/icons"
)
