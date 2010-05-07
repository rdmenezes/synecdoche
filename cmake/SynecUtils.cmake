macro(add_svnversion_target)
    IF(WIN32)
        STRING(REPLACE "/" "\\" VERSION_CPP_PATH ${CMAKE_BINARY_DIR}/svnversion.cpp)
        ADD_CUSTOM_TARGET(update_svnversion ALL
            ${CMAKE_SOURCE_DIR}/version.bat ${VERSION_CPP_PATH}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            VERBATIM
        )
    ELSE(WIN32)
        ADD_CUSTOM_TARGET(update_svnversion ALL
            ${CMAKE_SOURCE_DIR}/version.sh ${CMAKE_BINARY_DIR}/svnversion.cpp
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            VERBATIM
        )
    ENDIF(WIN32)

    FILE(WRITE ${CMAKE_BINARY_DIR}/svnversion.cpp "//dummy")
endmacro(add_svnversion_target)

macro(get_boinc_platform var)
    if(NOT DEFINED ${var} OR ${var} STREQUAL "unknown-unknown")

    # If the variable is stored in the cache as unknown-unknown,
    # we need to unset it; otherwise we can't set it again,
    # and even with a successful platform detection,
    # the cache will keep saying unknown-unknown
    unset(${var} CACHE)

    set(_boinc_platform "unknown-unknown")
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(_boinc_platform "windows_intelx86")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_boinc_platform "windows_x86_64")
        endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    elseif(APPLE)
        message(WARNING "Building on Mac is not supported")
        #message(FATAL_ERROR "Detecting Mac CPU type is not supported yet.\nPlease set the platform name manually with -DBOINC_PLATFORM=")
    elseif(UNIX)
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
                set(_boinc_platform "i686-pc-linux-gnu")
            elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
                set(_boinc_platform "x86_64-pc-linux-gnu")
            else()
                set(_boinc_platform_err "Unknown CPU type")
            endif()
        else()
            set(_boinc_platform_err "Unknown Unix variant: ${CMAKE_SYSTEM_NAME}")
        endif()
    endif()
    set(${var} "${_boinc_platform}" CACHE STRING "BOINC platform string")
    if(DEFINED _boinc_platform_err)
        message(FATAL_ERROR "${_boinc_platform_err}\nYou can set the platform name manually with -D${var}=platform")
    endif()

    endif(NOT DEFINED ${var} OR ${var} STREQUAL "unknown-unknown")
endmacro(get_boinc_platform)
