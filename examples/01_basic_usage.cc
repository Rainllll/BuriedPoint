/**
 * @file 01_basic_usage.cc
 * @brief BuriedPoint 基础使用示例
 * 
 * 演示如何创建实例、配置服务、上报数据和清理资源
 */

#include <iostream>
#include <chrono>
#include <thread>
#include "../include/buried.h"

int main() {
    std::cout << "=== BuriedPoint 基础使用示例 ===" << std::endl;
    
    // 1. 创建埋点实例
    std::cout << "1. 创建埋点实例..." << std::endl;
    Buried* buried = Buried_Create("./data/basic_example");
    if (!buried) {
        std::cerr << "错误: 无法创建埋点实例" << std::endl;
        return -1;
    }
    std::cout << "   ✅ 实例创建成功" << std::endl;

    // 2. 配置埋点服务
    std::cout << "2. 配置埋点服务..." << std::endl;
    BuriedConfig config;
    config.host = "localhost";
    config.port = "8080";
    config.topic = "user_analytics";
    config.user_id = "user_12345";
    config.app_version = "1.0.0";
    config.app_name = "ExampleApp";
    config.custom_data = R"({"platform":"desktop","env":"development"})";
    
    Buried_Start(buried, &config);
    std::cout << "   ✅ 服务启动成功" << std::endl;

    // 3. 上报基础事件
    std::cout << "3. 上报基础事件..." << std::endl;
    
    // 用户登录事件
    Buried_Report(buried, "user_login", R"({"method":"email","success":true})", 1);
    std::cout << "   📊 用户登录事件已上报" << std::endl;
    
    // 页面访问事件
    Buried_Report(buried, "page_view", R"({"page":"/dashboard","duration":1500})", 1);
    std::cout << "   📊 页面访问事件已上报" << std::endl;
    
    // 功能使用事件
    Buried_Report(buried, "feature_usage", R"({"feature":"export","format":"pdf"})", 1);
    std::cout << "   📊 功能使用事件已上报" << std::endl;

    // 4. 等待数据处理
    std::cout << "4. 等待数据处理..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "   ✅ 数据处理完成" << std::endl;

    // 5. 清理资源
    std::cout << "5. 清理资源..." << std::endl;
    Buried_Destroy(buried);
    std::cout << "   ✅ 资源清理完成" << std::endl;
    
    std::cout << "=== 示例运行完成 ===" << std::endl;
    return 0;
}