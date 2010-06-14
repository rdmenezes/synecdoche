function(synec_add_test name)
    include_directories(${UNITTESTPP_INCLUDE_DIR})
    
    set(srcs ${ARGN})
    set(testlib synectest) #default testlib
    if(${ARGV1} STREQUAL "NO_TESTLIB")
        #use unittest++ only
        set(testlib ${UNITTESTPP_LIBRARY})
        list(REMOVE_AT srcs 0)
    elseif(${ARGV1} STREQUAL "TESTLIB")
        set(testlib ${ARGV2})
        list(REMOVE_AT srcs 0 1)
    endif(${ARGV1} STREQUAL "NO_TESTLIB")

    add_executable(${name} ${srcs})
    target_link_libraries(${name} ${testlib})
    add_test(NAME ${name} COMMAND ${name})
endfunction(synec_add_test)

include_directories(${UNITTESTPP_INCLUDE_DIR})
add_library(synectest STATIC EXCLUDE_FROM_ALL TestRunner.cpp Util.cpp)
target_link_libraries(synectest ${UNITTESTPP_LIBRARY})
