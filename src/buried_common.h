#pragma once

/**
 * @brief 埋点操作结果的枚举类型
 * 
 * 该枚举定义了一系列表示埋点操作结果的常量，用于在埋点相关函数中返回操作状态。
 */
enum BuriedResult {
  // 表示埋点操作成功
  kBuriedOk = 0,
  // 表示埋点操作因传入无效参数而失败
  kBuriedInvalidParam = 1,
  // 表示埋点操作失败
  kBuriedError = 2,
  // 表示埋点操作因未知原因失败
  kBuriedUnknown = -1,
};