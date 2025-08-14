/**
 * @file 02_multithreaded_usage.cc
 * @brief BuriedPoint 多线程使用示例
 * 
 * 演示在多线程环境中安全使用 BuriedPoint 进行数据上报
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <sstream>
#include "../include/buried.h"

// 模拟用户行为的线程函数
void simulate_user_behavior(Buried* buried, int user_id, int event_count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delay_dist(100, 1000);  // 100-1000ms 随机延迟
    std::uniform_int_distribution<> action_dist(1, 4);      // 4种不同的行为
    
    for (int i = 0; i < event_count; ++i) {
        std::stringstream event_data;
        std::string event_type;
        
        // 随机选择事件类型
        switch (action_dist(gen)) {
            case 1:
                event_type = "page_view";
                event_data << R"({"user_id":)" << user_id 
                          << R"(,"page":"/page_)" << (i % 5) + 1 
                          << R"(","timestamp":)" << std::time(nullptr) << "}";
                break;
            case 2:
                event_type = "button_click";
                event_data << R"({"user_id":)" << user_id 
                          << R"(,"button":"btn_)" << (i % 3) + 1 
                          << R"(","timestamp":)" << std::time(nullptr) << "}";
                break;
            case 3:
                event_type = "api_call";
                event_data << R"({"user_id":)" << user_id 
                          << R"(,"endpoint":"/api/data_)" << (i % 4) + 1 
                          << R"(","timestamp":)" << std::time(nullptr) << "}";
                break;
            case 4:
                event_type = "error_occurred";
                event_data << R"({"user_id":)" << user_id 
                          << R"(,"error_code":)" << (i % 10) + 400 
                          << R"(","timestamp":)" << std::time(nullptr) << "}";
                break;
        }
        
        // 上报事件（线程安全）
        Buried_Report(buried, event_type.c_str(), event_data.str().c_str(), 1);
        
        // 随机延迟模拟真实用户行为
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(gen)));
    }
    
    std::cout << "用户 " << user_id << " 完成了 " << event_count << " 个事件上报" << std::endl;
}

int main() {
    std::cout << "=== BuriedPoint 多线程使用示例 ===" << std::endl;
    
    // 1. 创建埋点实例
    std::cout << "1. 创建埋点实例..." << std::endl;
    Buried* buried = Buried_Create("./data/multithreaded_example");
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
    config.topic = "multithreaded_analytics";
    config.user_id = "system";
    config.app_version = "1.0.0";
    config.app_name = "MultithreadedApp";
    config.custom_data = R"({"test_type":"multithreaded","thread_count":5})";
    
    Buried_Start(buried, &config);
    std::cout << "   ✅ 服务启动成功" << std::endl;

    // 3. 创建多个线程模拟并发用户
    std::cout << "3. 启动多线程模拟..." << std::endl;
    const int thread_count = 5;
    const int events_per_thread = 20;
    std::vector<std::thread> threads;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 启动多个线程
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(simulate_user_behavior, buried, i + 1, events_per_thread);
    }
    
    std::cout << "   🚀 已启动 " << thread_count << " 个线程，每个线程上报 " 
              << events_per_thread << " 个事件" << std::endl;

    // 4. 等待所有线程完成
    std::cout << "4. 等待所有线程完成..." << std::endl;
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "   ✅ 所有线程完成，总耗时: " << duration.count() << " ms" << std::endl;
    std::cout << "   📊 总共上报事件数: " << thread_count * events_per_thread << std::endl;

    // 5. 等待数据处理完成
    std::cout << "5. 等待数据处理完成..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "   ✅ 数据处理完成" << std::endl;

    // 6. 清理资源
    std::cout << "6. 清理资源..." << std::endl;
    Buried_Destroy(buried);
    std::cout << "   ✅ 资源清理完成" << std::endl;
    
    std::cout << "=== 多线程示例运行完成 ===" << std::endl;
    return 0;
}