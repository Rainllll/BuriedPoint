# BuriedPoint 跨平台构建脚本使用指南

## 概述

`build_cross_platform.py` 是基于原有的 `build.py` 脚本扩展的跨平台构建脚本，支持 Windows、macOS 和 Linux 平台的自动化构建。

## 特性

- 🔍 **自动平台检测**: 自动识别当前操作系统
- 🛠 **依赖检查**: 自动检查 CMake 和编译器
- 🎨 **彩色输出**: 清晰的状态信息显示
- ⚡ **并行构建**: 自动使用多核心加速编译
- 🧪 **测试支持**: 可选的测试构建和运行
- 📦 **示例程序**: 可选的示例程序构建

## 基本用法

### 快速开始

```bash
# 检查依赖
python3 scripts/build_cross_platform.py --check-deps

# 默认构建 (Release 配置)
python3 scripts/build_cross_platform.py

# 构建 Debug 版本
python3 scripts/build_cross_platform.py --config Debug
```

### 完整参数列表

```bash
python3 scripts/build_cross_platform.py [选项]

选项:
  --config {Debug,Release}  构建配置 (默认: Release)
  --platform PLATFORM      Windows 平台架构 (默认: x64)
  --test                   启用单元测试构建
  --example                启用示例程序构建
  --run-tests              构建后运行测试
  --no-clean               不清理构建目录
  --check-deps             只检查依赖，不进行构建
  -h, --help               显示帮助信息
```

## 使用示例

### 1. 开发构建
```bash
# 构建 Debug 版本，包含测试和示例
python3 scripts/build_cross_platform.py --config Debug --test --example
```

### 2. 发布构建
```bash
# 构建 Release 版本，仅核心库
python3 scripts/build_cross_platform.py --config Release
```

### 3. 完整测试
```bash
# 构建并运行测试
python3 scripts/build_cross_platform.py --test --run-tests
```

### 4. 增量构建
```bash
# 不清理构建目录，进行增量构建
python3 scripts/build_cross_platform.py --no-clean
```

## 平台特定说明

### Windows
- **支持编译器**: Visual Studio 2017+, MinGW-w64
- **自动检测**: 优先使用 Visual Studio，回退到 MinGW
- **构建目录**: `build/x64-Release` 或 `build/x64-Debug`

### macOS
- **支持编译器**: Xcode Command Line Tools, Homebrew GCC/Clang
- **构建目录**: `build/macos-release` 或 `build/macos-debug`
- **系统要求**: macOS 10.15+

### Linux
- **支持编译器**: GCC 9+, Clang 10+
- **构建目录**: `build/linux-release` 或 `build/linux-debug`
- **发行版**: Ubuntu 18.04+, CentOS 7+, Arch Linux

## 输出说明

### 构建产物位置

**Windows:**
```
build/x64-Release/
├── src/
│   ├── Buried_static.lib      # 静态库
│   └── Buried_shared.dll      # 动态库
└── examples/                  # 示例程序 (如果启用)
```

**macOS/Linux:**
```
build/macos-release/  (或 linux-release)
├── src/
│   ├── libBuried_static.a     # 静态库
│   └── libBuried_shared.dylib # 动态库 (.so for Linux)
└── examples/                  # 示例程序 (如果启用)
```

### 状态信息

脚本使用彩色输出显示不同类型的信息：
- 🔵 **[INFO]**: 一般信息
- 🟢 **[SUCCESS]**: 成功操作
- 🟡 **[WARNING]**: 警告信息
- 🔴 **[ERROR]**: 错误信息

## 故障排除

### 常见问题

1. **CMake 未找到**
   ```
   [ERROR] CMake 未安装或不在 PATH 中
   ```
   **解决**: 安装 CMake 并确保在 PATH 中

2. **编译器未找到**
   ```
   [ERROR] 未找到合适的编译器
   ```
   **解决**: 安装对应平台的编译器

3. **构建失败**
   ```
   [ERROR] 构建失败: ...
   ```
   **解决**: 检查错误信息，通常是依赖或配置问题

### 调试技巧

1. **查看详细输出**
   ```bash
   python3 scripts/build_cross_platform.py --config Debug
   ```

2. **保留构建目录**
   ```bash
   python3 scripts/build_cross_platform.py --no-clean
   ```

3. **单独检查依赖**
   ```bash
   python3 scripts/build_cross_platform.py --check-deps
   ```

## 与原版 build.py 的对比

| 特性 | 原版 build.py | 新版 build_cross_platform.py |
|------|---------------|-------------------------------|
| 平台支持 | 仅 Windows | Windows + macOS + Linux |
| 编译器支持 | 仅 Visual Studio | VS + MinGW + GCC + Clang |
| 依赖检查 | 无 | 自动检查 CMake 和编译器 |
| 彩色输出 | 无 | 支持彩色状态信息 |
| 测试运行 | 无 | 支持构建后自动运行测试 |
| 增量构建 | 无 | 支持 --no-clean 选项 |

## 高级用法

### 自定义构建目录
```bash
# 修改脚本中的 BUILD_DIR_PATH 变量
BUILD_DIR_PATH = os.path.join(PROJECT_ROOT, 'custom_build')
```

### 添加自定义 CMake 参数
在脚本的 `cmake_args` 列表中添加参数：
```python
cmake_args.extend(['-DCUSTOM_OPTION=ON'])
```

### 集成到 CI/CD
```yaml
# GitHub Actions 示例
- name: Build BuriedPoint
  run: python3 scripts/build_cross_platform.py --test --run-tests
```

## 贡献

如需改进构建脚本，请：
1. 保持与原版 `build.py` 的兼容性
2. 添加适当的错误处理
3. 更新此文档
4. 测试所有支持的平台