cmake_minimum_required(VERSION 3.5)
project(qdbusmonitor LANGUAGES CXX)

option(BUILD_CLI "Build command-line executable" ON)
option(BUILD_GUI "Build Qt5 frontend executable" ON)

add_subdirectory("libqdbusmonitor")

if (BUILD_CLI)
    add_subdirectory("cli")
endif()

if (BUILD_GUI)
    add_subdirectory("qtgui")
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
