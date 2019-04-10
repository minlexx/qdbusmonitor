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
)

find_package(LibDBus REQUIRED)

add_library(${PROJECT_NAME} SHARED
    "dbusmessageobject.cpp"
    "dbusmonitorthread.cpp"
    "dbusmonitorthread_p.cpp"
    "messagecontentsparser.cpp"
    "utils.cpp"
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    LibDBus::LibDBus
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    BUILD_LIBQDBUSMONITOR
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
    LIBRARY DESTINATION lib
)

