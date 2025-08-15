#!/usr/bin/env python3
"""
BuriedPoint 示例运行脚本

用于构建和运行 BuriedPoint 的各种示例程序
"""

import os
import sys
import subprocess
import argparse
import time
from pathlib import Path

def run_command(cmd, cwd=None, check=True):
    """运行命令并返回结果"""
    print(f"执行命令: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, cwd=cwd, check=check, 
                              capture_output=True, text=True)
        if result.stdout:
            print(result.stdout)
        return result
    except subprocess.CalledProcessError as e:
        print(f"命令执行失败: {e}")
        if e.stderr:
            print(f"错误信息: {e.stderr}")
        if check:
            sys.exit(1)
        return e

def build_project():
    """构建项目"""
    print("=== 构建 BuriedPoint 项目 ===")
    
    # 使用跨平台构建脚本
    build_script = Path("scripts/build_cross_platform.py")
    if not build_script.exists():
        print("错误: 找不到构建脚本")
        return False
    
    cmd = [sys.executable, str(build_script), "--example", "--test"]
    result = run_command(cmd, check=False)
    
    return result.returncode == 0

def find_executable(name, build_dir):
    """查找可执行文件"""
    # 在不同平台和配置下查找
    possible_paths = [
        build_dir / "examples" / name,
        build_dir / "examples" / f"{name}.exe",
        build_dir / "tests" / name,
        build_dir / "tests" / f"{name}.exe",
        # 平台特定的构建目录
        build_dir / "macos-release" / "examples" / name,
        build_dir / "macos-release" / "examples" / f"{name}.exe",
        build_dir / "macos-release" / "tests" / name,
        build_dir / "macos-release" / "tests" / f"{name}.exe",
        build_dir / "linux-release" / "examples" / name,
        build_dir / "linux-release" / "examples" / f"{name}.exe",
        build_dir / "linux-release" / "tests" / name,
        build_dir / "linux-release" / "tests" / f"{name}.exe",
        build_dir / "windows-release" / "examples" / f"{name}.exe",
        build_dir / "windows-release" / "tests" / f"{name}.exe",
    ]
    
    for path in possible_paths:
        if path.exists():
            return path
    
    return None

def run_example(example_name, build_dir):
    """运行指定的示例"""
    print(f"\n=== 运行示例: {example_name} ===")
    
    executable = find_executable(example_name, build_dir)
    if not executable:
        print(f"错误: 找不到可执行文件 {example_name}")
        return False
    
    print(f"找到可执行文件: {executable}")
    
    # 创建数据目录
    data_dir = Path("./data")
    data_dir.mkdir(exist_ok=True)
    
    try:
        result = subprocess.run([str(executable)], timeout=30, 
                              capture_output=True, text=True)
        
        print("--- 程序输出 ---")
        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print("错误输出:")
            print(result.stderr)
        
        if result.returncode == 0:
            print(f"✅ {example_name} 运行成功")
            return True
        else:
            print(f"❌ {example_name} 运行失败，返回码: {result.returncode}")
            return False
            
    except subprocess.TimeoutExpired:
        print(f"⏰ {example_name} 运行超时（30秒）")
        return False
    except Exception as e:
        print(f"❌ 运行 {example_name} 时发生错误: {e}")
        return False

def run_test(test_name, build_dir):
    """运行指定的测试"""
    print(f"\n=== 运行测试: {test_name} ===")
    
    executable = find_executable(test_name, build_dir)
    if not executable:
        print(f"错误: 找不到测试可执行文件 {test_name}")
        return False
    
    print(f"找到测试可执行文件: {executable}")
    
    try:
        result = subprocess.run([str(executable)], timeout=60,
                              capture_output=True, text=True)
        
        print("--- 测试输出 ---")
        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print("错误输出:")
            print(result.stderr)
        
        if result.returncode == 0:
            print(f"✅ {test_name} 测试通过")
            return True
        else:
            print(f"❌ {test_name} 测试失败，返回码: {result.returncode}")
            return False
            
    except subprocess.TimeoutExpired:
        print(f"⏰ {test_name} 测试超时（60秒）")
        return False
    except Exception as e:
        print(f"❌ 运行测试 {test_name} 时发生错误: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="BuriedPoint 示例和测试运行器")
    parser.add_argument("--build", action="store_true", help="构建项目")
    parser.add_argument("--example", choices=[
        "01_basic_usage", "02_multithreaded_usage", 
        "03_performance_test", "04_error_handling", "05_priority_demo", "all"
    ], help="运行指定示例")
    parser.add_argument("--test", choices=[
        "buried_test", "buried_comprehensive_test", 
        "buried_thread_safety_test", "all"
    ], help="运行指定测试")
    parser.add_argument("--build-dir", default="build", help="构建目录")
    
    args = parser.parse_args()
    
    if not any([args.build, args.example, args.test]):
        parser.print_help()
        return
    
    build_dir = Path(args.build_dir)
    
    # 构建项目
    if args.build:
        if not build_project():
            print("构建失败，退出")
            return
    
    # 查找实际的构建目录
    if not build_dir.exists():
        # 尝试查找平台特定的构建目录
        possible_dirs = list(Path(".").glob("build*"))
        if possible_dirs:
            build_dir = possible_dirs[0]
            print(f"使用构建目录: {build_dir}")
        else:
            print(f"错误: 找不到构建目录 {build_dir}")
            return
    
    success_count = 0
    total_count = 0
    
    # 运行示例
    if args.example:
        examples = [
            "01_basic_usage", "02_multithreaded_usage", 
            "03_performance_test", "04_error_handling", "05_priority_demo"
        ] if args.example == "all" else [args.example]
        
        for example in examples:
            total_count += 1
            if run_example(example, build_dir):
                success_count += 1
            time.sleep(1)  # 间隔1秒
    
    # 运行测试
    if args.test:
        tests = [
            "buried_test", "buried_comprehensive_test", 
            "buried_thread_safety_test"
        ] if args.test == "all" else [args.test]
        
        for test in tests:
            total_count += 1
            if run_test(test, build_dir):
                success_count += 1
            time.sleep(1)  # 间隔1秒
    
    # 总结
    if total_count > 0:
        print(f"\n=== 运行总结 ===")
        print(f"总计: {total_count}")
        print(f"成功: {success_count}")
        print(f"失败: {total_count - success_count}")
        print(f"成功率: {success_count/total_count*100:.1f}%")
        
        if success_count == total_count:
            print("🎉 所有测试都通过了！")
        else:
            print("⚠️ 部分测试失败，请检查输出")

if __name__ == "__main__":
    main()