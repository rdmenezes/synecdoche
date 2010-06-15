add_library(synecguitest STATIC EXCLUDE_FROM_ALL TestRunnerWx.cpp)
target_link_libraries(synecguitest ${UNITTESTPP_LIBRARIES} ${wxWidgets_LIBRARIES})

function(synec_add_gui_test name)
    synec_add_test(${name} TESTLIB synecguitest ${ARGN})
    target_link_libraries(${name} synecguitest ${wxWidgets_LIBRARIES})
endfunction(synec_add_gui_test)

synec_add_test(TestGui TestFormatString.cpp ../UiFormatString.cpp)
synec_add_gui_test(TestBuildLayout TestBuildLayout.cpp ../UiFormatString.cpp ../BuildLayout.cpp)

target_link_libraries(TestGui ${wxWidgets_LIBRARIES})
