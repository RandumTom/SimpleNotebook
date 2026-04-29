cmake_minimum_required(VERSION 3.16)
project(SimpleNotebook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(QTermWidget6 REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.cpp
    mainwindow.h
    startpage.cpp
    startpage.h
    editorview.cpp
    editorview.h
    gitpanel.cpp
    gitpanel.h
    MathConverter.cpp
    MathConverter.h
)

target_include_directories(${PROJECT_NAME} PRIVATE /usr/include/qtermwidget6)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets qtermwidget6)
