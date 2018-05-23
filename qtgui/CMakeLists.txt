cmake_minimum_required(VERSION 3.5)

project(qdbusmonitor-qt5 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    DBus
)

add_executable(${PROJECT_NAME}
    "main.cpp"
    "monitorapp.cpp"
    "dbusmessagesmodel.cpp"
    "qml.qrc"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "../libqdbusmonitor"
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::DBus
    qdbusmonitor
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    QT_STRICT_ITERATORS
    QT_NO_SIGNALS_SLOTS_KEYWORDS
    QT_USE_FAST_OPERATOR_PLUS
    QT_USE_QSTRINGBUILDER
)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
