synec_add_test(TestLib
    TestXmlParser.cpp
    TestStrUtil.cpp
    TestMioFile.cpp
    TestXmlWrite.cpp
    TestUtil.cpp
)
target_link_libraries(TestLib boinc)
