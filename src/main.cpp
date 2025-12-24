#include <windows.h>
#include <QApplication>
#include <QFontDatabase>
#include <filesystem>

#include "mainwindow.h"

#include "prog.hpp"
#include "prog_config.hpp"
#include "utils.hpp"

static void ensure_font() {
  int font_id = QFontDatabase::addApplicationFont(prog::env::default_font_filename.c_str());
  if (font_id == -1) {
    logln(std::format("fatal: default font file '{}' read error, quit",
                      prog::env::default_font_filename.c_str()),
          LogLevel::ALWAYS);
    exit(-1);
  }
  auto fontfamily = QFontDatabase::applicationFontFamilies(font_id).at(0);
  logln(std::format("using default font file '{}'\nfont family : '{}'",
                    prog::env::default_font_filename, fontfamily.toStdString()));
  prog::global::font = {fontfamily, 14, 700, false};
}
static void ensure_clip_pic() {
  const std::filesystem::path clip_pic_path{prog::env::clip_pic_path};
  if (!std::filesystem::exists(clip_pic_path)) {
    logln(std::format("clip pic path '{}' inexists", prog::env::clip_pic_path));
    return;
  }

  for (const auto &entry : std::filesystem::directory_iterator{clip_pic_path}) {
    if (entry.is_directory()) continue;
    prog::global::clip_pic_name_list.push_back(entry.path().filename().string());
  }
  logln(std::format("there are {} pic(s) in {}", prog::global::clip_pic_name_list.size(),
                    prog::env::clip_pic_path));
}

class ENSURE_SINGLE_INSTANCE {
  HANDLE hMutex = NULL;

public:
  ENSURE_SINGLE_INSTANCE() {
    hMutex = CreateMutex(NULL, FALSE, TEXT("mdstats.exe"));
    if (hMutex == NULL) return;
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
      CloseHandle(hMutex);
      hMutex = NULL;
    }
  }
  ~ENSURE_SINGLE_INSTANCE() {
    if (hMutex != NULL) {
      CloseHandle(hMutex);
    }
  }
  bool ok() { return hMutex != NULL; }
};

int main(int argc, char *argv[]) {
  ENSURE_SINGLE_INSTANCE _si;
  if (!_si.ok()) {
    logln(std::format("mdstats is already running!"), LogLevel::ALWAYS);
    return -2;
  }

  prog::env::config::load_prog_config();

  MyModule::StatsTable::RAII _module_statstable_raii;

  if constexpr (prog::env::debug::matcher_img_log || prog::env::debug::matcher_text_log) {
    std::filesystem::create_directories(prog::env::opencv_log_directory);
  }

  QApplication a(argc, argv);

  ensure_font();
  ensure_clip_pic();

  MainWindow w;

  w.show();

  return a.exec();
}
