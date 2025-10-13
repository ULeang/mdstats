#include <windows.h>
#include <QApplication>
#include <QFontDatabase>

#include <rapidcsv.h>

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
    auto fontname = QFontDatabase::applicationFontFamilies(font_id).at(0);
    logln(std::format("using default font file '{}'\nfont family : '{}'",
                      prog::env::default_font_filename, fontname.toStdString()));
    prog::global::qt_font_id = font_id;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::filesystem::create_directories(prog::env::opencv_log_directory);

    ensure_font();

    MainWindow w;

    w.setWindowTitle("MD stats");
    w.setGeometry({800, 400, 900, 400});
    w.show();

    return a.exec();
}