#include "prog.hpp"
#include "mainwindow.h"
#include "delegate.hpp"

#include <QPushButton>
#include <QHeaderView>
#include <QFontDatabase>
#include <QColor>
#include <QFileDialog>
#include <QMessageBox>
#include <QMovie>
#include <windows.h>
#include <random>

using std::format;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    construct_all();
    setGeometry({prog::env::config::misc_prog_window_init_geometry_x,
                 prog::env::config::misc_prog_window_init_geometry_y,
                 prog::env::config::misc_prog_window_init_geometry_width,
                 prog::env::config::misc_prog_window_init_geometry_height});
}

MainWindow::~MainWindow()
{
    destruct_all();
}
void MainWindow::reset()
{
    destruct_all();
    construct_all();
}
void MainWindow::construct_all()
{
    stats_tbl = new QTableView;
    record_tbl = new QTableView;

    startBtn = new QPushButton;
    stopBtn = new QPushButton;
    cptoclpbdBtn = new QPushButton;
    reloadBtn = new QPushButton;
    openCSVBtn = new QPushButton;
    saveAsBtn = new QPushButton;
    clearrecordBtn = new QPushButton;
    manual_0Btn = new QPushButton;
    manual_1Btn = new QPushButton;
    open_config_Btn = new QPushButton;
    reload_config_Btn = new QPushButton;

    coin_lbl = new QLabel;
    st_nd_lbl = new QLabel;
    result_lbl = new QLabel;
    time_lbl = new QLabel;
    corrupted_csv_lbl = new QLabel;

    h_layout1 = new QHBoxLayout;
    v_layout1 = new QVBoxLayout;
    g_layout1 = new QGridLayout;
    g_layout2 = new QGridLayout;
    g_layout3 = new QGridLayout;
    widget = new QWidget;

    data = new DataBase;
    matcher_thread = new QThread;
    matcher = new MatcherWorker;

    startBtn->setText("启动");
    stopBtn->setText("停止");
    cptoclpbdBtn->setText("复制到剪贴板");
    reloadBtn->setText("重新加载数据");
    openCSVBtn->setText("打开data.csv目录");
    saveAsBtn->setText("把csv另存为");
    clearrecordBtn->setText("清除所有记录");
    manual_reset();
    open_config_Btn->setText("打开配置文件");
    reload_config_Btn->setText("重新加载配置文件");

    matcher->moveToThread(matcher_thread);
    matcher_thread->start();

    record_tbl->setModel(data);
    stats_tbl->setModel(data->get_stats());

    stats_tbl->horizontalHeader()->setVisible(false);
    stats_tbl->verticalHeader()->setVisible(false);

    // set grid
    stats_tbl->setShowGrid(false);

    // set span
    stats_tbl->setSpan(0, 1, 1, 2);
    stats_tbl->setSpan(3, 1, 1, 2);
    stats_tbl->setSpan(4, 1, 1, 2);
    stats_tbl->setSpan(5, 1, 1, 2);

    // set delegate
    record_tbl->setItemDelegateForColumn(0, new MyDelegate({"赢币", "输币"}, false));
    record_tbl->setItemDelegateForColumn(1, new MyDelegate({"先攻", "后攻"}, false));
    record_tbl->setItemDelegateForColumn(2, new MyDelegate({"胜利", "失败", "平局"}));

    set_qss();
    start_stop_switch(true);
    disable_manual_btn(true);

    corrupted_csv_lbl->setText(
        R"(警告:csv文件已损坏,你正处于无csv模式,
此时的一切数据都不会被保存
请点击下方'打开data.csv目录'尝试修复,
或者直接删除该文件,然后点击'重新加载数据'
提示:目前仅支持utf8编码和utf16编码)");
    corrupted_csv_lbl->setVisible(false);

    // set font
    // auto font = QFont("FiraCode Nerd Font Mono", 16);
    setFont(prog::global::font);

    setWindowTitle("MD stats");
    ensure_config();

    // set layout
    g_layout1->addWidget(manual_0Btn, 0, 0, 1, 3);
    g_layout1->addWidget(manual_1Btn, 0, 3, 1, 3);
    g_layout1->addWidget(coin_lbl, 1, 0, 1, 2);
    g_layout1->addWidget(st_nd_lbl, 1, 2, 1, 2);
    g_layout1->addWidget(result_lbl, 1, 4, 1, 2);
    g_layout1->addWidget(time_lbl, 2, 0, 1, 6);

    g_layout2->addWidget(cptoclpbdBtn, 0, 0, 1, 6);
    g_layout2->addWidget(startBtn, 1, 0, 1, 3);
    g_layout2->addWidget(stopBtn, 1, 3, 1, 3);

    v_layout1->addWidget(stats_tbl, 0);
    v_layout1->addLayout(g_layout1, 0);
    v_layout1->addWidget(new QWidget, 1);
    v_layout1->addLayout(g_layout2, 0);

    g_layout3->addWidget(record_tbl, 0, 0, 1, 2);
    g_layout3->addWidget(corrupted_csv_lbl, 0, 0, 1, 2);
    g_layout3->addWidget(openCSVBtn, 1, 0);
    g_layout3->addWidget(reloadBtn, 1, 1);
    g_layout3->addWidget(saveAsBtn, 2, 0);
    g_layout3->addWidget(clearrecordBtn, 2, 1);
    g_layout3->addWidget(open_config_Btn, 3, 0);
    g_layout3->addWidget(reload_config_Btn, 3, 1);

    h_layout1->addLayout(v_layout1, 0);
    h_layout1->addLayout(g_layout3, 1);
    widget->setLayout(h_layout1);
    this->setCentralWidget(widget);

    connect_signals();
    load_database();
}
void MainWindow::destruct_all()
{
    this->setCentralWidget(nullptr);

    on_stopBtn_clicked();

    matcher_thread->quit();
    matcher_thread->wait();
    delete matcher_thread;

    delete widget;

    // delete a itemview will not result in the delete of its model
    delete data;
}

void MainWindow::connect_signals()
{
    connect(startBtn, SIGNAL(clicked()), this, SLOT(on_startBtn_clicked()));
    connect(startBtn, SIGNAL(clicked()), matcher, SLOT(main_matcher()), Qt::QueuedConnection);

    connect(stopBtn, SIGNAL(clicked()), this, SLOT(on_stopBtn_clicked()));
    connect(reloadBtn, SIGNAL(clicked()), this, SLOT(on_reloadBtn_clicked()));
    connect(openCSVBtn, SIGNAL(clicked()), this, SLOT(on_openCSVBtn_clicked()));
    connect(cptoclpbdBtn, SIGNAL(clicked()), this, SLOT(on_cptoclpbdBtn_clicked()));
    connect(saveAsBtn, SIGNAL(clicked()), this, SLOT(on_saveAsBtn_clicked()));
    connect(clearrecordBtn, SIGNAL(clicked()), this, SLOT(on_clearrecordBtn_clicked()));
    connect(manual_0Btn, SIGNAL(clicked()), this, SLOT(on_manual_0Btn_clicked()));
    connect(manual_1Btn, SIGNAL(clicked()), this, SLOT(on_manual_1Btn_clicked()));
    connect(open_config_Btn, SIGNAL(clicked()), this, SLOT(on_open_config_Btn_clicked()));
    connect(reload_config_Btn, SIGNAL(clicked()), this, SLOT(on_reload_config_Btn_clicked()));

    connect(matcher, SIGNAL(got_match_step(MatcherGotType, size_t)),
            this, SLOT(on_matcher_got_match_step(MatcherGotType, size_t)));
    connect(matcher, SIGNAL(exited(ErrorType)),
            this, SLOT(on_matcher_exited(ErrorType)));

    connect(data, SIGNAL(warning_corrupted_csv(std::filesystem::path)),
            this, SLOT(on_database_warning_corrupted_csv(std::filesystem::path)));
    connect(data, SIGNAL(good_csv(std::filesystem::path)),
            this, SLOT(on_database_good_csv(std::filesystem::path)));
}
void MainWindow::on_database_warning_corrupted_csv(std::filesystem::path path)
{
    corrupted_csv_lbl->setVisible(true);
}
void MainWindow::on_database_good_csv(std::filesystem::path path)
{
    corrupted_csv_lbl->setVisible(false);
}

void MainWindow::ensure_config()
{
    // if (prog::env::config::preprocessed::stats_tbl_color_background.typeId() == 0x1003)
    // {
    //     stats_tbl->setPalette(prog::env::config::preprocessed::stats_tbl_color_background.value<QColor>());
    // }

    for (size_t i = 0; i < prog::env::config::stats_tbl_column_width.size(); ++i)
    {
        stats_tbl->setColumnWidth(i, prog::env::config::stats_tbl_column_width[i]);
    }
    for (size_t i = 0; i < data->get_stats()->rowCount(); ++i)
    {
        stats_tbl->setRowHeight(i, prog::env::config::stats_tbl_rows_height);
    }
    stats_tbl->setFixedWidth(prog::env::config::stats_tbl_column_width[0] +
                             prog::env::config::stats_tbl_column_width[1] +
                             prog::env::config::stats_tbl_column_width[2] + 5);
    stats_tbl->setFixedHeight(prog::env::config::stats_tbl_rows_height * data->get_stats()->rowCount() + 5);

    record_tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    record_tbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    for (size_t i = 0; i < prog::env::config::record_tbl_column_width.size(); ++i)
    {
        if (prog::env::config::record_tbl_column_width[i] == 0)
        {
            record_tbl->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeMode::ResizeToContents);
        }
        else
        {
            record_tbl->setColumnWidth(i, prog::env::config::record_tbl_column_width[i]);
        }
    }

    record_tbl->setItemDelegateForColumn(3, new MyDelegate(prog::env::config::preprocessed::custom_list_deck));
    record_tbl->setItemDelegateForColumn(4, new MyDelegate(prog::env::config::preprocessed::custom_list_note));
}
ErrorType MainWindow::load_database()
{
    std::string data_csv_name;
    if (prog::env::config::misc_use_daily_record_csv)
    {
        auto t = get_local_time();
        data_csv_name = std::format("data-{:04}-{:02}-{:02}.csv",
                                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    }
    else
    {
        data_csv_name = prog::env::default_data_csv_name;
    }

    std::string data_csv_full_name = prog::env::data_csv_path + data_csv_name;
    logln(format("loading record from '{}'", data_csv_full_name));

    std::filesystem::path csv_filepath(data_csv_full_name);

    if (!DataBase::ensure_csv(csv_filepath))
    {
        logln("cannot ensure csv!");
        return ErrorType::ErrEnsureCSV;
    }

    if (data->load_csv(csv_filepath))
    {
        auto_scroll_record_tbl();

        return ErrorType::OK;
    }
    return ErrorType::ErrOthers;
}
// ensure the latest item is visible
void MainWindow::auto_scroll_record_tbl()
{
    record_tbl->scrollToBottom();
}
void MainWindow::add_record_to_db_by_lbl()
{
    data->append_record({coin_lbl->text(), st_nd_lbl->text(), result_lbl->text(), {}, {}, time_lbl->text()});
    auto_scroll_record_tbl();
}
void MainWindow::clear_lbl()
{
    coin_lbl->clear();
    st_nd_lbl->clear();
    result_lbl->clear();
    time_lbl->clear();
}
void MainWindow::on_startBtn_clicked()
{
    logln("Matcher running...");
    start_stop_switch(false);

    disable_manual_btn(false);
    manual_reset();
}
void MainWindow::on_stopBtn_clicked()
{
    logln("Matcher stopping...");
    matcher->request_stop();
}
void MainWindow::on_openCSVBtn_clicked()
{
    ShellExecuteA(0, "open", prog::env::data_csv_path.c_str(), 0, 0, SW_SHOWNORMAL);
}
void MainWindow::on_saveAsBtn_clicked()
{
    QString new_csv_filename =
        QFileDialog::getSaveFileName(
            this,
            "另存为csv",
            (prog::env::data_csv_path + "new.csv").c_str(),
            "csv(*.csv);;All files(*.*)");
    if (new_csv_filename.isEmpty())
    {
        return;
    }
    logln(format("save csv as '{}'", new_csv_filename.toStdString()));
    data->save_csv_as({new_csv_filename.toStdString()});
}
void MainWindow::on_clearrecordBtn_clicked()
{
    auto chosen_button = QMessageBox::question(
        this,
        "清除所有记录",
        "是否清除所有记录?此操作不可逆",
        QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
        QMessageBox::No);
    if (chosen_button == QMessageBox::Yes)
    {
        logln("clearing all records by user");
        data->trunc_last(~0);
        data->save_csv();
    }
}
void MainWindow::on_cptoclpbdBtn_clicked()
{
    data->get_stats()->copy_to_clipboard();

    if (prog::env::config::misc_show_clip_success)
    {
        QMessageBox msgbox;
        msgbox.setWindowTitle("Congratulations");

        static auto e = std::mt19937_64(std::random_device{}());
        static auto d = std::uniform_int_distribution<size_t>(0, prog::env::clip_pic_name_list.size() - 1); // [a, b]
        auto chosen = d(e);

        QLabel *giflabel = new QLabel;
        QMovie movie{(prog::env::clip_pic_path + prog::env::clip_pic_name_list[chosen]).c_str()};
        giflabel->setMovie(&movie);

        QLabel *textlabel = new QLabel;
        textlabel->setText("喜报:成功复制到剪贴板了!");
        auto font = prog::global::font;
        font.setPointSize(26);
        textlabel->setFont(font);

        QVBoxLayout *v_layout = new QVBoxLayout;
        v_layout->addWidget(giflabel);
        v_layout->addWidget(textlabel);

        delete msgbox.layout();
        msgbox.setLayout(v_layout);

        movie.start();
        msgbox.show();
        msgbox.exec();
    }
}
void MainWindow::on_open_config_Btn_clicked()
{
    ShellExecuteA(0, "open", prog::env::config_filename.c_str(), 0, 0, SW_SHOWNORMAL);
}
void MainWindow::on_reload_config_Btn_clicked()
{
    destruct_all();
    prog::env::config::load_prog_config();
    construct_all();
}
void MainWindow::on_matcher_got_match_step(MatcherGotType got, size_t n)
{
    switch (got)
    {
    case MatcherGotType::Coin:
        clear_lbl();
        coin_lbl->setText({n == 0 ? "赢币" : "输币"});
        manual_0Btn->setText("先攻");
        manual_1Btn->setText("后攻");
        disable_manual_btn(false);
        break;
    case MatcherGotType::St_nd:
        st_nd_lbl->setText({n == 0 ? "先攻" : "后攻"});
        manual_0Btn->setText("胜利");
        manual_1Btn->setText("失败");
        disable_manual_btn(false);
        break;
    case MatcherGotType::Result:
    {
        result_lbl->setText({n == 0 ? "胜利" : "失败"});

        auto t = get_local_time();
        auto t_string = std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
                                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                    t->tm_hour, t->tm_min, t->tm_sec);
        time_lbl->setText(t_string.c_str());
        manual_0Btn->setText("赢币");
        manual_1Btn->setText("输币");
        disable_manual_btn(false);
        add_record_to_db_by_lbl();
        data->save_csv();
    }
    break;
    default:
        break;
    }
}
void MainWindow::on_matcher_exited(ErrorType err)
{
    start_stop_switch(true);

    disable_manual_btn(true);

    logln(format("Matcher exited with {}", size_t(err)));
}
void MainWindow::manual_reset()
{
    manual_0Btn->setText("赢币");
    manual_1Btn->setText("输币");
}
void MainWindow::start_stop_switch(bool start)
{
    startBtn->setEnabled(start);
    stopBtn->setDisabled(start);
}
void MainWindow::set_qss()
{
    const auto qss = QString{"QPushButton { background-color: %1; color : %2;}"
                             "QPushButton:disabled { background-color: %3; color : %4;}"};
    startBtn->setStyleSheet(qss
                                .arg(prog::env::config::preprocessed::button_color_start_enabled_background)
                                .arg(prog::env::config::preprocessed::button_color_start_enabled_foreground)
                                .arg(prog::env::config::preprocessed::button_color_start_disabled_background)
                                .arg(prog::env::config::preprocessed::button_color_start_disabled_foreground));
    startBtn->setFont(prog::global::font);
    stopBtn->setStyleSheet(qss
                               .arg(prog::env::config::preprocessed::button_color_stop_enabled_background)
                               .arg(prog::env::config::preprocessed::button_color_stop_enabled_foreground)
                               .arg(prog::env::config::preprocessed::button_color_stop_disabled_background)
                               .arg(prog::env::config::preprocessed::button_color_stop_disabled_foreground));
    stopBtn->setFont(prog::global::font);
    manual_0Btn->setStyleSheet(qss
                                   .arg(prog::env::config::preprocessed::button_color_manual0_enabled_background)
                                   .arg(prog::env::config::preprocessed::button_color_manual0_enabled_foreground)
                                   .arg(prog::env::config::preprocessed::button_color_manual0_disabled_background)
                                   .arg(prog::env::config::preprocessed::button_color_manual0_disabled_foreground));
    manual_0Btn->setFont(prog::global::font);
    manual_1Btn->setStyleSheet(qss
                                   .arg(prog::env::config::preprocessed::button_color_manual1_enabled_background)
                                   .arg(prog::env::config::preprocessed::button_color_manual1_enabled_foreground)
                                   .arg(prog::env::config::preprocessed::button_color_manual1_disabled_background)
                                   .arg(prog::env::config::preprocessed::button_color_manual1_disabled_foreground));
    manual_1Btn->setFont(prog::global::font);
}
void MainWindow::on_manual_0Btn_clicked()
{
    disable_manual_btn(true);
    matcher->set_extern_input(0);
}
void MainWindow::on_manual_1Btn_clicked()
{
    disable_manual_btn(true);
    matcher->set_extern_input(1);
}
void MainWindow::disable_manual_btn(bool disable)
{
    manual_0Btn->setDisabled(disable);
    manual_1Btn->setDisabled(disable);
}
void MainWindow::on_reloadBtn_clicked()
{
    load_database();
}