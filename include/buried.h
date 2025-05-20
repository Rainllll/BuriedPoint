#pragma once  // 防止头文件重复包含

#include <stdint.h>  // 使用标准整型类型

// 定义导出宏（Windows平台动态库导出标记）
#define BURIED_EXPORT __declspec(dllexport)

// C语言接口声明（保证C/C++兼容性）
extern "C" {

// 前向声明埋点核心对象（不透明指针隐藏实现细节）
typedef struct Buried Buried;

// 埋点服务配置结构体
struct BuriedConfig {
  const char* host;        // 埋点服务器地址
  const char* port;        // 服务器端口号
  const char* topic;       // 数据分类主题
  const char* user_id;     // 用户唯一标识
  const char* app_version; // 应用程序版本号
  const char* app_name;    // 应用程序名称
  const char* custom_data; // 自定义元数据（JSON格式）
};

// 创建埋点实例（返回对象指针）
// @param work_dir 数据存储目录路径
BURIED_EXPORT Buried* Buried_Create(const char* work_dir);

// 销毁埋点实例（回收资源）
// @param buried 由Buried_Create创建的对象指针
BURIED_EXPORT void Buried_Destroy(Buried* buried);

// 启动埋点服务（初始化网络连接等）
// @param buried 埋点对象指针
// @param config 配置参数指针
// @return 状态码（0表示成功）
BURIED_EXPORT int32_t Buried_Start(Buried* buried, BuriedConfig* config);

// 上报埋点数据（线程安全）
// @param buried 埋点对象指针
// @param title   事件标题
// @param data    事件数据内容
// @param priority 优先级（数值越大优先级越高）
// @return 状态码（0表示成功）
BURIED_EXPORT int32_t Buried_Report(Buried* buried, const char* title,
                                    const char* data, uint32_t priority);
}
