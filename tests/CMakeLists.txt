cmake_minimum_required(VERSION 3.20)
project(BuriedTest)

# 平台特定定义
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

include_directories(. .. ../src/third_party)

# 原有测试文件
set(LEGACY_TEST_SRC
    test_crypt.cc
    test_common_service.cc
    test_http.cc
    test_executor.cc
    test_db.cc
    test.cc)

# 新的完善测试文件
set(COMPREHENSIVE_TEST_SRC
    test_comprehensive.cc)

set(THREAD_SAFETY_TEST_SRC
    test_thread_safety.cc)

# 创建测试可执行文件
add_executable(buried_test ${LEGACY_TEST_SRC})
add_executable(buried_comprehensive_test ${COMPREHENSIVE_TEST_SRC})
add_executable(buried_thread_safety_test ${THREAD_SAFETY_TEST_SRC})

# 链接库
target_link_libraries(buried_test Buried_static gtest)
target_link_libraries(buried_comprehensive_test Buried_static gtest)
target_link_libraries(buried_thread_safety_test Buried_static gtest)

# 设置 C++ 标准
set_target_properties(
    buried_test buried_comprehensive_test buried_thread_safety_test
    PROPERTIES CXX_STANDARD 20
)

# 添加测试到 CTest
enable_testing()
add_test(NAME LegacyTests COMMAND buried_test)
add_test(NAME ComprehensiveTests COMMAND buried_comprehensive_test)
add_test(NAME ThreadSafetyTests COMMAND buried_thread_safety_test)