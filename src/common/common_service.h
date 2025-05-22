#pragma once  // 确保头文件只被编译一次，防止重复包含

#include <stdint.h>  // 包含标准整数类型定义头文件

#include <string>  // 包含标准字符串库头文件

#include "nlohmann/json.hpp"  // 包含第三方 JSON 库头文件

namespace buried {  // 定义一个名为 buried 的命名空间，用于封装相关类和函数

/**
 * @brief 公共服务结构体，包含埋点相关的通用信息和工具方法
 */
struct CommonService {
 public:
  std::string host;  // 主机地址，用于指定服务的访问地址
  std::string port;  // 端口号，用于指定服务的访问端口
  std::string topic;  // 主题，可能用于消息队列或事件订阅等场景
  std::string user_id;  // 用户 ID，用于标识具体用户
  std::string app_version;  // 应用版本号，记录应用当前的版本信息
  std::string app_name;  // 应用名称，标识应用的名称

  nlohmann::json custom_data;  // 自定义数据，使用 JSON 格式存储额外的自定义信息

  std::string system_version;  // 系统版本号，记录设备操作系统的版本信息
  std::string device_name;  // 设备名称，标识设备的型号或名称
  std::string device_id;  // 设备 ID，用于唯一标识设备
  std::string buried_version;  // 埋点版本号，记录埋点系统的版本信息
  std::string lifecycle_id;  // 生命周期 ID，可能用于跟踪应用的生命周期

 public:
  /**
   * @brief 构造函数，用于创建 CommonService 对象
   */
  CommonService();

  /**
   * @brief 获取进程运行时间
   * @return 包含进程运行时间的字符串
   */
  static std::string GetProcessTime();

  /**
   * @brief 获取当前日期
   * @return 包含当前日期的字符串
   */
  static std::string GetNowDate();

  /**
   * @brief 生成随机 ID
   * @return 包含随机 ID 的字符串
   */
  static std::string GetRandomId();

 private:
  /**
   * @brief 初始化函数，用于初始化 CommonService 对象的成员变量
   */
  void Init();
};

}  // namespace buried
