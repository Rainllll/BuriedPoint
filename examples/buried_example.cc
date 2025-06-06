#include <chrono>
#include <thread>

#include "../include/buried.h"

int main() {
  // 程序入口 ==============================================
  // 创建埋点实例，指定数据存储路径为D:/buried
  Buried* buried = Buried_Create("D:/buried");
  if (!buried) {  // 对象创建失败检查
    return -1;
  }

  // SDK配置初始化 =========================================
  BuriedConfig config;
  config.host = "localhost";    // 埋点服务器地址
  config.port = "5678";         // 服务器端口号
  config.topic = "test_topic";  // 数据主题/分类
  config.user_id = "test_user"; // 用户唯一标识
  config.app_version = "1.0.0"; // 应用程序版本
  config.app_name = "test_app"; // 应用程序名称
  config.custom_data = "{\"test\":\"test\"}"; // 自定义元数据（JSON格式）
  
  // 启动埋点服务（建立网络连接等）
  Buried_Start(buried, &config);

  // 多线程上报测试 ========================================
  // 创建第一个上报线程（模拟500ms间隔的持续上报）
  std::thread t1([&]() {
    for (int i = 0; i < 100; ++i) {
      Buried_Report(buried, "test_title", "test_data", i); // 上报测试数据
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });

  // 创建第二个上报线程（不同标题和数据内容）
  std::thread t2([&]() {
    for (int i = 0; i < 100; ++i) {
      Buried_Report(buried, "test_2title", "test_2data", i);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });

  // 等待线程结束 ==========================================
  t1.join();
  t2.join();
  
  // 保持运行状态1小时（实际使用时可能需要事件驱动机制）
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  
  // 清理资源 ==============================================
  Buried_Destroy(buried);
  return 0;  // 正常退出
}
