#include "report/buried_report.h"

#include <chrono>
#include <filesystem>
#include <atomic>
#include <mutex>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_service.hpp"
#include "context/context.h"
#include "crypt/crypt.h"
#include "database/database.h"
#include "report/http_report.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace buried {

static const char kDbName[] = "buried.db";

class BuriedReportImpl {
 public:
  BuriedReportImpl(std::shared_ptr<spdlog::logger> logger,
                   CommonService common_service, std::string work_path)
      : logger_(std::move(logger)),
        common_service_(std::move(common_service)),
        work_dir_(std::move(work_path)),
        upload_batch_size_(10),      // 默认每次上传10条
        upload_interval_ms_(5000) {  // 默认5秒间隔
    if (logger_ == nullptr) {
      logger_ = spdlog::stdout_color_mt("buried");
    }
    std::string key = AESCrypt::GetKey("buried_salt", "buried_password");
    crypt_ = std::make_unique<AESCrypt>(key);
    SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl init success");
    Context::GetGlobalContext().GetReportStrand().post([this]() { Init_(); });
  }

  ~BuriedReportImpl() = default;

  void Start();

  void InsertData(const BuriedData& data);
  
  // 动态配置上传参数
  bool SetUploadConfig(uint32_t batch_size, uint32_t interval_ms);
  
  // 获取当前上传配置
  void GetUploadConfig(uint32_t* batch_size, uint32_t* interval_ms);

 private:
  void Init_();

  void ReportCache_();

  void NextCycle_();

  BuriedDb::Data MakeDbData_(const BuriedData& data);

  std::string GenReportData_(const std::vector<BuriedDb::Data>& datas);

  bool ReportData_(const std::string& data);

 private:
  std::shared_ptr<spdlog::logger> logger_;
  std::string work_dir_;
  std::unique_ptr<BuriedDb> db_;
  CommonService common_service_;
  std::unique_ptr<buried::Crypt> crypt_;

  std::unique_ptr<boost::asio::deadline_timer> timer_;

  std::vector<BuriedDb::Data> data_caches_;
  
  // 动态配置参数（线程安全）
  std::atomic<uint32_t> upload_batch_size_;   // 每次上传数据条数
  std::atomic<uint32_t> upload_interval_ms_;  // 上传间隔毫秒数
  std::mutex config_mutex_;                   // 配置变更保护锁
};

void BuriedReportImpl::Init_() {
  std::filesystem::path db_path = work_dir_;
  SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl init db path: {}",
                     db_path.string());
  db_path /= kDbName;
  db_ = std::make_unique<BuriedDb>(db_path.string());
}

void BuriedReportImpl::Start() {
  SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl start");

  uint32_t interval_ms = upload_interval_ms_.load();
  timer_ = std::make_unique<boost::asio::deadline_timer>(
      Context::GetGlobalContext().GetMainContext(),
      boost::posix_time::milliseconds(interval_ms));

  timer_->async_wait(Context::GetGlobalContext().GetReportStrand().wrap(
      [this](const boost::system::error_code& ec) {
        if (ec) {
          logger_->error("BuriedReportImpl::Start error: {}", ec.message());
          return;
        }
        ReportCache_();
      }));
}

void BuriedReportImpl::InsertData(const BuriedData& data) {
  Context::GetGlobalContext().GetReportStrand().post(
      [this, data]() { db_->InsertData(MakeDbData_(data)); });
}

bool BuriedReportImpl::ReportData_(const std::string& data) {
  HttpReporter reporter(logger_);
  return reporter.Host(common_service_.host)
      .Topic(common_service_.topic)
      .Port(common_service_.port)
      .Body(data)
      .Report();
}

void BuriedReportImpl::ReportCache_() {
  SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl report cache");
  if (data_caches_.empty()) {
    uint32_t batch_size = upload_batch_size_.load();
    data_caches_ = db_->QueryData(static_cast<int32_t>(batch_size));
    SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl query {} data items", batch_size);
  }

  if (!data_caches_.empty()) {
    std::string report_data = GenReportData_(data_caches_);
    if (ReportData_(report_data)) {
      db_->DeleteDatas(data_caches_);
      data_caches_.clear();
    }
  }

  NextCycle_();
}

std::string BuriedReportImpl::GenReportData_(
    const std::vector<BuriedDb::Data>& datas) {
  nlohmann::json json_datas;
  for (const auto& data : datas) {
    std::string content =
        crypt_->Decrypt(data.content.data(), data.content.size());
    SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl report data content size: {}",
                       data.content.size());
    json_datas.push_back(content);
  }
  std::string ret = json_datas.dump();
  return ret;
}

BuriedDb::Data BuriedReportImpl::MakeDbData_(const BuriedData& data) {
  BuriedDb::Data db_data;
  db_data.id = -1;
  db_data.priority = data.priority;
  db_data.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
  nlohmann::json json_data;
  json_data["title"] = data.title;
  json_data["data"] = data.data;
  json_data["user_id"] = common_service_.user_id;
  json_data["app_version"] = common_service_.app_version;
  json_data["app_name"] = common_service_.app_name;
  json_data["custom_data"] = common_service_.custom_data;
  json_data["system_version"] = common_service_.system_version;
  json_data["device_name"] = common_service_.device_name;
  json_data["device_id"] = common_service_.device_id;
  json_data["buried_version"] = common_service_.buried_version;
  json_data["lifecycle_id"] = common_service_.lifecycle_id;
  json_data["priority"] = data.priority;
  json_data["timestamp"] = CommonService::GetNowDate();
  json_data["process_time"] = CommonService::GetProcessTime();
  json_data["report_id"] = CommonService::GetRandomId();
  std::string report_data = crypt_->Encrypt(json_data.dump());
  db_data.content = std::vector<char>(report_data.begin(), report_data.end());
  SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl insert data size: {}",
                     db_data.content.size());

  return db_data;
}

void BuriedReportImpl::NextCycle_() {
  SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl next cycle");
  uint32_t interval_ms = upload_interval_ms_.load();
  timer_->expires_at(timer_->expires_at() + boost::posix_time::milliseconds(interval_ms));
  timer_->async_wait([this](const boost::system::error_code& ec) {
    if (ec) {
      logger_->error("BuriedReportImpl::NextCycle_ error: {}", ec.message());
      return;
    }
    Context::GetGlobalContext().GetReportStrand().post(
        [this]() { ReportCache_(); });
  });
}

bool BuriedReportImpl::SetUploadConfig(uint32_t batch_size, uint32_t interval_ms) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  
  // 参数验证
  if (batch_size > 0) {
    if (batch_size < 1 || batch_size > 100) {
      SPDLOG_LOGGER_WARN(logger_, "Invalid batch_size: {}, must be 1-100", batch_size);
      return false;
    }
  }
  
  if (interval_ms > 0) {
    if (interval_ms < 100 || interval_ms > 60000) {
      SPDLOG_LOGGER_WARN(logger_, "Invalid interval_ms: {}, must be 100-60000", interval_ms);
      return false;
    }
  }
  
  // 更新配置
  if (batch_size > 0) {
    uint32_t old_batch = upload_batch_size_.exchange(batch_size);
    SPDLOG_LOGGER_INFO(logger_, "Upload batch size changed: {} -> {}", old_batch, batch_size);
  }
  
  if (interval_ms > 0) {
    uint32_t old_interval = upload_interval_ms_.exchange(interval_ms);
    SPDLOG_LOGGER_INFO(logger_, "Upload interval changed: {}ms -> {}ms", old_interval, interval_ms);
    
    // 如果定时器已经启动，需要重新调度
    if (timer_) {
      Context::GetGlobalContext().GetReportStrand().post([this]() {
        timer_->cancel();  // 取消当前定时器
        uint32_t new_interval = upload_interval_ms_.load();
        timer_->expires_from_now(boost::posix_time::milliseconds(new_interval));
        timer_->async_wait([this](const boost::system::error_code& ec) {
          if (ec && ec != boost::asio::error::operation_aborted) {
            logger_->error("BuriedReportImpl::SetUploadConfig timer error: {}", ec.message());
            return;
          }
          if (!ec) {
            ReportCache_();
          }
        });
      });
    }
  }
  
  return true;
}

void BuriedReportImpl::GetUploadConfig(uint32_t* batch_size, uint32_t* interval_ms) {
  if (batch_size) {
    *batch_size = upload_batch_size_.load();
  }
  if (interval_ms) {
    *interval_ms = upload_interval_ms_.load();
  }
}

// ========

BuriedReport::BuriedReport(std::shared_ptr<spdlog::logger> logger,
                           CommonService common_service, std::string work_path)
    : impl_(std::make_unique<BuriedReportImpl>(
          std::move(logger), std::move(common_service), std::move(work_path))) {
}

void BuriedReport::Start() { impl_->Start(); }

void BuriedReport::InsertData(const BuriedData& data) {
  impl_->InsertData(data);
}

bool BuriedReport::SetUploadConfig(uint32_t batch_size, uint32_t interval_ms) {
  return impl_->SetUploadConfig(batch_size, interval_ms);
}

void BuriedReport::GetUploadConfig(uint32_t* batch_size, uint32_t* interval_ms) {
  impl_->GetUploadConfig(batch_size, interval_ms);
}

BuriedReport::~BuriedReport() {}

}  // namespace buried