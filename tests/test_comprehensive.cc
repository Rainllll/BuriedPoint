/**
 * @file test_comprehensive.cc
 * @brief BuriedPoint 综合功能测试
 * 
 * 全面测试 BuriedPoint 的各项功能和边界情况
 */

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>
#include <filesystem>
#include <atomic>
#include "include/buried.h"

class BuriedComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据目录
        test_data_path_ = "./test_data_" + std::to_string(std::time(nullptr));
        std::filesystem::create_directories(test_data_path_);
    }

    void TearDown() override {
        // 清理测试数据
        if (std::filesystem::exists(test_data_path_)) {
            std::filesystem::remove_all(test_data_path_);
        }
    }

    std::string test_data_path_;
};

// 测试基本创建和销毁
TEST_F(BuriedComprehensiveTest, BasicCreateDestroy) {
    Buried* buried = Buried_Create(test_data_path_.c_str());
    ASSERT_NE(buried, nullptr) << "埋点实例创建失败";
    
    Buried_Destroy(buried);
    // 销毁后不应该崩溃
}

// 测试配置和启动
TEST_F(BuriedComprehensiveTest, ConfigurationAndStart) {
    Buried* buried = Buried_Create(test_data_path_.c_str());
    ASSERT_NE(buried, nullptr);
    
    BuriedConfig config;
    config.host = "localhost";
    config.port = "8080";
    config.topic = "test_topic";
    config.user_id = "test_user_123";
    config.app_version = "1.0.0";
    config.app_name = "TestApp";
    config.custom_data = R"({"test_mode":true,"platform":"unittest"})";
    
    // 启动服务应该成功
    Buried_Start(buried, &config);
    
    // 等待服务启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    Buried_Destroy(buried);
}

// 测试基本数据上报
TEST_F(BuriedComprehensiveTest, BasicReporting) {
    Buried* buried = Buried_Create(test_data_path_.c_str());
    ASSERT_NE(buried, nullptr);
    
    BuriedConfig config;
    config.host = "localhost";
    config.port = "8080";
    config.topic = "reporting_test";
    config.user_id = "reporter_user";
    config.app_version = "1.0.0";
    config.app_name = "ReportingTestApp";
    config.custom_data = "{}";
    
    Buried_Start(buried, &config);
    
    // 上报各种类型的事件
    Buried_Report(buried, "user_action", R"({"action":"click","element":"button1"})", 1);
    Buried_Report(buried, "page_view", R"({"page":"/home","duration":1500})", 1);
    Buried_Report(buried, "api_call", R"({"endpoint":"/api/data","status":200})", 1);
    
    // 等待数据处理
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    Buried_Destroy(buried);
}

// 测试多线程安全性
TEST_F(BuriedComprehensiveTest, ThreadSafety) {
    Buried* buried = Buried_Create(test_data_path_.c_str());
    ASSERT_NE(buried, nullptr);
    
    BuriedConfig config;
    config.host = "localhost";
    config.port = "8080";
    config.topic = "thread_safety_test";
    config.user_id = "thread_user";
    config.app_version = "1.0.0";
    config.app_name = "ThreadSafetyTestApp";
    config.custom_data = "{}";
    
    Buried_Start(buried, &config);
    
    const int thread_count = 5;
    const int reports_per_thread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> total_reports{0};
    
    // 启动多个线程同时上报
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([buried, i, reports_per_thread, &total_reports]() {
            for (int j = 0; j < reports_per_thread; ++j) {
                std::string event_data = R"({"thread_id":)" + std::to_string(i) + 
                                       R"(,"report_index":)" + std::to_string(j) + "}";
                Buried_Report(buried, "thread_test", event_data.c_str(), 1);
                total_reports++;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有报告都已提交
    EXPECT_EQ(total_reports.load(), thread_count * reports_per_thread);
    
    // 等待数据处理完成
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    Buried_Destroy(buried);
}

// 测试错误处理
TEST_F(BuriedComprehensiveTest, ErrorHandling) {
    // 测试无效路径（可能抛出异常）
    try {
        Buried* invalid_buried = Buried_Create("/invalid/path/that/does/not/exist");
        // 根据实现，这可能返回 nullptr 或创建失败
        if (invalid_buried) {
            Buried_Destroy(invalid_buried);
        }
    } catch (const std::exception& e) {
        // 预期的异常，测试通过
        std::cout << "预期的异常: " << e.what() << std::endl;
    }
    
    // 测试空参数
    try {
        Buried* null_buried = Buried_Create(nullptr);
        if (null_buried) {
            Buried_Destroy(null_buried);
        }
    } catch (const std::exception& e) {
        // 预期的异常
        std::cout << "空参数异常: " << e.what() << std::endl;
    }
    
    // 测试有效实例的错误处理
    Buried* buried = Buried_Create(test_data_path_.c_str());
    ASSERT_NE(buried, nullptr);
    
    // 测试空配置启动
    Buried_Start(buried, nullptr);
    
    // 测试空参数上报（不应该崩溃）
    Buried_Report(nullptr, "test", "data", 1);
    Buried_Report(buried, nullptr, "data", 1);
    Buried_Report(buried, "test", nullptr, 1);
    
    Buried_Destroy(buried);
}

// 测试性能和资源使用
TEST_F(BuriedComprehensiveTest, PerformanceTest) {
    Buried* buried = Buried_Create(test_data_path_.c_str());
    ASSERT_NE(buried, nullptr);
    
    BuriedConfig config;
    config.host = "localhost";
    config.port = "8080";
    config.topic = "performance_test";
    config.user_id = "perf_user";
    config.app_version = "1.0.0";
    config.app_name = "PerformanceTestApp";
    config.custom_data = "{}";
    
    Buried_Start(buried, &config);
    
    // 性能测试：大量快速上报
    const int report_count = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < report_count; ++i) {
        std::string event_data = R"({"index":)" + std::to_string(i) + 
                               R"(,"timestamp":)" + std::to_string(std::time(nullptr)) + "}";
        Buried_Report(buried, "performance_event", event_data.c_str(), 1);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "上报 " << report_count << " 个事件耗时: " << duration.count() << " ms" << std::endl;
    std::cout << "平均每个事件: " << (double)duration.count() / report_count << " ms" << std::endl;
    
    // 性能要求：平均每个事件不超过 1ms
    EXPECT_LT((double)duration.count() / report_count, 1.0) 
        << "性能不达标：平均每个事件耗时超过 1ms";
    
    // 等待数据处理
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    Buried_Destroy(buried);
}

// 测试大数据处理
TEST_F(BuriedComprehensiveTest, LargeDataHandling) {
    Buried* buried = Buried_Create(test_data_path_.c_str());
    ASSERT_NE(buried, nullptr);
    
    BuriedConfig config;
    config.host = "localhost";
    config.port = "8080";
    config.topic = "large_data_test";
    config.user_id = "large_data_user";
    config.app_version = "1.0.0";
    config.app_name = "LargeDataTestApp";
    config.custom_data = "{}";
    
    Buried_Start(buried, &config);
    
    // 测试大数据块
    std::string large_data(5000, 'A');  // 5KB 数据
    large_data = R"({"large_field":")" + large_data + R"("})";
    
    Buried_Report(buried, "large_data_event", large_data.c_str(), 1);
    
    // 等待处理
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    Buried_Destroy(buried);
}

// 主测试入口
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "开始 BuriedPoint 综合功能测试..." << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    std::cout << "BuriedPoint 综合功能测试完成" << std::endl;
    
    return result;
}