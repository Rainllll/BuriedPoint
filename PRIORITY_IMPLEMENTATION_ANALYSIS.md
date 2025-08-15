# BuriedPoint 优先级上传数据实现详解

## 🎯 **概述**

BuriedPoint 项目通过多层架构实现了基于优先级的数据上传机制，确保高优先级数据能够优先被处理和上报。

## 🏗️ **架构流程**

```
用户调用 API
    ↓
Buried_Report(title, data, priority)
    ↓
BuriedCore::Report()
    ↓
BuriedReport::InsertData()
    ↓
数据库存储 (带优先级)
    ↓
定时器触发 ReportCache_()
    ↓
按优先级查询数据 (ORDER BY priority DESC)
    ↓
网络上报
    ↓
删除已上报数据
```

## 📊 **数据结构设计**

### 1. API 层数据结构
```cpp
// include/buried.h
BURIED_EXPORT int32_t Buried_Report(Buried* buried, const char* title,
                                    const char* data, uint32_t priority);
```

### 2. 内部数据结构
```cpp
// src/report/buried_report.h
struct BuriedData {
  std::string title;
  std::string data;
  uint32_t priority;  // 优先级字段
};
```

### 3. 数据库数据结构
```cpp
// src/database/database.h
struct Data {
  int32_t id;           // 自增主键
  int32_t priority;     // 优先级 (数值越大优先级越高)
  uint64_t timestamp;   // 时间戳
  std::vector<char> content;  // 加密后的数据内容
};
```

## 🔄 **完整实现流程**

### 第1步: API 调用
```cpp
// 用户调用 (高优先级紧急数据)
Buried_Report(buried, "error_event", "critical_error_data", 3);

// 用户调用 (普通数据)
Buried_Report(buried, "click_event", "button_click_data", 1);
```

### 第2步: 数据转换和存储
```cpp
// src/buried_core.cc:68-76
BuriedResult Buried::Report(std::string title, std::string data, uint32_t priority) {
  buried::BuriedData buried_data;
  buried_data.title = std::move(title);
  buried_data.data = std::move(data);
  buried_data.priority = priority;  // 保存优先级
  buried_report_->InsertData(buried_data);
  return BuriedResult::kBuriedOk;
}
```

### 第3步: 异步数据库插入
```cpp
// src/report/buried_report.cc:91-94
void BuriedReportImpl::InsertData(const BuriedData& data) {
  Context::GetGlobalContext().GetReportStrand().post(
      [this, data]() { db_->InsertData(MakeDbData_(data)); });
}
```

### 第4步: 数据库记录构造
```cpp
// src/report/buried_report.cc:136-165
BuriedDb::Data BuriedReportImpl::MakeDbData_(const BuriedData& data) {
  BuriedDb::Data db_data;
  db_data.id = -1;                    // 自增ID
  db_data.priority = data.priority;   // 保存优先级到数据库
  db_data.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
  
  // 构造完整的JSON数据
  nlohmann::json json_data;
  json_data["title"] = data.title;
  json_data["data"] = data.data;
  json_data["priority"] = data.priority;  // JSON中也保存优先级
  // ... 其他字段
  
  // 加密存储
  std::string report_data = crypt_->Encrypt(json_data.dump());
  db_data.content = std::vector<char>(report_data.begin(), report_data.end());
  
  return db_data;
}
```

### 第5步: 数据库存储
```cpp
// src/database/database.cc:30-34
void BuriedDbImpl::InsertData(const BuriedDb::Data& data) {
  auto guard = storage_->transaction_guard();
  storage_->insert(data);  // SQLite插入，包含priority字段
  guard.commit();
}
```

### 第6步: 定时器触发上报
```cpp
// src/report/buried_report.cc:74-89
void BuriedReportImpl::Start() {
  timer_ = std::make_unique<boost::asio::deadline_timer>(
      Context::GetGlobalContext().GetMainContext(),
      boost::posix_time::seconds(5));  // 每5秒触发一次

  timer_->async_wait(Context::GetGlobalContext().GetReportStrand().wrap(
      [this](const boost::system::error_code& ec) {
        if (ec) return;
        ReportCache_();  // 触发数据上报
      }));
}
```

### 第7步: 按优先级查询数据 ⭐ **核心实现**
```cpp
// src/database/database.cc:52-56
std::vector<BuriedDb::Data> BuriedDbImpl::QueryData(int32_t limit_size) {
  auto limited = storage_->get_all<BuriedDb::Data>(
      order_by(&BuriedDb::Data::priority).desc(),  // 按优先级降序排列
      limit(limit_size));                          // 限制数量
  return limited;
}
```

**关键点**: 使用 `order_by(&BuriedDb::Data::priority).desc()` 确保高优先级数据优先被查询出来。

### 第8步: 数据上报处理
```cpp
// src/report/buried_report.cc:105-120
void BuriedReportImpl::ReportCache_() {
  if (data_caches_.empty()) {
    data_caches_ = db_->QueryData(10);  // 查询前10条高优先级数据
  }

  if (!data_caches_.empty()) {
    std::string report_data = GenReportData_(data_caches_);
    if (ReportData_(report_data)) {
      db_->DeleteDatas(data_caches_);  // 上报成功后删除
      data_caches_.clear();
    }
  }

  NextCycle_();  // 继续下一轮
}
```

## 🎯 **优先级策略**

### 优先级定义
- **3 (高优先级)**: 错误事件、崩溃报告、安全事件
- **2 (中优先级)**: 用户行为、业务事件
- **1 (低优先级)**: 统计数据、性能指标

### 处理策略
1. **存储时**: 所有数据都会被存储，不会因为优先级而丢弃
2. **上报时**: 高优先级数据优先被查询和上报
3. **批量处理**: 每次最多处理10条数据，确保高优先级数据能快速上报
4. **持续处理**: 5秒间隔的定时器确保数据持续被处理

## 🔍 **SQL 查询示例**

实际的 SQLite 查询语句：
```sql
SELECT id, priority, timestamp, content 
FROM buried_data 
ORDER BY priority DESC 
LIMIT 10;
```

这确保了：
- 优先级为 3 的数据最先被查询出来
- 相同优先级的数据按插入顺序处理
- 每次最多处理 10 条数据，避免内存占用过大

## 🚀 **性能优化**

1. **索引优化**: priority 字段可以添加索引提高查询性能
2. **批量处理**: 每次处理多条数据，减少网络请求次数
3. **异步处理**: 所有数据库操作都在独立线程中进行
4. **内存缓存**: 查询结果缓存在内存中，避免重复查询

## 📊 **实际效果验证**

### 测试场景
```cpp
// 插入不同优先级的数据
Buried_Report(buried, "low_priority", "data1", 1);    // 低优先级
Buried_Report(buried, "high_priority", "data2", 3);   // 高优先级  
Buried_Report(buried, "medium_priority", "data3", 2); // 中优先级
```

### 上报顺序
1. **第一批**: high_priority (优先级3) 最先被上报
2. **第二批**: medium_priority (优先级2) 其次被上报  
3. **第三批**: low_priority (优先级1) 最后被上报

## 🎯 **总结**

BuriedPoint 的优先级实现通过以下关键技术保证了高优先级数据的优先处理：

1. **数据库层面**: 使用 `ORDER BY priority DESC` 确保查询顺序
2. **应用层面**: 定时器 + 批量处理保证持续性
3. **架构层面**: 异步处理 + 线程安全保证性能
4. **存储层面**: 事务保护 + 加密存储保证可靠性

这种设计既保证了数据的完整性，又确保了关键数据能够优先得到处理，是一个生产级别的优先级队列实现。