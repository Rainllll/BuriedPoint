# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.20)

# 定义项目名称和版本号
set(PROJECT_NAME "BuriedPoint")
project(${PROJECT_NAME} VERSION 1.1.1.1)

# 构建选项配置
option(BUILD_BURIED_SHARED_LIBS "构建动态链接库" ON)  # 默认构建动态库
option(BUILD_BURIED_STATIC_LIBS "构建静态链接库" ON)  # 默认构建静态库
option(BUILD_BURIED_EXAMPLES "构建示例程序" OFF)      # 默认不构建示例
option(BUILD_BURIED_TEST "构建单元测试" OFF)          # 默认不构建测试
option(BUILD_BURIED_FOR_MT "使用/MT运行时库（静态链接运行时库）" OFF)

# 多进程编译选项（加快编译速度）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

# MT运行时库配置
if(BUILD_BURIED_FOR_MT)
    # 调试版本使用MTd
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    # 发布版本使用MT
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

# 系统库依赖
set(LIBS ${LIBS} 
    winmm      # Windows多媒体库
    iphlpapi   # IP帮助API
    ws2_32     # Winsock2
    dbghelp    # 调试帮助库
    Kernel32   # Windows内核API
)

# 生成PDB调试符号文件配置
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")  # 生成调试信息
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
    "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")  # 链接器优化选项

# 包含目录设置
set(INCDIRS . lib ${CMAKE_BINARY_DIR})
include_directories(${INCDIRS})

# 语言标准设置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")  # C++20标准
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c11")        # C11标准

# 构建类型信息输出
message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")

# 根据构建类型定义预处理宏
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DDEBUG)    # 调试模式定义DEBUG宏
else()
    add_definitions(-DNDEBUG)   # 发布模式定义NDEBUG宏
endif()

# 添加子目录（核心代码）
add_subdirectory(src)

# 条件包含示例代码目录
if(BUILD_BURIED_EXAMPLES)
    add_subdirectory(examples)
endif()

# 条件包含测试框架
if(BUILD_BURIED_TEST)
    # Google Test包含目录
    include_directories(
        googletest/googletest
        googletest/googletest/include)
    add_subdirectory(googletest)  # 添加gtest子项目
    add_subdirectory(tests)       # 添加测试代码目录
endif()
