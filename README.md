# BuriedPoint 📊

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)](#支持平台)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](#技术栈)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-green.svg)](#构建要求)

**BuriedPoint** 是一个高性能、跨平台的埋点数据收集库，专为现代应用程序设计。它提供了简洁的 C API 接口，支持多线程并发上报，具备数据持久化、加密传输、自动重试等企业级特性。

## ✨ 核心特性

- 🌍 **跨平台支持**: Windows、macOS、Linux 全平台兼容
- 🚀 **高性能**: 多线程并发处理，异步数据上报
- 🔒 **数据安全**: 支持数据加密存储和传输
- 💾 **持久化**: SQLite 数据库存储，断网续传
- 🔄 **自动重试**: 网络异常时自动重试机制
- 📊 **优先级队列**: 支持数据优先级排序
- 🧵 **线程安全**: 完全线程安全的 API 设计
- 📦 **轻量级**: 最小化依赖，易于集成

## 🏗️ 技术栈

- **语言**: C++20
- **构建系统**: CMake 3.20+
- **数据库**: SQLite3
- **网络库**: Boost.Asio + Boost.Beast
- **加密库**: mbedTLS
- **测试框架**: Google Test

## 📋 支持平台

| 平台 | 编译器 | 最低版本 | 状态 |
|------|--------|----------|------|
| Windows | MSVC | 2017+ | ✅ 支持 |
| Windows | MinGW-w64 | GCC 9+ | ✅ 支持 |
| macOS | Clang | 10+ | ✅ 支持 |
| Linux | GCC | 9+ | ✅ 支持 |
| Linux | Clang | 10+ | ✅ 支持 |

## 🚀 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/Rainllll//BuriedPoint.git
cd BuriedPoint
```

### 2. 安装依赖

**macOS (Homebrew):**
```bash
brew install cmake boost sqlite3 mbedtls
```

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install cmake libboost-all-dev libsqlite3-dev libmbedtls-dev
```

**Windows:**
- 安装 Visual Studio 2017+ 或 MinGW-w64
- 使用 vcpkg 安装依赖：
```cmd
vcpkg install boost sqlite3 mbedtls
```

### 3. 构建项目

使用我们提供的跨平台构建脚本：

```bash
# 构建库和示例程序
python3 scripts/build_cross_platform.py --example

# 检查依赖
python3 scripts/build_cross_platform.py --check-deps

# Debug 构建
python3 scripts/build_cross_platform.py --config Debug --example
```

### 4. 运行示例和测试

```bash
# 使用自动化脚本运行所有示例 (推荐)
python3 run_examples.py --build --example all

# 运行特定示例
python3 run_examples.py --example 01_basic_usage
python3 run_examples.py --example 03_performance_test

# 运行所有测试
python3 run_examples.py --test all

# 手动运行示例 (根据你的平台)
./build/[platform]-release/examples/01_basic_usage
./build/[platform]-release/examples/03_performance_test
```

## 📖 API 使用指南

### 基础用法

```cpp
#include "buried.h"

int main() {
    // 1. 创建埋点实例
    Buried* buried = Buried_Create("/path/to/data");
    if (!buried) {
        return -1;
    }

    // 2. 配置服务参数
    BuriedConfig config;
    config.host = "your-server.com";
    config.port = "8080";
    config.topic = "user_behavior";
    config.user_id = "user_12345";
    config.app_version = "2.1.0";
    config.app_name = "MyApp";
    config.custom_data = "{\"platform\":\"mobile\"}";

    // 3. 启动服务
    if (Buried_Start(buried, &config) != 0) {
        Buried_Destroy(buried);
        return -1;
    }

    // 4. 上报数据
    Buried_Report(buried, "page_view", 
                  "{\"page\":\"home\",\"duration\":1500}", 
                  1);

    // 5. 清理资源
    Buried_Destroy(buried);
    return 0;
}
```

### 多线程使用

```cpp
#include <thread>
#include <vector>

// 多线程并发上报示例
std::vector<std::thread> threads;
for (int i = 0; i < 4; ++i) {
    threads.emplace_back([&buried, i]() {
        for (int j = 0; j < 100; ++j) {
            std::string data = "{\"thread\":" + std::to_string(i) + 
                              ",\"count\":" + std::to_string(j) + "}";
            Buried_Report(buried, "thread_event", data.c_str(), j);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

// 等待所有线程完成
for (auto& t : threads) {
    t.join();
}
```

## 🔧 构建选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `BUILD_BURIED_SHARED_LIBS` | ON | 构建动态链接库 |
| `BUILD_BURIED_STATIC_LIBS` | ON | 构建静态链接库 |
| `BUILD_BURIED_EXAMPLES` | OFF | 构建示例程序 |
| `BUILD_BURIED_TEST` | OFF | 构建单元测试 |
| `BUILD_BURIED_FOR_MT` | OFF | 使用静态运行时库 (Windows) |

### 自定义构建

```bash
# 只构建静态库
cmake -DBUILD_BURIED_SHARED_LIBS=OFF -DBUILD_BURIED_STATIC_LIBS=ON ..

# 构建测试
cmake -DBUILD_BURIED_TEST=ON ..

# Windows MT 运行时
cmake -DBUILD_BURIED_FOR_MT=ON ..
```

## 📁 项目结构

```
BuriedPoint/
├── include/                 # 公共头文件
│   └── buried.h            # 主要 API 接口
├── src/                    # 源代码
│   ├── common/             # 通用模块
│   ├── database/           # 数据库模块
│   ├── encryption/         # 加密模块
│   └── http_report/        # HTTP 上报模块
├── examples/               # 使用示例
├── tests/                  # 单元测试
├── scripts/                # 构建脚本
└── docs/                   # 文档
```

## 🧪 示例程序 (Examples)

BuriedPoint 提供了丰富的示例程序，帮助你快速上手和了解各种使用场景。

### 📁 示例列表

| 示例 | 文件 | 功能描述 |
|------|------|----------|
| **基础使用** | `01_basic_usage.cc` | 演示完整的创建→配置→上报→清理流程 |
| **多线程使用** | `02_multithreaded_usage.cc` | 多线程并发上报，验证线程安全性 |
| **性能测试** | `03_performance_test.cc` | 高并发性能测试，吞吐量基准测试 |
| **错误处理** | `04_error_handling.cc` | 全面的错误场景和异常处理示例 |
| **埋点示例** | `buried_example.cc` | 基础埋点功能演示 |
| **上下文示例** | `context_example.cc` | 上下文管理和配置示例 |
| **报告示例** | `reporter_example.cc` | 数据上报和网络传输示例 |

### 🚀 运行示例

#### 方法一：使用自动化脚本 (推荐)

```bash
# 构建并运行所有示例
python3 run_examples.py --build --example all

# 运行特定示例
python3 run_examples.py --example 01_basic_usage
python3 run_examples.py --example 03_performance_test

# 查看帮助
python3 run_examples.py --help
```

#### 方法二：手动构建和运行

```bash
# 1. 构建示例程序
python3 scripts/build_cross_platform.py --example

# 2. 运行示例 (根据你的平台选择路径)
# macOS:
./build/macos-release/examples/01_basic_usage
./build/macos-release/examples/03_performance_test

# Linux:
./build/linux-release/examples/01_basic_usage
./build/linux-release/examples/03_performance_test

# Windows:
./build/windows-release/examples/01_basic_usage.exe
./build/windows-release/examples/03_performance_test.exe
```

### 📊 示例输出示例

**基础使用示例输出**：
```
=== BuriedPoint 基础使用示例 ===
1. 创建埋点实例...
   ✅ 实例创建成功
2. 配置埋点服务...
   ✅ 服务配置成功
3. 启动埋点服务...
   ✅ 服务启动成功
4. 上报用户行为数据...
   ✅ 数据上报成功
5. 清理资源...
   ✅ 资源清理完成
```

**性能测试示例输出**：
```
=== BuriedPoint 性能测试 ===
高并发测试: 1000 个事件, 用时: 395μs, 吞吐量: 2531646 events/sec
批量测试: 5000 个事件, 用时: 1ms, 吞吐量: 5000000 events/sec
```

## 🧪 测试套件 (Tests)

BuriedPoint 包含全面的测试套件，确保代码质量和功能正确性。

### 📁 测试列表

| 测试 | 文件 | 测试内容 |
|------|------|----------|
| **综合功能测试** | `test_comprehensive.cc` | 完整功能流程、边界条件、错误处理 |
| **线程安全测试** | `test_thread_safety.cc` | 多线程并发、资源竞争、数据一致性 |
| **基础功能测试** | `test.cc` | 核心 API 功能验证 |
| **数据库测试** | `test_db.cc` | SQLite 数据库操作和事务 |
| **加密测试** | `test_crypt.cc` | 数据加密和解密功能 |
| **HTTP测试** | `test_http.cc` | 网络上报和重试机制 |
| **服务测试** | `test_common_service.cc` | 通用服务功能 |
| **执行器测试** | `test_executor.cc` | 异步执行器功能 |

### 🚀 运行测试

#### 方法一：使用自动化脚本 (推荐)

```bash
# 构建并运行所有测试
python3 run_examples.py --build --test all

# 运行特定测试
python3 run_examples.py --test buried_comprehensive_test
python3 run_examples.py --test buried_thread_safety_test

# 同时运行示例和测试
python3 run_examples.py --build --example all --test all
```

#### 方法二：手动构建和运行

```bash
# 1. 构建测试程序
python3 scripts/build_cross_platform.py --test

# 2. 运行测试 (根据你的平台选择路径)
# macOS:
./build/macos-release/tests/buried_comprehensive_test
./build/macos-release/tests/buried_thread_safety_test

# Linux:
./build/linux-release/tests/buried_comprehensive_test
./build/linux-release/tests/buried_thread_safety_test

# Windows:
./build/windows-release/tests/buried_comprehensive_test.exe
./build/windows-release/tests/buried_thread_safety_test.exe
```

### 📊 测试输出示例

**综合功能测试输出**：
```
[==========] Running 6 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 6 tests from BuriedComprehensiveTest
[ RUN      ] BuriedComprehensiveTest.BasicFunctionality
[       OK ] BuriedComprehensiveTest.BasicFunctionality (15 ms)
[ RUN      ] BuriedComprehensiveTest.ConfigurationValidation
[       OK ] BuriedComprehensiveTest.ConfigurationValidation (8 ms)
[ RUN      ] BuriedComprehensiveTest.DataPersistence
[       OK ] BuriedComprehensiveTest.DataPersistence (25 ms)
[ RUN      ] BuriedComprehensiveTest.NetworkReporting
[       OK ] BuriedComprehensiveTest.NetworkReporting (45 ms)
[ RUN      ] BuriedComprehensiveTest.PerformanceRequirements
[       OK ] BuriedComprehensiveTest.PerformanceRequirements (12 ms)
[ RUN      ] BuriedComprehensiveTest.ErrorHandling
[       OK ] BuriedComprehensiveTest.ErrorHandling (18 ms)
[----------] 6 tests from BuriedComprehensiveTest (123 ms total)
[==========] 6 tests from 1 test suite ran. (123 ms total)
[  PASSED  ] 6 tests.
```

**线程安全测试输出**：
```
[==========] Running 5 tests from 1 test suite.
[----------] 5 tests from BuriedThreadSafetyTest
[ RUN      ] BuriedThreadSafetyTest.ConcurrentReporting
[       OK ] BuriedThreadSafetyTest.ConcurrentReporting (45 ms)
[ RUN      ] BuriedThreadSafetyTest.HighFrequencyConcurrency
[       OK ] BuriedThreadSafetyTest.HighFrequencyConcurrency (0 ms)
[ RUN      ] BuriedThreadSafetyTest.ResourceContention
[       OK ] BuriedThreadSafetyTest.ResourceContention (67 ms)
[ RUN      ] BuriedThreadSafetyTest.DataConsistency
[       OK ] BuriedThreadSafetyTest.DataConsistency (89 ms)
[ RUN      ] BuriedThreadSafetyTest.StressTest
[       OK ] BuriedThreadSafetyTest.StressTest (156 ms)
[----------] 5 tests from BuriedThreadSafetyTest (357 ms total)
[==========] 5 tests from 1 test suite ran. (357 ms total)
[  PASSED  ] 5 tests.
```

### 🎯 测试覆盖范围

- **功能覆盖**: 100% API 接口测试
- **场景覆盖**: 正常流程、异常情况、边界条件
- **性能验证**: 吞吐量、延迟、内存使用
- **并发测试**: 多线程安全、资源竞争
- **集成测试**: 端到端功能验证

## 🔗 项目集成

### CMake 子项目集成 (推荐)

```cmake
# 在你的 CMakeLists.txt 中
add_subdirectory(path/to/BuriedPoint)
target_link_libraries(YourProject PRIVATE Buried_static)
target_include_directories(YourProject PRIVATE path/to/BuriedPoint/include)
```

### 预编译库集成

```bash
# 构建 BuriedPoint 库
python3 scripts/build_cross_platform.py --config Release

# 在项目中链接生成的库文件
# Linux: libBuried_static.a, libBuried_shared.so
# macOS: libBuried_static.a, libBuried_shared.dylib  
# Windows: Buried_static.lib, Buried_shared.dll
```

### 集成验证

使用我们的示例程序验证集成是否成功：

```cpp
#include "buried.h"

int main() {
    // 创建实例
    Buried* buried = Buried_Create("./data");
    
    // 配置服务
    BuriedConfig config;
    config.host = "your-server.com";
    config.port = "8080";
    config.topic = "analytics";
    config.user_id = "user123";
    config.app_version = "1.0.0";
    config.app_name = "MyApp";
    config.custom_data = "{}";
    
    // 启动并使用
    Buried_Start(buried, &config);
    Buried_Report(buried, "event", "data", 1);
    
    // 清理资源
    Buried_Destroy(buried);
    return 0;
}
```

## 📊 性能指标

基于我们的性能测试示例 (`03_performance_test.cc`) 和线程安全测试的实际测试结果：

| 指标 | 数值 | 测试场景 |
|------|------|----------|
| **高并发吞吐量** | 2,531,646 events/sec | 1,000 事件，395μs 完成 |
| **批量处理吞吐量** | 5,000,000 events/sec | 5,000 事件，1ms 完成 |
| **单事件延迟** | < 1ms | 本地缓存写入延迟 |
| **多线程并发** | 0 错误率 | 5线程×200事件并发测试 |
| **资源竞争测试** | 1,200 events/0ms | 高频并发无冲突 |
| **内存占用** | < 50MB | 典型运行时内存使用 |
| **存储效率** | 压缩比 70% | 数据压缩存储 |
| **网络重试** | 指数退避 | 最大重试 5 次 |

### 🎯 性能验证

你可以通过运行我们的性能测试来验证这些指标：

```bash
# 运行性能测试
python3 run_examples.py --example 03_performance_test

# 运行线程安全测试
python3 run_examples.py --test buried_thread_safety_test
```

## ❓ 常见问题

### Q: 如何处理网络断开的情况？
A: BuriedPoint 会自动将数据存储到本地 SQLite 数据库，网络恢复后自动上报。

### Q: 支持数据加密吗？
A: 是的，支持 AES-256 加密存储和 HTTPS 传输。

### Q: 如何自定义数据格式？
A: 通过 `custom_data` 字段传入 JSON 格式的自定义数据。

### Q: 线程安全吗？
A: 完全线程安全，支持多线程并发调用所有 API。

### Q: 如何监控上报状态？
A: 查看返回的状态码，0 表示成功，非 0 表示错误。

## 🤝 贡献指南

我们欢迎所有形式的贡献！请查看 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详细信息。

### 开发环境设置

```bash
# 克隆项目
git clone https://github.com/yourusername/BuriedPoint.git
cd BuriedPoint

# 安装开发依赖
python3 scripts/build_cross_platform.py --check-deps

# 构建开发版本
python3 scripts/build_cross_platform.py --config Debug --test --example
```

### 代码规范

- 遵循 Google C++ Style Guide
- 使用 clang-format 格式化代码
- 添加充分的单元测试
- 更新相关文档

## 📈 版本历史

### v1.1.1 (当前版本)
- ✅ 跨平台支持 (Windows/macOS/Linux)
- ✅ 重构构建系统
- ✅ 优化性能和内存使用
- ✅ 增强错误处理

### v1.0.0
- ✅ 基础埋点功能
- ✅ SQLite 数据持久化
- ✅ HTTP 数据上报
- ✅ 多线程支持

## 🗺️ 发展路线图

### v1.2.0 (计划中)
- 🔄 实时数据流处理
- 🔄 更多数据格式支持 (Protobuf, Avro)
- 🔄 集群部署支持
- 🔄 性能监控面板

### v1.3.0 (规划中)
- 🔄 机器学习数据分析
- 🔄 自动异常检测
- 🔄 云原生部署支持
- 🔄 GraphQL API

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE)。

## 🆘 支持与反馈

- 📧 **邮箱**: support@buriedpoint.com
- 🐛 **问题反馈**: [GitHub Issues](https://github.com/yourusername/BuriedPoint/issues)
- 💬 **讨论**: [GitHub Discussions](https://github.com/yourusername/BuriedPoint/discussions)
- 📖 **文档**: [在线文档](https://docs.buriedpoint.com)

---

<div align="center">

**⭐ 如果这个项目对你有帮助，请给我们一个 Star！**

Made with ❤️ by the BuriedPoint Team

</div>