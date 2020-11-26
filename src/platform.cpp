#include "platform.hpp"
#include "utility.hpp"

#if defined(_WIN32)
#define NP_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#define NP_PLATFORM_LINUX 1
#elif defined(__APPLE__)
#define NP_PLATFORM_MAC 1
#else
#error Unsupported platform
#endif

#if NP_PLATFORM_WINDOWS
#include <winsock2.h>
#include <Windows.h>
#elif NP_PLATFORM_LINUX
#include <sys/syscall.h>
#include <errno.h>
#include <unistd.h>
#elif NP_PLATFORM_MAC
#include <mach-o/dyld.h>
#include <pthread.h>
#include <unistd.h>
#endif

#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

namespace {
#if NP_PLATFORM_WINDOWS
  std::string utf16ToUtf8(std::wstring_view str) {
    // Worst case, each 16 bit code unit becomes a 3-byte UTF-8 sequence
    const auto max_len = str.size() * 3;
    std::string result(max_len, '\0');

    const auto result_len = WideCharToMultiByte(CP_UTF8,
      0,
      str.data(),
      static_cast<int>(str.size()),
      result.data(),
      static_cast<int>(result.size()),
      nullptr,
      nullptr);
    result.resize(result_len);
    return result;
  }

  struct ScopedWinSockInit {
    ScopedWinSockInit() { (void)WSAStartup(MAKEWORD(2, 2), &data); }
    ~ScopedWinSockInit() { WSACleanup(); }

  private:
    WSADATA data;
  } winsock_init;

#endif
} // namespace

namespace np::log {
  namespace internal {
    uint64_t processId() {
#if NP_PLATFORM_WINDOWS
      return GetCurrentProcessId();
#else
      return static_cast<uint64_t>((getpid()));
#endif
    }

    uint64_t threadId() {
#if NP_PLATFORM_WINDOWS
      return GetCurrentThreadId();
#elif NP_PLATFORM_LINUX
      return static_cast<uint64_t>(syscall(SYS_gettid));
#elif NP_PLATFORM_MAC
      uint64_t tid;
      pthread_threadid_np(pthread_self(), &tid);
      return tid;
#endif
    }

    std::string hostname() {
      char buf[256];
      if (gethostname(buf, 256) == 0) { return std::string(buf); }
      return "<error>";
    }

    std::string executableName() {
#if NP_PLATFORM_WINDOWS
      std::vector<wchar_t> buf(512);
      for (;;) {
        const auto result = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
        if (result != buf.size()) { return utf16ToUtf8(std::wstring_view(buf.data(), result)); }
        buf.resize(buf.size() * 2);
      }
#elif NP_PLATFORM_LINUX
      std::vector<char> buf(512);
      for (;;) {
        const auto ret = readlink("/proc/self/exe", buf.data(), buf.size());
        if (ret != -1) { return std::string(buf.data(), to_size_t_checked(ret)); }
        if (ret == -1 && errno != ENAMETOOLONG) { return "<error>"; }
        buf.resize(buf.size() * 2);
      }
#elif NP_PLATFORM_MAC
      std::vector<char> buf(512);
      uint32_t len = 0;
      _NSGetExecutablePath(buf.data(), &len);
      buf.resize(len);
      if (_NSGetExecutablePath(buf.data(), &len) != 0) { return "<error>"; }
      return std::string(buf.data(), buf.size() - 1);
#endif
    }

    struct CachedFields {
      CachedFields()
        : hostname(hostnameFromFqdn(internal::hostname()))
        , executable(filenameFromPath(internal::executableName())) {}

      std::string hostname;
      std::string executable;
    };

    std::shared_mutex mtx;
    std::unique_ptr<CachedFields> cached;
  } // namespace internal

  namespace platform {
    uint64_t processId() { return internal::processId(); }
    uint64_t threadId() { return internal::threadId(); }
    std::string hostname() {
      {
        std::shared_lock<std::shared_mutex> lock(internal::mtx);
        if (internal::cached) { return internal::cached->hostname; }
      }
      std::lock_guard<std::shared_mutex> lock(internal::mtx);
      internal::cached = std::make_unique<internal::CachedFields>();
      return internal::cached->hostname;
    }
    std::string executableName() {
      {
        std::shared_lock<std::shared_mutex> lock(internal::mtx);
        if (internal::cached) { return internal::cached->executable; }
      }
      std::lock_guard<std::shared_mutex> lock(internal::mtx);
      internal::cached = std::make_unique<internal::CachedFields>();
      return internal::cached->executable;
    }
  } // namespace platform
} // namespace np::log
