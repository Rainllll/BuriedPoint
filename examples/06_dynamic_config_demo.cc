/**
 * @file 06_dynamic_config_demo.cc
 * @brief BuriedPoint 动态配置演示
 * 
 * 本示例演示如何动态配置上传批量大小和上传频率，
 * 展示了灵活的配置管理能力。
 */

#include <iostream>
#include <thread>
#include <chrono>
#include "../include/buried.h"

void print_config(Buried* buried) {
    uint32_t batch_size = 0, interval_ms = 0;
    int32_t result = Buried_GetUploadConfig(buried, &batch_size, &interval_ms);
    if (result == 0) {
        std::cout << "📊 当前配置: 批量大小=" << batch_size 
                  << ", 上传间隔=" << interval_ms << "ms" << std::endl;
    } else {
        std::cout << "❌ 获取配置失败" << std::endl;
    }
}

int main() {
    std::cout << "🚀 BuriedPoint 动态配置演示" << std::endl;
    std::cout << "================================" << std::endl;

    // 1. 创建埋点实例
    Buried* buried = Buried_Create("./data");
    if (!buried) {
        std::cout << "❌ 创建埋点实例失败" << std::endl;
        return -1;
    }

    // 2. 配置埋点服务（使用初始配置）
    BuriedConfig config = {};
    config.host = "localhost";
    config.port = "8080";
    config.topic = "analytics";
    config.user_id = "user123";
    config.app_version = "1.0.0";
    config.app_name = "DynamicConfigDemo";
    config.custom_data = "{}";
    
    // 设置初始上传配置：每次5条，间隔2秒
    config.upload_batch_size = 5;
    config.upload_interval_ms = 2000;

    int32_t result = Buried_Start(buried, &config);
    if (result != 0) {
        std::cout << "❌ 启动埋点服务失败: " << result << std::endl;
        Buried_Destroy(buried);
        return -1;
    }

    std::cout << "✅ 埋点服务启动成功" << std::endl;
    
    // 3. 显示初始配置
    std::cout << "\n📋 初始配置:" << std::endl;
    print_config(buried);

    // 4. 上报一些数据
    std::cout << "\n📤 上报初始数据..." << std::endl;
    for (int i = 1; i <= 8; ++i) {
        std::string data = "初始数据_" + std::to_string(i);
        Buried_Report(buried, "initial_event", data.c_str(), 2);
        std::cout << "  上报: " << data << std::endl;
    }

    // 等待一个上传周期
    std::cout << "\n⏳ 等待2秒观察上传..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 5. 动态修改配置 - 增加批量大小
    std::cout << "\n🔧 动态配置1: 增加批量大小到15条" << std::endl;
    result = Buried_SetUploadConfig(buried, 15, 0);  // 0表示不修改间隔
    if (result == 0) {
        std::cout << "✅ 配置更新成功" << std::endl;
        print_config(buried);
    } else {
        std::cout << "❌ 配置更新失败" << std::endl;
    }

    // 上报更多数据
    std::cout << "\n📤 上报更多数据..." << std::endl;
    for (int i = 1; i <= 20; ++i) {
        std::string data = "批量数据_" + std::to_string(i);
        Buried_Report(buried, "batch_event", data.c_str(), 1);
        if (i % 5 == 0) {
            std::cout << "  已上报 " << i << " 条数据" << std::endl;
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 6. 动态修改配置 - 减少上传间隔
    std::cout << "\n🔧 动态配置2: 减少上传间隔到1秒" << std::endl;
    result = Buried_SetUploadConfig(buried, 0, 1000);  // 0表示不修改批量大小
    if (result == 0) {
        std::cout << "✅ 配置更新成功" << std::endl;
        print_config(buried);
    } else {
        std::cout << "❌ 配置更新失败" << std::endl;
    }

    // 上报高频数据
    std::cout << "\n📤 上报高频数据..." << std::endl;
    for (int i = 1; i <= 10; ++i) {
        std::string data = "高频数据_" + std::to_string(i);
        Buried_Report(buried, "frequent_event", data.c_str(), 3);
        std::cout << "  上报: " << data << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 7. 测试无效配置
    std::cout << "\n🔧 测试无效配置: 批量大小200条（超出限制）" << std::endl;
    result = Buried_SetUploadConfig(buried, 200, 0);
    if (result == 0) {
        std::cout << "✅ 配置更新成功" << std::endl;
    } else {
        std::cout << "❌ 配置更新失败（预期行为）" << std::endl;
    }
    print_config(buried);

    // 8. 恢复合理配置
    std::cout << "\n🔧 恢复合理配置: 批量10条，间隔3秒" << std::endl;
    result = Buried_SetUploadConfig(buried, 10, 3000);
    if (result == 0) {
        std::cout << "✅ 配置更新成功" << std::endl;
        print_config(buried);
    } else {
        std::cout << "❌ 配置更新失败" << std::endl;
    }

    // 9. 最终数据上报
    std::cout << "\n📤 最终数据上报..." << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::string data = "最终数据_" + std::to_string(i);
        Buried_Report(buried, "final_event", data.c_str(), 2);
        std::cout << "  上报: " << data << std::endl;
    }

    std::cout << "\n⏳ 等待最后一次上传..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(4));

    // 10. 清理资源
    std::cout << "\n🧹 清理资源..." << std::endl;
    Buried_Destroy(buried);

    std::cout << "\n✅ 动态配置演示完成！" << std::endl;
    std::cout << "\n💡 总结:" << std::endl;
    std::cout << "   - 支持动态修改上传批量大小（1-100条）" << std::endl;
    std::cout << "   - 支持动态修改上传间隔（100-60000ms）" << std::endl;
    std::cout << "   - 配置立即生效，无需重启服务" << std::endl;
    std::cout << "   - 自动验证参数有效性" << std::endl;
    std::cout << "   - 线程安全的配置操作" << std::endl;

    return 0;
}