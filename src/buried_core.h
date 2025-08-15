#pragma once

#include <stdint.h>

#include <filesystem>
#include <memory>
#include <string>

#include "buried_common.h"
#include "include/buried.h"

namespace spdlog {
class logger;
}

namespace buried {
class BuriedReport;
}

struct Buried {
 public:
  struct Config {
    std::string host;
    std::string port;
    std::string topic;
    std::string user_id;
    std::string app_version;
    std::string app_name;
    std::string custom_data;
    
    // 上传配置参数（可选，使用0表示使用默认值）
    uint32_t upload_batch_size = 0;    // 每次上传的数据条数（默认10）
    uint32_t upload_interval_ms = 0;   // 上传间隔毫秒数（默认5000ms）
  };

 public:
  Buried(const std::string& work_dir);

  ~Buried();

  BuriedResult Start(const Config& config);

  BuriedResult Report(std::string title, std::string data, uint32_t priority);
  
  // 动态配置上传参数
  BuriedResult SetUploadConfig(uint32_t batch_size, uint32_t interval_ms);
  
  // 获取当前上传配置
  BuriedResult GetUploadConfig(uint32_t* batch_size, uint32_t* interval_ms);

 public:
  std::shared_ptr<spdlog::logger> Logger();

 private:
  void InitWorkPath_(const std::string& work_dir);

  void InitLogger_();

 private:
  std::shared_ptr<spdlog::logger> logger_;
  std::unique_ptr<buried::BuriedReport> buried_report_;

  std::filesystem::path work_path_;
};