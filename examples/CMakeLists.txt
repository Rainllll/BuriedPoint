cmake_minimum_required(VERSION 3.20)
project(BuriedExample)

# 平台特定定义
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

include_directories(. .. ../src ../src/third_party ../src/third_party/spdlog/include ../src/third_party/boost)

# 新的完善示例
add_executable(01_basic_usage 01_basic_usage.cc)
add_executable(02_multithreaded_usage 02_multithreaded_usage.cc)
add_executable(03_performance_test 03_performance_test.cc)
add_executable(04_error_handling 04_error_handling.cc)
add_executable(05_priority_demo 05_priority_demo.cc)
add_executable(06_dynamic_config_demo 06_dynamic_config_demo.cc)

# 原有示例（向后兼容）
add_executable(buried_example buried_example.cc)
add_executable(buried_example_shared buried_example.cc)
add_executable(context_example context_example.cc)
add_executable(reporter_example reporter_example.cc)

# 链接库 - 新示例
target_link_libraries(01_basic_usage Buried_static)
target_link_libraries(02_multithreaded_usage Buried_static)
target_link_libraries(03_performance_test Buried_static)
target_link_libraries(04_error_handling Buried_static)
target_link_libraries(05_priority_demo Buried_static)
target_link_libraries(06_dynamic_config_demo Buried_static)

# 链接库 - 原有示例
target_link_libraries(buried_example Buried_static)
target_link_libraries(buried_example_shared Buried_shared)
target_link_libraries(context_example Buried_static)
target_link_libraries(reporter_example Buried_static)

# 设置 C++ 标准
set_target_properties(
    01_basic_usage 02_multithreaded_usage 03_performance_test 04_error_handling 05_priority_demo 06_dynamic_config_demo
    buried_example buried_example_shared context_example reporter_example
    PROPERTIES CXX_STANDARD 20
)