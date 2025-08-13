#!/usr/bin/env python3
"""
BuriedPoint 跨平台构建脚本
基于原有的 build.py 扩展，支持 Windows、macOS 和 Linux 平台
"""

import os
import sys
import shutil
import argparse
import platform
import subprocess
import multiprocessing

# 脚本路径配置
SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_PATH, '..'))
BUILD_DIR_PATH = os.path.join(PROJECT_ROOT, 'build')

# 平台检测
CURRENT_OS = platform.system().lower()
IS_WINDOWS = CURRENT_OS == 'windows'
IS_MACOS = CURRENT_OS == 'darwin'
IS_LINUX = CURRENT_OS == 'linux'

# 颜色输出支持
class Colors:
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    MAGENTA = '\033[95m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

def print_info(msg):
    """打印信息消息"""
    print(f"{Colors.BLUE}[INFO]{Colors.ENDC} {msg}")

def print_success(msg):
    """打印成功消息"""
    print(f"{Colors.GREEN}[SUCCESS]{Colors.ENDC} {msg}")

def print_warning(msg):
    """打印警告消息"""
    print(f"{Colors.YELLOW}[WARNING]{Colors.ENDC} {msg}")

def print_error(msg):
    """打印错误消息"""
    print(f"{Colors.RED}[ERROR]{Colors.ENDC} {msg}")

def get_cpu_count():
    """获取CPU核心数"""
    try:
        return multiprocessing.cpu_count()
    except:
        return 4

def check_dependencies():
    """检查构建依赖"""
    print_info("检查构建依赖...")
    
    # 检查 CMake
    try:
        result = subprocess.run(['cmake', '--version'], 
                              capture_output=True, text=True, check=True)
        cmake_version = result.stdout.split('\n')[0]
        print_info(f"找到 {cmake_version}")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print_error("CMake 未安装或不在 PATH 中")
        return False
    
    # 检查编译器
    compiler_found = False
    if IS_WINDOWS:
        # 检查 Visual Studio
        try:
            subprocess.run(['cl'], capture_output=True, check=True)
            print_info("找到 Visual Studio 编译器")
            compiler_found = True
        except:
            pass
        
        # 检查 MinGW
        if not compiler_found:
            try:
                result = subprocess.run(['gcc', '--version'], 
                                      capture_output=True, text=True, check=True)
                print_info("找到 MinGW GCC 编译器")
                compiler_found = True
            except:
                pass
    else:
        # Unix-like 系统检查 GCC/Clang
        for compiler in ['gcc', 'clang', 'g++', 'clang++']:
            try:
                result = subprocess.run([compiler, '--version'], 
                                      capture_output=True, text=True, check=True)
                print_info(f"找到编译器: {compiler}")
                compiler_found = True
                break
            except:
                continue
    
    if not compiler_found:
        print_error("未找到合适的编译器")
        return False
    
    print_success("依赖检查完成")
    return True

def clear():
    """清理构建目录"""
    if os.path.exists(BUILD_DIR_PATH):
        print_info(f"清理构建目录: {BUILD_DIR_PATH}")
        shutil.rmtree(BUILD_DIR_PATH)

def get_platform_name():
    """获取平台名称"""
    if IS_WINDOWS:
        return "windows"
    elif IS_MACOS:
        return "macos"
    elif IS_LINUX:
        return "linux"
    else:
        return "unknown"

def build_windows(platform='x64', config='Release', args=None):
    """Windows 平台构建"""
    print_info(f"开始 Windows 平台构建 ({platform}-{config})")
    
    # 创建平台专属构建目录
    platform_dir = os.path.join(BUILD_DIR_PATH, f'{platform}-{config}')
    os.makedirs(platform_dir, exist_ok=True)
    
    original_dir = os.getcwd()
    os.chdir(platform_dir)
    
    try:
        # 检测 Visual Studio 版本
        vs_generator = "Visual Studio 17 2022"  # 默认 VS2022
        try:
            # 尝试检测已安装的 Visual Studio
            subprocess.run(['cl'], capture_output=True, check=True)
        except:
            # 如果没有 cl，尝试使用 MinGW
            try:
                subprocess.run(['gcc', '--version'], capture_output=True, check=True)
                vs_generator = "MinGW Makefiles"
                print_info("使用 MinGW Makefiles 生成器")
            except:
                print_error("未找到合适的编译器")
                return False
        
        # 生成 CMake 命令
        cmake_args = [
            'cmake', '../..',
            '-G', vs_generator,
            f'-DCMAKE_BUILD_TYPE={config}',
            '-DBUILD_BURIED_SHARED_LIBS=ON',
            '-DBUILD_BURIED_STATIC_LIBS=ON'
        ]
        
        if vs_generator != "MinGW Makefiles":
            cmake_args.extend([f'-DCMAKE_GENERATOR_PLATFORM={platform}', '-T', 'v143'])
        
        # 添加可选参数
        if args and args.test:
            cmake_args.append('-DBUILD_BURIED_TEST=ON')
        if args and args.example:
            cmake_args.append('-DBUILD_BURIED_EXAMPLES=ON')
        
        print_info(f"CMake 配置命令: {' '.join(cmake_args)}")
        
        # 执行 CMake 配置
        result = subprocess.run(cmake_args, check=True)
        
        # 执行编译
        cpu_count = get_cpu_count()
        build_args = ['cmake', '--build', '.', '--config', config, '--parallel', str(cpu_count)]
        print_info(f"编译命令: {' '.join(build_args)}")
        
        result = subprocess.run(build_args, check=True)
        
        print_success(f"Windows 平台构建成功 ({platform}-{config})")
        return True
        
    except subprocess.CalledProcessError as e:
        print_error(f"构建失败: {e}")
        return False
    finally:
        os.chdir(original_dir)

def build_unix(config='Release', args=None):
    """Unix-like 系统构建 (macOS/Linux)"""
    platform_name = get_platform_name()
    print_info(f"开始 {platform_name.title()} 平台构建 ({config})")
    
    # 创建平台专属构建目录
    platform_dir = os.path.join(BUILD_DIR_PATH, f'{platform_name}-{config.lower()}')
    os.makedirs(platform_dir, exist_ok=True)
    
    original_dir = os.getcwd()
    os.chdir(platform_dir)
    
    try:
        # 生成 CMake 命令
        cmake_args = [
            'cmake', '../..',
            '-G', 'Unix Makefiles',
            f'-DCMAKE_BUILD_TYPE={config}',
            '-DBUILD_BURIED_SHARED_LIBS=ON',
            '-DBUILD_BURIED_STATIC_LIBS=ON'
        ]
        
        # 添加可选参数
        if args and args.test:
            cmake_args.append('-DBUILD_BURIED_TEST=ON')
        if args and args.example:
            cmake_args.append('-DBUILD_BURIED_EXAMPLES=ON')
        
        print_info(f"CMake 配置命令: {' '.join(cmake_args)}")
        
        # 执行 CMake 配置
        result = subprocess.run(cmake_args, check=True)
        
        # 执行编译
        cpu_count = get_cpu_count()
        build_args = ['make', f'-j{cpu_count}']
        print_info(f"编译命令: {' '.join(build_args)}")
        
        result = subprocess.run(build_args, check=True)
        
        print_success(f"{platform_name.title()} 平台构建成功 ({config})")
        return True
        
    except subprocess.CalledProcessError as e:
        print_error(f"构建失败: {e}")
        return False
    finally:
        os.chdir(original_dir)

def run_tests(config='Release'):
    """运行测试"""
    print_info("运行测试...")
    
    platform_name = get_platform_name()
    if IS_WINDOWS:
        platform_dir = os.path.join(BUILD_DIR_PATH, f'x64-{config}')
        test_executable = os.path.join(platform_dir, 'tests', config, 'buried_test.exe')
    else:
        platform_dir = os.path.join(BUILD_DIR_PATH, f'{platform_name}-{config.lower()}')
        test_executable = os.path.join(platform_dir, 'tests', 'buried_test')
    
    if os.path.exists(test_executable):
        try:
            result = subprocess.run([test_executable], check=True)
            print_success("测试通过")
            return True
        except subprocess.CalledProcessError:
            print_warning("测试失败")
            return False
    else:
        print_warning(f"未找到测试可执行文件: {test_executable}")
        return False

def show_build_results(config='Release'):
    """显示构建结果"""
    print_info("构建结果:")
    
    platform_name = get_platform_name()
    if IS_WINDOWS:
        platform_dir = os.path.join(BUILD_DIR_PATH, f'x64-{config}')
        patterns = ['*.exe', '*.dll', '*.lib']
    else:
        platform_dir = os.path.join(BUILD_DIR_PATH, f'{platform_name}-{config.lower()}')
        patterns = ['*.so', '*.dylib', '*.a']
    
    if os.path.exists(platform_dir):
        for root, dirs, files in os.walk(platform_dir):
            for file in files:
                for pattern in patterns:
                    if file.endswith(pattern.replace('*', '')):
                        rel_path = os.path.relpath(os.path.join(root, file), PROJECT_ROOT)
                        print(f"  {rel_path}")

def main():
    """主函数"""
    print_info("BuriedPoint 跨平台构建脚本")
    print_info(f"检测到平台: {get_platform_name().title()}")
    
    # 配置命令行参数解析器
    parser = argparse.ArgumentParser(description='BuriedPoint 跨平台构建脚本')
    parser.add_argument('--config', choices=['Debug', 'Release'], default='Release',
                        help='构建配置 (默认: Release)')
    parser.add_argument('--platform', default='x64',
                        help='Windows 平台架构 (默认: x64)')
    parser.add_argument('--test', action='store_true', default=False,
                        help='启用单元测试构建')
    parser.add_argument('--example', action='store_true', default=False,
                        help='启用示例程序构建')
    parser.add_argument('--run-tests', action='store_true', default=False,
                        help='构建后运行测试')
    parser.add_argument('--no-clean', action='store_true', default=False,
                        help='不清理构建目录')
    parser.add_argument('--check-deps', action='store_true', default=False,
                        help='只检查依赖，不进行构建')
    
    args = parser.parse_args()
    
    # 检查依赖
    if not check_dependencies():
        print_error("依赖检查失败")
        sys.exit(1)
    
    if args.check_deps:
        print_success("依赖检查完成，所有必需的工具都已安装")
        return
    
    # 清理构建目录
    if not args.no_clean:
        clear()
    
    # 创建构建目录
    os.makedirs(BUILD_DIR_PATH, exist_ok=True)
    
    # 执行构建
    build_success = False
    if IS_WINDOWS:
        build_success = build_windows(platform=args.platform, config=args.config, args=args)
    else:
        build_success = build_unix(config=args.config, args=args)
    
    if not build_success:
        print_error("构建失败")
        sys.exit(1)
    
    # 运行测试
    if args.run_tests and args.test:
        run_tests(args.config)
    
    # 显示构建结果
    show_build_results(args.config)
    
    print_success("构建完成!")

if __name__ == '__main__':
    main()