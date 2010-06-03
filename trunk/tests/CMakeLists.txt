find_package(UnitTestPP)
include_directories(${UNITTESTPP_INCLUDE_DIR})

macro(synec_add_test name)
    add_executable(${name} TestRunner.cpp ${ARGN})
    target_link_libraries(${name} ${UNITTESTPP_LIBRARY})
    add_test(NAME ${name} COMMAND ${name})
endmacro(synec_add_test)

synec_add_test(TestLib
    TestXmlParser.cpp
    TestStrUtil.cpp
    TestMioFile.cpp
    TestXmlWrite.cpp
    TestUtil.cpp
    Util.cpp
)
target_link_libraries(TestLib boinc)
