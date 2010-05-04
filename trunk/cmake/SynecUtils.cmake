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
