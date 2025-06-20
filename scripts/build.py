import os
import sys
import shutil
import argparse

SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

BUILD_DIR_PATH = SCRIPT_PATH + '/../build'

def clear():
    if os.path.exists(BUILD_DIR_PATH):
        shutil.rmtree(BUILD_DIR_PATH)

def build_windows(platform='x64', config='Release', args=None):
    # 创建平台专属构建目录（格式：build/x64-Debug）
    platform_dir = '%s/%s-%s' % (BUILD_DIR_PATH, platform, config)
    os.makedirs(platform_dir, exist_ok=True)

    os.chdir(platform_dir)  # 切换到构建目录

    # 生成CMake命令（使用VS2022编译器和v143工具集）
    build_cmd = 'cmake ../.. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=%s -DCMAKE_GENERATOR_PLATFORM=%s -T v143' % (
        config, platform)

    # 条件添加测试选项
    if args.test:
        build_cmd += ' -DBUILD_BURIED_TEST=ON'
    # 条件添加示例程序选项
    if args.example:
        build_cmd += ' -DBUILD_BURIED_EXAMPLES=ON'

    print("build cmd:" + build_cmd)
    # 执行CMake配置命令
    ret = os.system(build_cmd)
    if ret != 0:
        print('!!!!!!!!!!!!!!!!!!build fail')
        return False

    # 执行编译命令（使用8线程并行编译）
    build_cmd = 'cmake --build . --config %s --parallel 8' % (config)
    ret = os.system(build_cmd)
    if ret != 0:
        print('build fail!!!!!!!!!!!!!!!!!!!!')
        return False
    return True

def main():
    """主函数"""
    clear()  # 清理旧构建
    os.makedirs(BUILD_DIR_PATH, exist_ok=True)  # 创建新构建目录
    
    # 配置命令行参数解析器
    parser = argparse.ArgumentParser(description='build windows')
    parser.add_argument('--test', action='store_true', default=False,
                        help='启用单元测试构建')
    parser.add_argument('--example', action='store_true', default=False,
                        help='启用示例程序构建')
    args = parser.parse_args()

    # 执行构建流程（默认使用Debug配置）
    if not build_windows(platform='x64', config='Debug', args=args):
        exit(1)  # 构建失败时退出码为1

if __name__ == '__main__':
    main()
