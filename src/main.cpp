#include <windows.h>
#include <QApplication>
#include <QFontDatabase>

#include "mainwindow.h"

#include "utils.hpp"
#include "prog.hpp"

static void ensure_font()
{
    int font_id = QFontDatabase::addApplicationFont(prog::env::default_font_filename.c_str());
    if (font_id == -1)
    {
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

int main(int argc, char *argv[])
{
    system("chcp 65001 >nul");

    prog::env::config::load_prog_config();

    if (prog::env::config::misc_hide_console)
    {
        FreeConsole();
    }

    if constexpr (prog::env::debug::matcher_img_log || prog::env::debug::matcher_text_log)
    {
        std::filesystem::create_directories(prog::env::opencv_log_directory);
    }

    QApplication a(argc, argv);

    ensure_font();

    MainWindow w;

    w.show();

    return a.exec();
}