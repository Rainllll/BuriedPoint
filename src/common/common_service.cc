#include "common/common_service.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <random>
#include <fstream>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#elif defined(__APPLE__)
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <unistd.h>
    #include <mach/mach.h>
    #include <mach/mach_time.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <libproc.h>
#elif defined(__linux__)
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fstream>
    #include <pwd.h>
#endif

#if !defined(_WIN32)
    #include <pwd.h>
#endif

#include "buried_config.h"

namespace buried {

CommonService::CommonService() { Init(); }

// 跨平台的设备ID存储和读取
class DeviceIdStorage {
public:
    static std::string GetStoragePath() {
#ifdef _WIN32
        // Windows: 使用注册表
        return "";  // 注册表不需要路径
#else
        // Unix-like系统: 使用配置文件
        const char* home = getenv("HOME");
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            home = pw ? pw->pw_dir : "/tmp";
        }
        return std::string(home) + "/.buried_config";
#endif
    }

    static void WriteDeviceId(const std::string& device_id) {
#ifdef _WIN32
        // Windows注册表实现
        HKEY h_key;
        LONG ret = ::RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Buried", 0, NULL,
                                   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                                   &h_key, NULL);
        if (ret != ERROR_SUCCESS) {
            return;
        }
        ret = ::RegSetValueExA(h_key, "device_id", 0, REG_SZ,
                             reinterpret_cast<const BYTE*>(device_id.c_str()),
                             device_id.size());
        ::RegCloseKey(h_key);
#else
        // Unix-like系统文件实现
        std::string config_path = GetStoragePath();
        std::ofstream file(config_path);
        if (file.is_open()) {
            file << "device_id=" << device_id << std::endl;
            file.close();
        }
#endif
    }

    static std::string ReadDeviceId() {
#ifdef _WIN32
        // Windows注册表实现
        HKEY h_key;
        LONG ret = ::RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Buried", 0,
                                 KEY_ALL_ACCESS, &h_key);
        if (ret != ERROR_SUCCESS) {
            return "";
        }
        char buf[1024] = {0};
        DWORD buf_size = sizeof(buf);
        ret = ::RegQueryValueExA(h_key, "device_id", NULL, NULL,
                               reinterpret_cast<BYTE*>(buf), &buf_size);
        ::RegCloseKey(h_key);
        if (ret != ERROR_SUCCESS) {
            return "";
        }
        return buf;
#else
        // Unix-like系统文件实现
        std::string config_path = GetStoragePath();
        std::ifstream file(config_path);
        if (!file.is_open()) {
            return "";
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("device_id=") == 0) {
                return line.substr(10);  // 跳过"device_id="
            }
        }
        return "";
#endif
    }
};

/**
 * @brief 获取设备唯一标识符（跨平台实现）
 */
static std::string GetDeviceId() {
    static std::string device_id = DeviceIdStorage::ReadDeviceId();
    if (device_id.empty()) {
        device_id = CommonService::GetRandomId();
        DeviceIdStorage::WriteDeviceId(device_id);
    }
    return device_id;
}

/**
 * @brief 获取生命周期 ID（跨平台实现）
 */
static std::string GetLifeCycleId() {
    static std::string life_cycle_id = CommonService::GetRandomId();
    return life_cycle_id;
}

/**
 * @brief 获取当前操作系统的版本信息（跨平台实现）
 */
static std::string GetSystemVersion() {
#ifdef _WIN32
    // Windows实现
    OSVERSIONINFOEXA os_version_info;
    ZeroMemory(&os_version_info, sizeof(OSVERSIONINFOEXA));
    os_version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
    ::GetVersionExA(reinterpret_cast<OSVERSIONINFOA*>(&os_version_info));
    
    return std::to_string(os_version_info.dwMajorVersion) + "." +
           std::to_string(os_version_info.dwMinorVersion) + "." +
           std::to_string(os_version_info.dwBuildNumber);
#elif defined(__APPLE__)
    // macOS实现
    char str[256];
    size_t size = sizeof(str);
    if (sysctlbyname("kern.osrelease", str, &size, NULL, 0) == 0) {
        return std::string(str);
    }
    return "Unknown";
#elif defined(__linux__)
    // Linux实现
    std::ifstream file("/proc/version");
    if (file.is_open()) {
        std::string line;
        std::getline(file, line);
        file.close();
        
        // 提取版本号
        size_t start = line.find("version ");
        if (start != std::string::npos) {
            start += 8;  // 跳过"version "
            size_t end = line.find(" ", start);
            if (end != std::string::npos) {
                return line.substr(start, end - start);
            }
        }
    }
    return "Unknown";
#else
    return "Unknown";
#endif
}

/**
 * @brief 获取当前设备的名称（跨平台实现）
 */
static std::string GetDeviceName() {
#ifdef _WIN32
    // Windows实现
    char buf[1024] = {0};
    DWORD buf_size = sizeof(buf);
    ::GetComputerNameA(buf, &buf_size);
    return std::string(buf);
#else
    // Unix-like系统实现
    char hostname[1024];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return std::string(hostname);
    }
    return "Unknown";
#endif
}

/**
 * @brief 获取当前进程的创建时间（跨平台实现）
 */
std::string CommonService::GetProcessTime() {
#ifdef _WIN32
    // Windows实现
    DWORD pid = ::GetCurrentProcessId();
    HANDLE h_process = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (h_process == NULL) {
        return "";
    }

    FILETIME create_time, exit_time, kernel_time, user_time;
    BOOL ret = ::GetProcessTimes(h_process, &create_time, &exit_time, &kernel_time, &user_time);
    ::CloseHandle(h_process);
    if (ret == 0) {
        return "";
    }

    FILETIME create_local_time;
    ::FileTimeToLocalFileTime(&create_time, &create_local_time);

    SYSTEMTIME create_sys_time;
    ::FileTimeToSystemTime(&create_local_time, &create_sys_time);

    char buf[128] = {0};
    sprintf_s(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 
              create_sys_time.wYear, create_sys_time.wMonth, create_sys_time.wDay,
              create_sys_time.wHour, create_sys_time.wMinute, create_sys_time.wSecond,
              create_sys_time.wMilliseconds);
    return buf;
#elif defined(__APPLE__)
    // macOS实现
    pid_t pid = getpid();
    struct proc_bsdinfo proc;
    if (proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &proc, sizeof(proc)) > 0) {
        time_t start_time = proc.pbi_start_tvsec;
        struct tm* tm_info = localtime(&start_time);
        char buf[128];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S.000", tm_info);
        return std::string(buf);
    }
    return "";
#elif defined(__linux__)
    // Linux实现
    pid_t pid = getpid();
    std::string stat_file = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(stat_file);
    if (file.is_open()) {
        std::string line;
        std::getline(file, line);
        file.close();
        
        // 解析stat文件获取启动时间
        std::istringstream iss(line);
        std::string token;
        for (int i = 0; i < 22; ++i) {  // starttime是第22个字段
            iss >> token;
        }
        
        if (!token.empty()) {
            // 转换为实际时间
            long long starttime = std::stoll(token);
            long long boot_time = 0;
            
            // 获取系统启动时间
            std::ifstream stat_file("/proc/stat");
            std::string stat_line;
            while (std::getline(stat_file, stat_line)) {
                if (stat_line.find("btime ") == 0) {
                    boot_time = std::stoll(stat_line.substr(6));
                    break;
                }
            }
            
            if (boot_time > 0) {
                long long process_start = boot_time + starttime / sysconf(_SC_CLK_TCK);
                time_t start_time = static_cast<time_t>(process_start);
                struct tm* tm_info = localtime(&start_time);
                char buf[128];
                strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S.000", tm_info);
                return std::string(buf);
            }
        }
    }
    return "";
#else
    return "";
#endif
}

/**
 * @brief 获取当前日期和时间的字符串表示（跨平台实现）
 */
std::string CommonService::GetNowDate() {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return std::ctime(&t);
}

/**
 * @brief 生成一个指定长度的随机 ID 字符串（跨平台实现）
 */
std::string CommonService::GetRandomId() {
    static constexpr size_t len = 32;
    static constexpr auto chars =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static std::mt19937_64 rng{std::random_device{}()};
    static std::uniform_int_distribution<size_t> dist{0, 60};
    
    std::string result;
    result.reserve(len);
    std::generate_n(std::back_inserter(result), len,
                    [&]() { return chars[dist(rng)]; });
    return result;
}

void CommonService::Init() {
    system_version = GetSystemVersion();
    device_name = GetDeviceName();
    device_id = GetDeviceId();
    buried_version = PROJECT_VER;
    lifecycle_id = GetLifeCycleId();
}

}  // namespace buried