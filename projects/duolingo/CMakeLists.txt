cmake_minimum_required(VERSION 3.16)

project(duolingo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets Sql Multimedia Core)
if(NOT Qt6_FOUND)
    message(STATUS "Qt6 not found, trying Qt5...")
    find_package(Qt5 COMPONENTS Widgets Sql Multimedia Core REQUIRED)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        authdialog.cpp
        datamanager.cpp
        userstatsdialog.cpp
        audioexercisewidget.cpp)

set(PROJECT_HEADERS
        mainwindow.h
        authdialog.h
        datamanager.h
        userstatsdialog.h
        audioexercisewidget.h
        user.h
        leveldata.h)

set(EXISTING_PROJECT_HEADERS "")
foreach(header_file ${PROJECT_HEADERS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${header_file}")
        list(APPEND EXISTING_PROJECT_HEADERS ${header_file})
    endif()
endforeach()
set(PROJECT_HEADERS ${EXISTING_PROJECT_HEADERS})

set(PROJECT_RESOURCES resources.qrc)

if(${QT_VERSION_MAJOR} EQUAL 6)
    qt_add_executable(
            duolingo
            ${PROJECT_SOURCES}
            ${PROJECT_HEADERS}
            ${PROJECT_RESOURCES})
else()
    add_executable(
            duolingo
            ${PROJECT_SOURCES}
            ${PROJECT_HEADERS}
            ${PROJECT_RESOURCES})
endif()

if(TARGET duolingo)
    target_link_libraries(
            duolingo PRIVATE Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Sql
            Qt${QT_VERSION_MAJOR}::Multimedia)
else()
    message(FATAL_ERROR "Target 'duolingo' was not created.")
endif()

if(TARGET duolingo AND (APPLE OR WIN32))
    if(APPLE)
        set_target_properties(
                duolingo PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
                MACOSX_BUNDLE_GUI_IDENTIFIER
                "com.yourcompany.duolingo")
    endif()
    set_target_properties(duolingo PROPERTIES WIN32_EXECUTABLE TRUE
            MACOSX_BUNDLE TRUE)
endif()

include(GNUInstallDirs)
if(TARGET duolingo)
    install(TARGETS duolingo BUNDLE DESTINATION . RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()