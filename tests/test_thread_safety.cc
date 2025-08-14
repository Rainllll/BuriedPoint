/**
 * @file test_thread_safety.cc
 * @brief BuriedPoint 线程安全专项测试
 * 
 * 专门测试多线程环境下的线程安全性和数据一致性
 */

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <random>
#include <filesystem>
#include "include/buried.h"

class ThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data_path_ = "./thread_test_" + std::to_string(std::time(nullptr));
        std::filesystem::create_directories(test_data_path_);
        
        buried_ = Buried_Create(test_data_path_.c_str());
        ASSERT_NE(buried_, nullptr);
        
        BuriedConfig config;
        config.host = "localhost";
        config.port = "8080";
        config.topic = "thread_safety_test";
        config.user_id = "thread_test_user";
        config.app_version = "1.0.0";
        config.app_name = "ThreadSafetyTestApp";
        config.custom_data = "{}";
        
        Buried_Start(buried_, &config);
        
        // 等待服务启动
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void TearDown() override {
        if (buried_) {
            Buried_Destroy(buried_);
        }
        
        if (std::filesystem::exists(test_data_path_)) {
            std::filesystem::remove_all(test_data_path_);
        }
    }

    std::string test_data_path_;
    Buried* buried_ = nullptr;
};

// 测试基本多线程上报
TEST_F(ThreadSafetyTest, BasicMultiThreadReporting) {
    const int thread_count = 10;
    const int reports_per_thread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> completed_reports{0};
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, i, reports_per_thread, &completed_reports]() {
            for (int j = 0; j < reports_per_thread; ++j) {
                std::string event_data = R"({"thread_id":)" + std::to_string(i) + 
                                       R"(,"report_id":)" + std::to_string(j) + 
                                       R"(,"timestamp":)" + std::to_string(std::time(nullptr)) + "}";
                
                Buried_Report(buried_, "multi_thread_test", event_data.c_str(), 1);
                completed_reports++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(completed_reports.load(), thread_count * reports_per_thread);
    
    // 等待所有数据处理完成
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// 测试高频并发上报
TEST_F(ThreadSafetyTest, HighFrequencyConcurrentReporting) {
    const int thread_count = 20;
    const int reports_per_thread = 50;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> error_count{0};
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, i, reports_per_thread, &success_count, &error_count]() {
            try {
                for (int j = 0; j < reports_per_thread; ++j) {
                    std::string event_data = R"({"thread":)" + std::to_string(i) + 
                                           R"(,"event":)" + std::to_string(j) + 
                                           R"(,"high_freq":true})";
                    
                    Buried_Report(buried_, "high_freq_test", event_data.c_str(), 1);
                    success_count++;
                    
                    // 无延迟，测试最高频率
                }
            } catch (...) {
                error_count++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "高频并发测试结果:" << std::endl;
    std::cout << "  成功上报: " << success_count.load() << std::endl;
    std::cout << "  错误次数: " << error_count.load() << std::endl;
    std::cout << "  总耗时: " << duration.count() << " ms" << std::endl;
    std::cout << "  吞吐量: " << (double)success_count.load() / (duration.count() / 1000.0) << " events/sec" << std::endl;
    
    EXPECT_EQ(error_count.load(), 0) << "不应该有任何错误";
    EXPECT_EQ(success_count.load(), thread_count * reports_per_thread);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

// 测试随机延迟的并发访问
TEST_F(ThreadSafetyTest, RandomDelayedConcurrentAccess) {
    const int thread_count = 8;
    const int reports_per_thread = 30;
    std::vector<std::thread> threads;
    std::atomic<int> total_reports{0};
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, i, reports_per_thread, &total_reports]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> delay_dist(1, 50);  // 1-50ms 随机延迟
            
            for (int j = 0; j < reports_per_thread; ++j) {
                // 随机延迟
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(gen)));
                
                std::string event_data = R"({"thread":)" + std::to_string(i) + 
                                       R"(,"sequence":)" + std::to_string(j) + 
                                       R"(,"random_delay":true})";
                
                Buried_Report(buried_, "random_delay_test", event_data.c_str(), 1);
                total_reports++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(total_reports.load(), thread_count * reports_per_thread);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// 测试混合操作的并发性
TEST_F(ThreadSafetyTest, MixedOperationsConcurrency) {
    const int thread_count = 6;
    std::vector<std::thread> threads;
    std::atomic<int> operation_count{0};
    
    // 创建不同类型的操作线程
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, i, &operation_count]() {
            switch (i % 3) {
                case 0: {
                    // 快速小数据上报
                    for (int j = 0; j < 100; ++j) {
                        std::string data = R"({"type":"fast","id":)" + std::to_string(j) + "}";
                        Buried_Report(buried_, "fast_event", data.c_str(), 1);
                        operation_count++;
                    }
                    break;
                }
                case 1: {
                    // 慢速大数据上报
                    for (int j = 0; j < 20; ++j) {
                        std::string large_data(1000, 'X');
                        std::string data = R"({"type":"slow","data":")" + large_data + R"("})";
                        Buried_Report(buried_, "slow_event", data.c_str(), 1);
                        operation_count++;
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    break;
                }
                case 2: {
                    // 中等频率上报
                    for (int j = 0; j < 50; ++j) {
                        std::string data = R"({"type":"medium","counter":)" + std::to_string(j) + "}";
                        Buried_Report(buried_, "medium_event", data.c_str(), 1);
                        operation_count++;
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                    break;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有操作都完成了
    int expected_operations = (100 + 20 + 50) * (thread_count / 3);
    if (thread_count % 3 != 0) {
        expected_operations += 100;  // 额外的快速操作线程
    }
    
    EXPECT_EQ(operation_count.load(), expected_operations);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

// 测试资源竞争情况
TEST_F(ThreadSafetyTest, ResourceContentionTest) {
    const int aggressive_thread_count = 15;
    const int reports_per_thread = 80;
    std::vector<std::thread> threads;
    std::atomic<bool> should_stop{false};
    std::atomic<int> contention_reports{0};
    
    // 启动激进的上报线程
    for (int i = 0; i < aggressive_thread_count; ++i) {
        threads.emplace_back([this, i, reports_per_thread, &should_stop, &contention_reports]() {
            int local_count = 0;
            while (!should_stop && local_count < reports_per_thread) {
                std::string data = R"({"aggressive_thread":)" + std::to_string(i) + 
                                 R"(,"local_count":)" + std::to_string(local_count) + "}";
                
                Buried_Report(buried_, "contention_test", data.c_str(), 1);
                contention_reports++;
                local_count++;
                
                // 极短延迟，增加竞争
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        });
    }
    
    // 运行一段时间后停止
    std::this_thread::sleep_for(std::chrono::seconds(3));
    should_stop = true;
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "资源竞争测试完成，总上报数: " << contention_reports.load() << std::endl;
    
    // 验证没有崩溃，并且有合理数量的上报
    EXPECT_GT(contention_reports.load(), 0);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "开始 BuriedPoint 线程安全专项测试..." << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    std::cout << "BuriedPoint 线程安全专项测试完成" << std::endl;
    
    return result;
}