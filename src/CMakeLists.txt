include_guard(GLOBAL)

include_directories(. third_party third_party/spdlog/include third_party/boost third_party/mbedtls/include)

# 平台特定的预处理器定义
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_definitions(-DBOOST_JSON_NO_LIB)
add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/mbedtls/library)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/buried_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/buried_config.h)

set(DB_SRCS third_party/sqlite/sqlite3.c database/database.cc)

set(BURIED_SRCS
    ${DB_SRCS}
    crypt/crypt.cc
    report/buried_report.cc
    report/http_report.cc
    common/common_service.cc
    context/context.cc
    buried.cc
    buried_core.cc
)

set(MODULE_NAME "Buried")

message(STATUS "libs ${LIBS}")

if(BUILD_BURIED_STATIC_LIBS)
    add_library(${MODULE_NAME}_static STATIC ${BURIED_SRCS})
    target_link_libraries(${MODULE_NAME}_static PRIVATE ${LIBS} spdlog mbedtls_static)
    
    # 添加C++17/20标准支持和filesystem库
    target_compile_features(${MODULE_NAME}_static PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
        target_link_libraries(${MODULE_NAME}_static PRIVATE stdc++fs)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
        target_link_libraries(${MODULE_NAME}_static PRIVATE c++fs)
    endif()
endif()

if(BUILD_BURIED_SHARED_LIBS)
    add_library(${MODULE_NAME}_shared SHARED ${BURIED_SRCS})
    target_link_libraries(${MODULE_NAME}_shared PRIVATE ${LIBS} spdlog mbedtls_static)
    
    # 定义导出宏
    target_compile_definitions(${MODULE_NAME}_shared PRIVATE BURIED_EXPORTS)
    
    # 添加C++17/20标准支持和filesystem库
    target_compile_features(${MODULE_NAME}_shared PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
        target_link_libraries(${MODULE_NAME}_shared PRIVATE stdc++fs)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
        target_link_libraries(${MODULE_NAME}_shared PRIVATE c++fs)
    endif()
endif()