if(ENABLE_TESTS)
    macro(add_my_test test)
        add_custom_target(runtest-${test} ${test} DEPENDS ${test})
        add_dependencies(test runtest-${test})
    endmacro(add_my_test)
    add_custom_target(test)

    find_package(UnitTestPP)
    include_directories(${UNITTESTPP_INCLUDE_DIR})

    add_executable(TestLib EXCLUDE_FROM_ALL
        TestRunner.cpp
        TestXmlParser.cpp
        TestStrUtil.cpp
        TestMioFile.cpp
        TestXmlWrite.cpp
        TestUtil.cpp
        Util.cpp
    )
    target_link_libraries(TestLib ${UNITTESTPP_LIBRARY} boinc)

    add_my_test(TestLib)
endif(ENABLE_TESTS)
