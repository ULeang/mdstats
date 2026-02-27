#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <windows.h>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iostream>

enum class ErrorType {
  OK = 0,
  ErrDeviceContext,
  ErrFindWindow,
  ErrGetWindowRect,
  ErrDeterRes,
  ErrPrintWindow,
  ErrEnsureCSV,
  ErrCheckResources,
  ErrStopRequested,
  ErrMatcherCannotGetImage,
  ErrOthers,
};

enum class MatcherFailT {
  OK = 0,
  CannotGetImage,
  TryNExceed,
  Other,
};

// log

// only logs with level bigger than or equal to current threshold will be logged
enum LogLevel {
  NEVER = 0,
  SELDOM,
  USUALLY,
  ALWAYS,
};

namespace prog {
namespace global {
inline LogLevel log_level = LogLevel::USUALLY;
}
}  // namespace prog

inline bool log(const std::string &l, LogLevel log_lv = LogLevel::USUALLY) {
  if (log_lv >= prog::global::log_level) {
    std::clog << l << std::flush;
    return true;
  }
  return false;
}
inline bool logln(const std::string &l, LogLevel log_lv = LogLevel::USUALLY) {
  if (log_lv >= prog::global::log_level) {
    std::clog << l << std::endl;
    return true;
  }
  return false;
}
inline std::tm *get_local_time() {
  auto        now        = std::chrono::system_clock::now();
  std::time_t now_time   = std::chrono::system_clock::to_time_t(now);
  std::tm    *local_time = std::localtime(&now_time);
  return local_time;
}

inline bool check_resources(const std::vector<std::filesystem::path> &files) {
  for (const auto &f : files) {
    if (!std::filesystem::exists(f)) {
      return false;
    }
  }
  return true;
}

// clipboard
inline void CopyToClipboard(const char *text) {
  // 打开剪贴板
  if (!OpenClipboard(nullptr)) return;

  // 清空剪贴板
  EmptyClipboard();

  // 为文本分配全局内存
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);

  HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size_needed * sizeof(wchar_t));
  if (!hGlobal) {
    CloseClipboard();
    return;
  }

  // 将文本复制到全局内存
  wchar_t *pGlobal = static_cast<wchar_t *>(GlobalLock(hGlobal));
  MultiByteToWideChar(CP_UTF8, 0, text, -1, pGlobal, size_needed);
  GlobalUnlock(hGlobal);

  // 设置剪贴板数据
  SetClipboardData(CF_UNICODETEXT, hGlobal);

  // 关闭剪贴板
  CloseClipboard();
}
inline void CopyToClipboard(const wchar_t *text) {
  // 打开剪贴板
  if (!OpenClipboard(nullptr)) return;

  // 清空剪贴板
  EmptyClipboard();

  // 为文本分配全局内存
  size_t  size    = wcslen(text) + 1;
  HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size * sizeof(wchar_t));
  if (!hGlobal) {
    CloseClipboard();
    return;
  }

  // 将文本复制到全局内存
  wchar_t *pGlobal = static_cast<wchar_t *>(GlobalLock(hGlobal));
  wcsncpy(pGlobal, text, size);
  GlobalUnlock(hGlobal);

  // 设置剪贴板数据
  SetClipboardData(CF_UNICODETEXT, hGlobal);

  // 关闭剪贴板
  CloseClipboard();
}
inline WINBOOL create_process(const char *cmdline, DWORD creation_flag = CREATE_NO_WINDOW) {
  STARTUPINFOA        sinfo;
  PROCESS_INFORMATION pinfo;
  ZeroMemory(&sinfo, sizeof(sinfo));
  sinfo.cb = sizeof(sinfo);
  ZeroMemory(&pinfo, sizeof(pinfo));
  return CreateProcessA(NULL, const_cast<char *>(cmdline), NULL, NULL, FALSE, creation_flag, NULL,
                        NULL, &sinfo, &pinfo);
}

#endif
