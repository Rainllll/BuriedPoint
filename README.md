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
git clone https://github.com/yourusername/BuriedPoint.git
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

### 4. 运行示例

```bash
# 运行基础示例
./build/[platform]-release/examples/buried_example

# 运行上报示例
./build/[platform]-release/examples/reporter_example
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

## 📊 性能指标

| 指标 | 数值 | 说明 |
|------|------|------|
| 吞吐量 | 10,000+ events/sec | 单线程上报性能 |
| 延迟 | < 1ms | 本地缓存写入延迟 |
| 内存占用 | < 50MB | 典型运行时内存使用 |
| 存储效率 | 压缩比 70% | 数据压缩存储 |
| 网络重试 | 指数退避 | 最大重试 5 次 |

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