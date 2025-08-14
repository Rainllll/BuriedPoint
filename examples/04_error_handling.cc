/**
 * @file 04_error_handling.cc
 * @brief BuriedPoint 错误处理示例
 * 
 * 演示如何处理各种错误情况和异常场景
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include "../include/buried.h"

void test_invalid_path() {
    std::cout << "\n--- 测试无效路径 ---" << std::endl;
    
    try {
        // 尝试使用无效路径创建实例
        Buried* buried = Buried_Create("/invalid/path/that/does/not/exist");
        if (!buried) {
            std::cout << "   ❌ 预期错误: 无效路径创建失败" << std::endl;
        } else {
            std::cout << "   ⚠️  意外: 无效路径创建成功" << std::endl;
            Buried_Destroy(buried);
        }
    } catch (const std::exception& e) {
        std::cout << "   ❌ 预期错误: 捕获异常 - " << e.what() << std::endl;
    } catch (...) {
        std::cout << "   ❌ 预期错误: 捕获未知异常" << std::endl;
    }
}

void test_null_parameters() {
    std::cout << "\n--- 测试空参数 ---" << std::endl;
    
    try {
        // 测试空路径
        Buried* buried1 = Buried_Create(nullptr);
        if (!buried1) {
            std::cout << "   ❌ 预期错误: 空路径创建失败" << std::endl;
        } else {
            Buried_Destroy(buried1);
        }
    } catch (const std::exception& e) {
        std::cout << "   ❌ 预期错误: 空路径异常 - " << e.what() << std::endl;
    }
    
    try {
        // 测试空字符串路径
        Buried* buried2 = Buried_Create("");
        if (!buried2) {
            std::cout << "   ❌ 预期错误: 空字符串路径创建失败" << std::endl;
        } else {
            Buried_Destroy(buried2);
        }
    } catch (const std::exception& e) {
        std::cout << "   ❌ 预期错误: 空字符串路径异常 - " << e.what() << std::endl;
    }
    
    try {
        // 创建有效实例用于后续测试
        Buried* buried = Buried_Create("./data/error_test");
        if (buried) {
            // 测试空配置
            Buried_Start(buried, nullptr);
            std::cout << "   ⚠️  空配置启动测试完成" << std::endl;
            
            // 测试空参数上报
            Buried_Report(nullptr, "test", "data", 1);
            Buried_Report(buried, nullptr, "data", 1);
            Buried_Report(buried, "test", nullptr, 1);
            std::cout << "   ⚠️  空参数上报测试完成" << std::endl;
            
            Buried_Destroy(buried);
        }
    } catch (const std::exception& e) {
        std::cout << "   ❌ 有效路径测试异常 - " << e.what() << std::endl;
    }
}

void test_invalid_config() {
    std::cout << "\n--- 测试无效配置 ---" << std::endl;
    
    Buried* buried = Buried_Create("./data/config_test");
    if (!buried) {
        std::cout << "   ❌ 实例创建失败" << std::endl;
        return;
    }
    
    // 测试无效主机配置
    BuriedConfig config1;
    config1.host = "";  // 空主机
    config1.port = "8080";
    config1.topic = "test";
    config1.user_id = "user";
    config1.app_version = "1.0.0";
    config1.app_name = "TestApp";
    config1.custom_data = "{}";
    
    Buried_Start(buried, &config1);
    std::cout << "   ⚠️  空主机配置测试完成" << std::endl;
    
    // 测试无效端口配置
    BuriedConfig config2;
    config2.host = "localhost";
    config2.port = "invalid_port";  // 无效端口
    config2.topic = "test";
    config2.user_id = "user";
    config2.app_version = "1.0.0";
    config2.app_name = "TestApp";
    config2.custom_data = "{}";
    
    Buried_Start(buried, &config2);
    std::cout << "   ⚠️  无效端口配置测试完成" << std::endl;
    
    Buried_Destroy(buried);
}

void test_resource_limits() {
    std::cout << "\n--- 测试资源限制 ---" << std::endl;
    
    Buried* buried = Buried_Create("./data/resource_test");
    if (!buried) {
        std::cout << "   ❌ 实例创建失败" << std::endl;
        return;
    }
    
    BuriedConfig config;
    config.host = "localhost";
    config.port = "8080";
    config.topic = "resource_test";
    config.user_id = "user";
    config.app_version = "1.0.0";
    config.app_name = "ResourceTestApp";
    config.custom_data = "{}";
    
    Buried_Start(buried, &config);
    
    // 测试大量数据上报
    std::cout << "   📊 测试大量数据上报..." << std::endl;
    std::string large_data(10000, 'A');  // 10KB 数据
    
    for (int i = 0; i < 100; ++i) {
        Buried_Report(buried, "large_data_test", large_data.c_str(), 1);
        if (i % 20 == 0) {
            std::cout << "   📈 已上报 " << i + 1 << " 个大数据事件" << std::endl;
        }
    }
    
    std::cout << "   ✅ 大量数据上报测试完成" << std::endl;
    
    Buried_Destroy(buried);
}

void test_concurrent_access() {
    std::cout << "\n--- 测试并发访问异常 ---" << std::endl;
    
    Buried* buried = Buried_Create("./data/concurrent_test");
    if (!buried) {
        std::cout << "   ❌ 实例创建失败" << std::endl;
        return;
    }
    
    BuriedConfig config;
    config.host = "localhost";
    config.port = "8080";
    config.topic = "concurrent_test";
    config.user_id = "user";
    config.app_version = "1.0.0";
    config.app_name = "ConcurrentTestApp";
    config.custom_data = "{}";
    
    Buried_Start(buried, &config);
    
    // 创建多个线程同时访问
    std::vector<std::thread> threads;
    std::atomic<int> error_count{0};
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([buried, i, &error_count]() {
            try {
                for (int j = 0; j < 50; ++j) {
                    std::string data = R"({"thread":)" + std::to_string(i) + 
                                     R"(,"index":)" + std::to_string(j) + "}";
                    Buried_Report(buried, "concurrent_test", data.c_str(), 1);
                }
            } catch (...) {
                error_count++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "   📊 并发访问错误数: " << error_count.load() << std::endl;
    std::cout << "   ✅ 并发访问测试完成" << std::endl;
    
    Buried_Destroy(buried);
}

int main() {
    std::cout << "=== BuriedPoint 错误处理测试 ===" << std::endl;
    
    // 创建测试数据目录
    std::filesystem::create_directories("./data");
    
    // 运行各种错误测试
    test_invalid_path();
    test_null_parameters();
    test_invalid_config();
    test_resource_limits();
    test_concurrent_access();
    
    std::cout << "\n=== 错误处理测试完成 ===" << std::endl;
    std::cout << "注意: 上述测试中的错误和警告是预期的，用于验证错误处理机制" << std::endl;
    
    return 0;
}