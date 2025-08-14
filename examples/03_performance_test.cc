/**
 * @file 03_performance_test.cc
 * @brief BuriedPoint 性能测试示例
 * 
 * 测试 BuriedPoint 在高并发场景下的性能表现
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <iomanip>
#include "../include/buried.h"

// 性能统计结构
struct PerformanceStats {
    std::atomic<int> total_events{0};
    std::atomic<int> successful_reports{0};
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
};

// 高频上报线程函数
void high_frequency_reporter(Buried* buried, int thread_id, int event_count, PerformanceStats* stats) {
    for (int i = 0; i < event_count; ++i) {
        // 构造事件数据
        std::string event_data = R"({"thread_id":)" + std::to_string(thread_id) + 
                               R"(,"event_index":)" + std::to_string(i) + 
                               R"(,"timestamp":)" + std::to_string(std::time(nullptr)) + "}";
        
        // 上报事件
        Buried_Report(buried, "performance_test", event_data.c_str(), 1);
        
        // 更新统计
        stats->total_events++;
        stats->successful_reports++;
    }
}

// 批量上报测试
void batch_reporter(Buried* buried, int batch_size, PerformanceStats* stats) {
    for (int i = 0; i < batch_size; ++i) {
        std::string event_data = R"({"batch_index":)" + std::to_string(i) + 
                               R"(,"data_size":1024,"timestamp":)" + 
                               std::to_string(std::time(nullptr)) + "}";
        
        Buried_Report(buried, "batch_test", event_data.c_str(), 1);
        stats->total_events++;
        stats->successful_reports++;
        
        // 无延迟，测试最大吞吐量
    }
}

void print_performance_results(const PerformanceStats& stats) {
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        stats.end_time - stats.start_time);
    
    double events_per_second = (double)stats.total_events.load() / (duration.count() / 1000.0);
    double success_rate = (double)stats.successful_reports.load() / stats.total_events.load() * 100.0;
    
    std::cout << "\n=== 性能测试结果 ===" << std::endl;
    std::cout << "总事件数: " << stats.total_events.load() << std::endl;
    std::cout << "成功上报: " << stats.successful_reports.load() << std::endl;
    std::cout << "测试耗时: " << duration.count() << " ms" << std::endl;
    std::cout << "成功率: " << std::fixed << std::setprecision(2) << success_rate << "%" << std::endl;
    std::cout << "吞吐量: " << std::fixed << std::setprecision(2) << events_per_second << " events/sec" << std::endl;
}

int main() {
    std::cout << "=== BuriedPoint 性能测试 ===" << std::endl;
    
    // 1. 创建埋点实例
    std::cout << "1. 创建埋点实例..." << std::endl;
    Buried* buried = Buried_Create("./data/performance_test");
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
    config.topic = "performance_analytics";
    config.user_id = "perf_test";
    config.app_version = "1.0.0";
    config.app_name = "PerformanceTestApp";
    config.custom_data = R"({"test_type":"performance","mode":"high_concurrency"})";
    
    Buried_Start(buried, &config);
    std::cout << "   ✅ 服务启动成功" << std::endl;

    // 3. 高并发测试
    std::cout << "\n3. 开始高并发测试..." << std::endl;
    PerformanceStats stats1;
    const int concurrent_threads = 10;
    const int events_per_thread = 1000;
    
    std::vector<std::thread> threads;
    stats1.start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < concurrent_threads; ++i) {
        threads.emplace_back(high_frequency_reporter, buried, i, events_per_thread, &stats1);
    }
    
    std::cout << "   🚀 已启动 " << concurrent_threads << " 个线程，每个线程上报 " 
              << events_per_thread << " 个事件" << std::endl;
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    stats1.end_time = std::chrono::high_resolution_clock::now();
    print_performance_results(stats1);

    // 4. 批量上报测试
    std::cout << "\n4. 开始批量上报测试..." << std::endl;
    PerformanceStats stats2;
    const int batch_size = 5000;
    
    stats2.start_time = std::chrono::high_resolution_clock::now();
    batch_reporter(buried, batch_size, &stats2);
    stats2.end_time = std::chrono::high_resolution_clock::now();
    
    print_performance_results(stats2);

    // 5. 等待数据处理完成
    std::cout << "\n5. 等待数据处理完成..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "   ✅ 数据处理完成" << std::endl;

    // 6. 清理资源
    std::cout << "6. 清理资源..." << std::endl;
    Buried_Destroy(buried);
    std::cout << "   ✅ 资源清理完成" << std::endl;
    
    std::cout << "\n=== 性能测试完成 ===" << std::endl;
    return 0;
}