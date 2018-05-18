cmake_minimum_required(VERSION 3.5)

project(qdbusmonitor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include(FeatureSummary)

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Core
    Quick
    QuickControls2
    DBus
)

find_package(LibDBus REQUIRED)

add_executable(${PROJECT_NAME}
    "main.cpp"
    "dbusmonitorthread.cpp"
    "dbusmessagesmodel.cpp"
    "dbusmessageobject.cpp"
    "qml.qrc"
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::DBus
    LibDBus::LibDBus
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
