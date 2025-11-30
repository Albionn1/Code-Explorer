cmake_minimum_required(VERSION 3.16)

project(FileExplorer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 with Widgets + Svg modules
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    iconprovider.cpp
    iconprovider.h
    resources.qrc
)

qt_add_executable(FileExplorer
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    resources.qrc
    folderdelegate.h folderdelegate.cpp
    iconfactory.h iconfactory.cpp
)

# Link against Qt6 Widgets and Svg
target_link_libraries(FileExplorer PRIVATE Qt6::Widgets Qt6::Svg)

qt_finalize_executable(FileExplorer)
