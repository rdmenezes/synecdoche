find_package(UnitTestPP)
include_directories(${UNITTESTPP_INCLUDE_DIR})

add_executable(TestLib
    TestRunner.cpp
    TestXmlParser.cpp
    TestStrUtil.cpp
    TestMioFile.cpp
    TestXmlWrite.cpp
    TestUtil.cpp
    Util.cpp
)
target_link_libraries(TestLib ${UNITTESTPP_LIBRARY} boinc)

add_test(NAME TestLib COMMAND TestLib)
