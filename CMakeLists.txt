cmake_minimum_required(VERSION 3.16)

project(FileExplorer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 with Widgets + Svg modules
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

# Project sources
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    iconprovider.cpp
    iconprovider.h
    iconfactory.cpp
    iconfactory.h
    ribbongroup.cpp
    ribbongroup.h
    folderdialog.cpp
    folderdialog.h
    resources.qrc
)

# Define executable
qt_add_executable(FileExplorer
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    codehighlighter.h codehighlighter.cpp
    codeviewer.h codeviewer.cpp
    codeviewerwindow.h codeviewerwindow.cpp
    linenumberarea.h linenumberarea.cpp
    codeeditor.h codeeditor.cpp
)

# Link against Qt6 and QScintilla
target_link_libraries(FileExplorer PRIVATE
    Qt6::Widgets
    Qt6::Svg
)

qt_finalize_executable(FileExplorer)
