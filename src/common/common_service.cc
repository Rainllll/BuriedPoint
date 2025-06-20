#include "common/common_service.h"

#include <windows.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <random>

#include "buried_config.h"

namespace buried {

CommonService::CommonService() { Init(); }

static void WriteRegister(const std::string& key, const std::string& value) {
  HKEY h_key;
  LONG ret = ::RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Buried", 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                               &h_key, NULL);
  if (ret != ERROR_SUCCESS) {
    return;
  }
  ret = ::RegSetValueExA(h_key, key.c_str(), 0, REG_SZ,
                         reinterpret_cast<const BYTE*>(value.c_str()),
                         value.size());
  if (ret != ERROR_SUCCESS) {
    return;
  }
  ::RegCloseKey(h_key);
}

/**
 * @brief 从注册表中读取指定键的值。
 * 
 * 该函数尝试打开 HKEY_CURRENT_USER\Software\Buried 注册表项，
 * 并从中读取指定键的值。若操作过程中出现错误，将返回空字符串。
 * 
 * @param key 要读取的注册表键名。
 * @return 包含注册表键值的字符串，若读取失败则返回空字符串。
 */
static std::string ReadRegister(const std::string& key) {
  // 定义注册表项句柄，用于后续操作注册表
  HKEY h_key;
  // 尝试打开 HKEY_CURRENT_USER\Software\Buried 注册表项，使用 KEY_ALL_ACCESS 权限
  LONG ret = ::RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Buried", 0,
                             KEY_ALL_ACCESS, &h_key);
  // 检查打开注册表项是否成功
  if (ret != ERROR_SUCCESS) {
    // 若失败，返回空字符串
    return "";
  }
  // 定义字符数组，用于存储从注册表中读取的值，初始大小为 1024 字节，初始化为全 0
  char buf[1024] = {0};
  // 定义 DWORD 类型变量，存储字符数组的大小，用于指定读取数据的缓冲区大小
  DWORD buf_size = sizeof(buf);
  // 尝试从打开的注册表项中读取指定键的值，将结果存储在 buf 中
  ret = ::RegQueryValueExA(h_key, key.c_str(), NULL, NULL,
                           reinterpret_cast<BYTE*>(buf), &buf_size);
  // 关闭注册表项句柄，释放系统资源
  ::RegCloseKey(h_key);
  // 检查读取注册表值是否成功
  if (ret != ERROR_SUCCESS) {
    // 若失败，返回空字符串
    return "";
  }
  // 将字符数组转换为 std::string 类型并返回
  return buf;
}

/**
 * @brief 获取设备唯一标识符。
 * 
 * 该函数会尝试从注册表中读取设备 ID。若注册表中不存在设备 ID，
 * 则生成一个新的随机设备 ID 并将其写入注册表。在整个程序运行期间，
 * 无论调用多少次该函数，都只会生成一次新的设备 ID（如果需要生成的话），
 * 后续调用将返回首次获取或生成的设备 ID。
 * 
 * @return 包含设备唯一标识符的字符串。
 */
static std::string GetDeviceId() {
  // 定义静态常量字符串，作为注册表中存储设备 ID 的键名
  static constexpr auto kDeviceIdKey = "device_id";
  // 定义静态字符串变量，用于存储设备 ID。程序启动时尝试从注册表中读取该值
  static std::string device_id = ReadRegister(kDeviceIdKey);
  // 检查从注册表中读取的设备 ID 是否为空
  if (device_id.empty()) {
    // 若为空，则调用 CommonService 类的 GetRandomId 方法生成一个新的随机设备 ID
    device_id = CommonService::GetRandomId();
    // 将新生成的设备 ID 写入注册表，以便后续使用
    WriteRegister(kDeviceIdKey, device_id);
  }
  // 返回存储的设备 ID
  return device_id;
}

/**
 * @brief 获取生命周期 ID。
 * 
 * 该函数会生成一个唯一的生命周期 ID，且在整个程序运行期间，
 * 无论调用多少次该函数，都只会生成一次生命周期 ID，后续调用将返回首次生成的 ID。
 * 
 * @return 包含唯一生命周期 ID 的字符串。
 */
static std::string GetLifeCycleId() {
  // 静态变量，确保生命周期 ID 只生成一次。调用 CommonService 类的 GetRandomId 方法生成随机 ID
  static std::string life_cycle_id = CommonService::GetRandomId();
  // 返回生成的生命周期 ID
  return life_cycle_id;
}

/**
 * @brief 获取当前操作系统的版本信息。
 * 
 * 该函数通过 Windows API 获取操作系统的版本信息，
 * 并将主要版本号、次要版本号和构建号组合成一个字符串返回。
 * 
 * @return 包含操作系统版本信息的字符串，格式为 "主要版本号.次要版本号.构建号"。
 */
static std::string GetSystemVersion() {
  // 定义 OSVERSIONINFOEXA 结构体变量，用于存储操作系统版本信息
  OSVERSIONINFOEXA os_version_info;
  // 将 os_version_info 结构体的内存区域清零，确保所有成员初始化为 0
  ZeroMemory(&os_version_info, sizeof(OSVERSIONINFOEXA));
  // 设置 os_version_info 结构体的大小，让 Windows API 知道要处理的结构体大小
  os_version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
  // 调用 Windows API 函数 GetVersionExA 获取操作系统版本信息
  // 由于 GetVersionExA 函数接受 OSVERSIONINFOA 类型指针，需要进行类型转换
  ::GetVersionExA(reinterpret_cast<OSVERSIONINFOA*>(&os_version_info));
  // 将获取到的主要版本号、次要版本号和构建号组合成一个字符串
  std::string system_version =
      std::to_string(os_version_info.dwMajorVersion) + "." +
      std::to_string(os_version_info.dwMinorVersion) + "." +
      std::to_string(os_version_info.dwBuildNumber);
  // 返回包含操作系统版本信息的字符串
  return system_version;
}

/**
 * @brief 获取当前设备的名称。
 * 
 * 该函数调用 Windows API 函数 GetComputerNameA 来获取当前计算机的名称，
 * 并将其存储在一个字符串中返回。
 * 
 * @return 包含当前设备名称的字符串，若获取失败可能为空字符串。
 */
static std::string GetDeviceName() {
  // 定义一个字符数组，用于存储获取到的设备名称，初始化为全 0
  char buf[1024] = {0};
  // 定义一个 DWORD 类型的变量，初始化为字符数组的大小，用于指定缓冲区的大小
  DWORD buf_size = sizeof(buf);
  // 调用 Windows API 函数 GetComputerNameA 获取当前计算机的名称
  // 该函数会将名称存储在 buf 中，并更新 buf_size 为实际使用的字符数
  ::GetComputerNameA(buf, &buf_size);
  // 将字符数组转换为 std::string 类型
  std::string device_name = buf;
  // 返回包含设备名称的字符串
  return device_name;
}

/**
 * @brief 获取当前进程的创建时间并格式化为字符串。
 * 
 * 该函数首先获取当前进程的 ID，然后打开该进程以查询其信息。
 * 接着获取进程的创建时间，并将其转换为本地时间和系统时间，
 * 最后将系统时间格式化为 "YYYY-MM-DD HH:MM:SS.mmm" 的字符串返回。
 * 
 * @return 包含当前进程创建时间的格式化字符串，若操作失败则返回空字符串。
 */
std::string CommonService::GetProcessTime() {
  // 获取当前进程的进程 ID
  DWORD pid = ::GetCurrentProcessId();
  // 打开当前进程，获取用于查询进程信息和读取虚拟内存的句柄
  HANDLE h_process =
      ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  // 若打开进程失败，返回空字符串
  if (h_process == NULL) {
    return "";
  }

  // 定义 FILETIME 结构体变量，用于存储进程的创建时间、退出时间、内核时间和用户时间
  FILETIME create_time;
  FILETIME exit_time;
  FILETIME kernel_time;
  FILETIME user_time;
  // 获取当前进程的时间信息
  BOOL ret = ::GetProcessTimes(h_process, &create_time, &exit_time,
                               &kernel_time, &user_time);
  // 关闭进程句柄，释放系统资源
  ::CloseHandle(h_process);
  // 若获取时间信息失败，返回空字符串
  if (ret == 0) {
    return "";
  }

  // 定义 FILETIME 结构体变量，用于存储转换后的本地创建时间
  FILETIME create_local_time;
  // 将进程的创建时间转换为本地时间
  ::FileTimeToLocalFileTime(&create_time, &create_local_time);

  // 定义 SYSTEMTIME 结构体变量，用于存储转换后的系统时间
  SYSTEMTIME create_sys_time;
  // 将本地创建时间转换为系统时间
  ::FileTimeToSystemTime(&create_local_time, &create_sys_time);

  // 定义字符数组，用于存储格式化后的时间字符串
  char buf[128] = {0};
  // 按照 "YYYY-MM-DD HH:MM:SS.mmm" 的格式将系统时间格式化到字符数组中
  // year month day hour minute second millisecond
  sprintf_s(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", create_sys_time.wYear,
            create_sys_time.wMonth, create_sys_time.wDay, create_sys_time.wHour,
            create_sys_time.wMinute, create_sys_time.wSecond,
            create_sys_time.wMilliseconds);
  // 返回格式化后的时间字符串
  return buf;
}

/**
 * @brief 获取当前日期和时间的字符串表示。
 * 
 * 该函数通过标准库的 chrono 模块获取当前系统时间，
 * 再将其转换为 C 风格的时间表示，最后返回该时间的字符串形式。
 * 
 * @return 包含当前日期和时间的字符串，格式遵循 ctime 函数的默认格式。
 */
std::string CommonService::GetNowDate() {
  // 获取当前系统时间点，并将其转换为 C 风格的 time_t 类型
  auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  // 将 time_t 类型的时间转换为字符串形式并返回
  return std::ctime(&t);
}

/**
 * @brief 生成一个指定长度的随机 ID 字符串。
 * 
 * 该函数使用 Mersenne Twister 算法生成随机数，从预定义的字符集中随机选取字符，
 * 最终拼接成指定长度的随机 ID 字符串。
 * 
 * @return 包含随机 ID 的字符串，长度为 32 个字符。
 */
std::string CommonService::GetRandomId() {
  // 定义随机 ID 的长度为 32 个字符
  static constexpr size_t len = 32;
  // 定义随机 ID 可用的字符集，包含数字、大写字母和小写字母
  static constexpr auto chars =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  // 使用 std::random_device 生成种子，初始化 64 位 Mersenne Twister 随机数生成器
  static std::mt19937_64 rng{std::random_device{}()};
  // 定义一个均匀分布，范围从 0 到 60，用于从字符集中随机选取字符
  static std::uniform_int_distribution<size_t> dist{0, 60};
  // 用于存储最终生成的随机 ID 字符串
  std::string result;
  // 预先分配足够的内存空间，避免后续添加字符时多次重新分配内存
  result.reserve(len);
  // 调用 std::generate_n 函数，重复调用 lambda 函数 32 次，每次从字符集中随机选取一个字符添加到 result 中
  std::generate_n(std::back_inserter(result), len,
                  [&]() { return chars[dist(rng)]; });
  // 返回生成的随机 ID 字符串
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
