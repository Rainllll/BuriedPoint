# 构建脚本说明

## 概述

`build_cross_platform.py` 是 BuriedPoint 项目的跨平台构建脚本，支持 Windows、macOS 和 Linux 平台的自动化构建。该脚本基于原有的 Windows 专用构建脚本进行了全面升级和扩展。

## 主要功能特性

| 功能特性 | 支持状态 | 说明 |
|----------|----------|------|
| **跨平台支持** | ✅ | Windows + macOS + Linux |
| **多编译器支持** | ✅ | Visual Studio + MinGW + GCC + Clang |
| **自动平台检测** | ✅ | 自动识别当前操作系统 |
| **依赖检查** | ✅ | 自动检查 CMake 和编译器 |
| **彩色输出** | ✅ | 清晰的状态信息显示 |
| **智能并行构建** | ✅ | 自动检测CPU核心数 |
| **测试支持** | ✅ | 构建和运行测试 |
| **示例程序** | ✅ | 构建示例程序 |
| **增量构建** | ✅ | 支持 --no-clean 选项 |
| **构建结果展示** | ✅ | 自动显示生成的文件 |
| **错误处理** | ✅ | 详细的错误信息和状态反馈 |

## 代码结构

### build_cross_platform.py (350+ 行)
```python
# 核心功能模块
- check_dependencies()      # 依赖检查
- clear()                   # 清理构建目录
- build_windows()          # Windows 平台构建
- build_unix()             # Unix-like 系统构建 (macOS/Linux)
- run_tests()              # 测试运行
- show_build_results()     # 构建结果展示
- main()                   # 主函数和参数解析

# 辅助功能
- get_platform_name()      # 平台名称获取
- get_cpu_count()          # CPU 核心数检测
- print_*()                # 彩色输出函数

# 特点
- 模块化设计
- 跨平台支持
- 用户友好的界面
- 完善的错误处理
```

## 使用方式

### 基本使用
```bash
# 基本构建 (Release 模式)
python3 scripts/build_cross_platform.py

# 带测试和示例程序
python3 scripts/build_cross_platform.py --test --example

# Debug 模式构建
python3 scripts/build_cross_platform.py --config Debug
```

### 高级功能
```bash
# 检查构建依赖
python3 scripts/build_cross_platform.py --check-deps

# 构建后运行测试
python3 scripts/build_cross_platform.py --test --run-tests

# 增量构建 (不清理旧文件)
python3 scripts/build_cross_platform.py --no-clean

# Windows 平台指定架构
python3 scripts/build_cross_platform.py --platform x64
```

## 输出示例

### 典型构建输出
```
[INFO] BuriedPoint 跨平台构建脚本
[INFO] 检测到平台: Macos
[INFO] 检查构建依赖...
[INFO] 找到 cmake version 4.1.0
[INFO] 找到编译器: gcc
[SUCCESS] 依赖检查完成
[INFO] 清理构建目录: /Users/user/BuriedPoint/build
[INFO] 开始 Macos 平台构建 (Release)
[INFO] CMake 配置命令: cmake ../.. -G Unix Makefiles -DCMAKE_BUILD_TYPE=Release...
[INFO] 编译命令: make -j8
[SUCCESS] Macos 平台构建成功 (Release)
[INFO] 构建结果:
  build/macos-release/src/libBuried_static.a
  build/macos-release/src/libBuried_shared.dylib
[SUCCESS] 构建完成!
```

### 依赖检查输出
```
[INFO] BuriedPoint 跨平台构建脚本
[INFO] 检测到平台: Macos
[INFO] 检查构建依赖...
[INFO] 找到 cmake version 4.1.0
[INFO] 找到编译器: gcc
[SUCCESS] 依赖检查完成，所有必需的工具都已安装
```

## 平台支持详情

### Windows 平台
- **编译器**: Visual Studio 2017+ / MinGW-w64
- **生成器**: Visual Studio 17 2022 / MinGW Makefiles
- **架构**: x64 (默认), x86
- **输出**: .exe, .dll, .lib 文件

### macOS 平台
- **编译器**: Xcode Command Line Tools / Homebrew GCC/Clang
- **生成器**: Unix Makefiles
- **架构**: Apple Silicon / Intel (自动检测)
- **输出**: .dylib, .a 文件

### Linux 平台
- **编译器**: GCC 9+ / Clang 10+
- **生成器**: Unix Makefiles
- **架构**: x86_64, ARM64
- **输出**: .so, .a 文件

## 性能特性

| 特性 | 实现方式 | 优势 |
|------|----------|------|
| **并行构建** | 自动检测CPU核心数 | 最大化构建速度 |
| **增量构建** | --no-clean 选项 | 节省重复构建时间 |
| **依赖缓存** | CMake 缓存机制 | 避免重复配置 |
| **智能检测** | 运行时平台识别 | 自动适配环境 |

## 故障排除

### 常见问题
1. **CMake 未找到**: 确保 CMake 已安装并在 PATH 中
2. **编译器未找到**: 安装对应平台的开发工具
3. **权限问题**: 确保对构建目录有写权限
4. **依赖缺失**: 使用 `--check-deps` 检查环境

### 调试技巧
```bash
# 详细输出
python3 scripts/build_cross_platform.py --config Debug

# 仅检查环境
python3 scripts/build_cross_platform.py --check-deps

# 保留构建缓存
python3 scripts/build_cross_platform.py --no-clean
```

## 总结

`build_cross_platform.py` 是一个功能完整的跨平台构建解决方案，提供了：

### 核心优势
- ✅ **真正的跨平台**: 一个脚本适配所有主流桌面平台
- ✅ **智能化构建**: 自动检测环境和优化构建参数
- ✅ **用户友好**: 彩色输出和详细的状态反馈
- ✅ **功能丰富**: 从基础构建到测试运行的完整工作流

### 适用场景
- ✅ **个人开发**: 简化本地构建流程
- ✅ **团队协作**: 统一的构建体验
- ✅ **CI/CD 集成**: 自动化构建和测试
- ✅ **跨平台发布**: 多平台库文件生成