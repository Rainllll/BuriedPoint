#include <chrono>
#include <iostream>
#include <thread>

#include "include/buried.h"

int main() {
    std::cout << "=== BuriedPoint 优先级演示 ===" << std::endl;
    
    // 1. 创建埋点实例
    Buried* buried = Buried_Create("./priority_demo_data");
    if (!buried) {
        std::cerr << "❌ 创建埋点实例失败" << std::endl;
        return -1;
    }
    
    // 2. 配置服务
    BuriedConfig config;
    config.host = "httpbin.org";
    config.port = "443";
    config.topic = "priority_test";
    config.user_id = "demo_user";
    config.app_version = "1.0.0";
    config.app_name = "PriorityDemo";
    config.custom_data = "{}";
    
    int32_t result = Buried_Start(buried, &config);
    if (result != 0) {
        std::cerr << "❌ 启动埋点服务失败: " << result << std::endl;
        Buried_Destroy(buried);
        return -1;
    }
    
    std::cout << "✅ 埋点服务启动成功" << std::endl;
    
    // 3. 模拟不同优先级的数据上报
    std::cout << "\n📊 开始上报不同优先级的数据..." << std::endl;
    
    // 低优先级数据 (统计类)
    std::cout << "📈 上报低优先级数据 (优先级=1)" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::string data = "统计数据_" + std::to_string(i);
        Buried_Report(buried, "statistics", data.c_str(), 1);
        std::cout << "  - " << data << " (优先级: 1)" << std::endl;
    }
    
    // 中优先级数据 (用户行为)
    std::cout << "\n🖱️  上报中优先级数据 (优先级=2)" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::string data = "用户行为_" + std::to_string(i);
        Buried_Report(buried, "user_action", data.c_str(), 2);
        std::cout << "  - " << data << " (优先级: 2)" << std::endl;
    }
    
    // 高优先级数据 (错误事件)
    std::cout << "\n🚨 上报高优先级数据 (优先级=3)" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::string data = "错误事件_" + std::to_string(i);
        Buried_Report(buried, "error_event", data.c_str(), 3);
        std::cout << "  - " << data << " (优先级: 3)" << std::endl;
    }
    
    std::cout << "\n📝 所有数据已提交到本地队列" << std::endl;
    std::cout << "💡 数据将按照优先级顺序上报:" << std::endl;
    std::cout << "   1️⃣ 优先级3 (错误事件) - 最先上报" << std::endl;
    std::cout << "   2️⃣ 优先级2 (用户行为) - 其次上报" << std::endl;
    std::cout << "   3️⃣ 优先级1 (统计数据) - 最后上报" << std::endl;
    
    // 4. 等待数据处理
    std::cout << "\n⏳ 等待数据处理和上报..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // 5. 清理资源
    std::cout << "\n🧹 清理资源..." << std::endl;
    Buried_Destroy(buried);
    
    std::cout << "✅ 优先级演示完成!" << std::endl;
    std::cout << "\n📋 技术实现要点:" << std::endl;
    std::cout << "   • 数据库存储: 包含 priority 字段" << std::endl;
    std::cout << "   • 查询排序: ORDER BY priority DESC" << std::endl;
    std::cout << "   • 批量处理: 每次最多10条数据" << std::endl;
    std::cout << "   • 定时上报: 每5秒检查一次队列" << std::endl;
    std::cout << "   • 线程安全: 异步处理保证性能" << std::endl;
    
    return 0;
}