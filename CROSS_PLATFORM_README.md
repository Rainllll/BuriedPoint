# BuriedPoint 跨平台构建指南

BuriedPoint 现在支持跨平台构建，可以在 Windows、macOS 和 Linux 系统上编译和运行。

## 支持的平台

- **Windows**: Windows 10/11 (Visual Studio 2017+ 或 MinGW-w64)
- **macOS**: macOS 10.15+ (Xcode Command Line Tools 或 Homebrew)
- **Linux**: Ubuntu 18.04+, CentOS 7+, Arch Linux 等主流发行版

## 系统要求

### 通用要求
- CMake 3.20+
- C++20 兼容的编译器
- Git

### Windows
- **选项1**: Visual Studio 2017 或更新版本
- **选项2**: MinGW-w64 (推荐通过 MSYS2 安装)

### macOS
- Xcode Command Line Tools: `xcode-select --install`
- 或者 Homebrew: `brew install cmake`

### Linux
- GCC 9+ 或 Clang 10+
- 构建工具: `build-essential` (Ubuntu/Debian) 或 `base-devel` (Arch)

## 快速开始

### 自动构建（推荐）

#### Unix-like 系统 (Linux/macOS)
```bash
# 克隆项目
git clone <repository-url>
cd BuriedPoint

# 运行跨平台构建脚本
./build_cross_platform.sh
```

#### Windows
```cmd
# 克隆项目
git clone <repository-url>
cd BuriedPoint

# 运行 Windows 构建脚本
build_cross_platform.bat
```

### 手动构建

#### 1. 安装依赖

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake git
```

**macOS:**
```bash
# 使用 Homebrew
brew install cmake

# 或安装 Xcode Command Line Tools
xcode-select --install
```

**Windows (MSYS2):**
```bash
# 安装 MSYS2 后
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make
```

#### 2. 构建项目

```bash
# 创建构建目录
mkdir build && cd build

# 配置 CMake
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_BURIED_SHARED_LIBS=ON \
         -DBUILD_BURIED_STATIC_LIBS=ON \
         -DBUILD_BURIED_EXAMPLES=ON \
         -DBUILD_BURIED_TEST=ON

# 构建
cmake --build . --config Release --parallel $(nproc)

# 运行测试（可选）
ctest --output-on-failure

# 安装（可选）
sudo cmake --install .
```

## 构建选项

| 选项 | 默认值 | 描述 |
|------|--------|------|
| `BUILD_BURIED_SHARED_LIBS` | ON | 构建动态链接库 |
| `BUILD_BURIED_STATIC_LIBS` | ON | 构建静态链接库 |
| `BUILD_BURIED_EXAMPLES` | OFF | 构建示例程序 |
| `BUILD_BURIED_TEST` | OFF | 构建单元测试 |
| `BUILD_BURIED_FOR_MT` | OFF | 使用静态运行时库 (仅Windows) |

## 平台特定说明

### Windows 特定

#### Visual Studio
- 支持 Visual Studio 2017, 2019, 2022
- 自动检测已安装的 Visual Studio 版本
- 生成 `.sln` 解决方案文件

#### MinGW-w64
- 推荐使用 MSYS2 环境
- 确保 MinGW-w64 工具链在 PATH 中

### macOS 特定

#### 依赖管理
- 使用系统框架: Foundation, CoreServices
- 支持 Intel 和 Apple Silicon (M1/M2)

#### 代码签名
```bash
# 如需代码签名
codesign --force --deep --sign - path/to/binary
```

### Linux 特定

#### 发行版支持
- Ubuntu 18.04+
- CentOS 7+
- Fedora 30+
- Arch Linux
- openSUSE Leap 15+

#### 运行时依赖
```bash
# Ubuntu/Debian
sudo apt-get install libpthread-stubs0-dev

# CentOS/RHEL
sudo yum install glibc-devel
```

## 跨平台特性

### 设备标识
- **Windows**: 使用注册表存储设备ID
- **Unix-like**: 使用 `~/.buried_config` 文件存储

### 系统信息获取
- **Windows**: Windows API (GetVersionEx, GetComputerName)
- **macOS**: sysctl 系统调用
- **Linux**: /proc 文件系统

### 进程时间获取
- **Windows**: GetProcessTimes API
- **macOS**: proc_pidinfo 系统调用
- **Linux**: /proc/[pid]/stat 文件解析

## 故障排除

### 常见问题

#### CMake 配置失败
```bash
# 清理构建目录
rm -rf build/
mkdir build && cd build

# 重新配置
cmake .. -DCMAKE_BUILD_TYPE=Release
```

#### 编译器不兼容
```bash
# 指定编译器
cmake .. -DCMAKE_CXX_COMPILER=g++-9 -DCMAKE_C_COMPILER=gcc-9
```

#### 依赖库缺失
```bash
# 检查系统包管理器
# Ubuntu/Debian
apt list --installed | grep -E "(cmake|build-essential)"

# CentOS/RHEL
yum list installed | grep -E "(cmake|gcc)"

# macOS
brew list | grep cmake
```

### 平台特定问题

#### Windows
- **问题**: "无法找到 MSVC 编译器"
- **解决**: 安装 Visual Studio Build Tools 或完整版 Visual Studio

#### macOS
- **问题**: "xcrun: error: invalid active developer path"
- **解决**: `xcode-select --install`

#### Linux
- **问题**: "fatal error: 'thread' file not found"
- **解决**: 确保使用支持 C++20 的编译器版本

## 性能优化

### 编译优化
```bash
# 发布版本优化
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG"

# 调试版本
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g -O0"
```

### 并行构建
```bash
# 使用所有可用核心
cmake --build . --parallel $(nproc)

# 指定核心数
cmake --build . --parallel 4
```

## 贡献指南

### 添加新平台支持
1. 在 `src/common/common_service_cross_platform.cc` 中添加平台特定代码
2. 更新 `CMakeLists.txt` 中的平台检测逻辑
3. 添加平台特定的构建脚本
4. 更新文档

### 测试
```bash
# 运行所有测试
ctest --output-on-failure

# 运行特定测试
ctest -R "test_name" --output-on-failure
```

## 许可证

本项目遵循原项目的许可证条款。

## 支持

如果遇到跨平台构建问题，请：
1. 检查系统要求是否满足
2. 查看故障排除部分
3. 提交 Issue 并包含以下信息：
   - 操作系统和版本
   - 编译器版本
   - CMake 版本
   - 完整的错误日志