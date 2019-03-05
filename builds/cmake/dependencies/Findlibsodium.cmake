if (NOT MSVC)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBSODIUM "libsodium")
    if (NOT PC_LIBSODIUM_FOUND)
        pkg_check_modules(PC_LIBSODIUM "sodium")
    endif (NOT PC_LIBSODIUM_FOUND)
    if (PC_LIBSODIUM_FOUND)
        # some libraries install the headers is a subdirectory of the include dir
        # returned by pkg-config, so use a wildcard match to improve chances of finding
        # headers and SOs.
        set(PC_LIBSODIUM_INCLUDE_HINTS ${PC_LIBSODIUM_INCLUDE_DIRS} ${PC_LIBSODIUM_INCLUDE_DIRS}/*)
        set(PC_LIBSODIUM_LIBRARY_HINTS ${PC_LIBSODIUM_LIBRARY_DIRS} ${PC_LIBSODIUM_LIBRARY_DIRS}/*)
    endif(PC_LIBSODIUM_FOUND)
endif (NOT MSVC)

find_path (
    LIBSODIUM_INCLUDE_DIRS
    NAMES sodium.h
    HINTS ${PC_LIBSODIUM_INCLUDE_HINTS}
)

find_library (
    LIBSODIUM_LIBRARIES
    NAMES libsodium
    HINTS ${PC_LIBSODIUM_LIBRARY_HINTS}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    LIBSODIUM
    REQUIRED_VARS LIBSODIUM_LIBRARIES LIBSODIUM_INCLUDE_DIRS
)
mark_as_advanced(
    LIBSODIUM_FOUND
    LIBSODIUM_LIBRARIES LIBSODIUM_INCLUDE_DIRS
)
