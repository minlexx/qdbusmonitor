if(NOT WIN32)
    # Use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PKG_LibDBus QUIET dbus-1)

    set(LibDBus_DEFINITIONS ${PKG_LibDBus_CFLAGS_OTHER})
    set(LibDBus_VERSION ${PKG_LibDBus_VERSION})

    #message(STATUS "PKG_LibDBus_INCLUDE_DIRS" ${PKG_LibDBus_INCLUDE_DIRS})
    #message(STATUS "PKG_LibDBus_LIBRARY_DIRS" ${PKG_LibDBus_LIBRARY_DIRS})

    find_path(LibDBus_INCLUDE_DIR
        NAMES
            dbus/dbus.h
            dbus/dbus-arch-deps.h
        HINTS
            ${PKG_LibDBus_INCLUDE_DIRS}
    )

    find_path(LibDBus_INCLUDE_DIR2
        NAMES
            dbus/dbus-arch-deps.h
        HINTS
            ${PKG_LibDBus_INCLUDE_DIRS}
    )

    set(LibDBus_INCLUDE_DIR ${LibDBus_INCLUDE_DIR} ${LibDBus_INCLUDE_DIR2})

    find_library(LibDBus_LIBRARY
        NAMES
            dbus-1
        HINTS
            ${PKG_LibDBus_LIBRARY_DIRS}
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(LibDBus
        FOUND_VAR
            LibDBus_FOUND
        REQUIRED_VARS
            LibDBus_LIBRARY
            LibDBus_INCLUDE_DIR
        VERSION_VAR
            LibDBus_VERSION
    )

    if(LibDBus_FOUND AND NOT TARGET LibDBus::LibDBus)
        add_library(LibDBus::LibDBus UNKNOWN IMPORTED)
        set_target_properties(LibDBus::LibDBus PROPERTIES
            IMPORTED_LOCATION "${LibDBus_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${LibDBus_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${LibDBus_INCLUDE_DIR}"
        )
    endif()

    mark_as_advanced(LibDBus_LIBRARY LibDBus_INCLUDE_DIR)
    mark_as_advanced(LibDBus_LIBRARY LibDBus_INCLUDE_DIR2)

    # compatibility variables
    set(LibDBus_LIBRARIES ${LibDBus_LIBRARY})
    set(LibDBus_INCLUDE_DIRS ${LibDBus_INCLUDE_DIR})
    set(LibDBus_VERSION_STRING ${LibDBus_VERSION})

else()
    message(STATUS "FindLibDBus.cmake cannot find libdbus-1 on Windows systems.")
    set(LibDBus_FOUND FALSE)
endif()

include(FeatureSummary)
set_package_properties(LibDBus PROPERTIES
    URL "https://dbus.freedesktop.org/"
    DESCRIPTION "Message bus system library."
)

