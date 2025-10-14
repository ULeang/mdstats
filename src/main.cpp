#include <windows.h>
#include <QApplication>
#include <QFontDatabase>

#include <rapidcsv.h>

// #include "mainwindow.h"

#include "utils.hpp"
#include "prog.hpp"
#include "databasemodel.hpp"
#include "matcher.hpp"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableView>

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
    prog::env::config::load_prog_config();

    QApplication a(argc, argv);
    if (prog::env::debug::matcher_img_log | prog::env::debug::matcher_text_log)
    {
        std::filesystem::create_directories(prog::env::opencv_log_directory);
    }

    ensure_font();

    // MainWindow w;

    // w.setWindowTitle("MD stats");
    // w.setGeometry({prog::env::config::prog_window_init_x_y_width_height[0],
    //                prog::env::config::prog_window_init_x_y_width_height[1],
    //                prog::env::config::prog_window_init_x_y_width_height[2],
    //                prog::env::config::prog_window_init_x_y_width_height[3]});
    // w.show();

    DataBase_ *database = new DataBase_;
    Stats_ *stats = database->get_stats();

    QTableView *table_record = new QTableView;
    QTableView *table_stats = new QTableView;

    table_stats->setSpan(0, 1, 1, 2);
    table_stats->setSpan(3, 1, 1, 2);
    table_stats->setSpan(4, 1, 1, 2);
    table_stats->setSpan(5, 1, 1, 2);

    database->load_csv("resource\\csv\\data.csv");
    table_stats->horizontalHeader()->setVisible(false);
    table_stats->verticalHeader()->setVisible(false);

    table_record->setModel(database);
    table_stats->setModel(stats);

    QThread *matcher_thread = new QThread;
    Matcher_ *matcher = new Matcher_;
    matcher->moveToThread(matcher_thread);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(table_record);
    layout->addWidget(table_stats);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    widget->show();

    return a.exec();
}