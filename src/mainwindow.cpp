#include "prog.hpp"
#include "mainwindow.h"

#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFontDatabase>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QColor>
#include <windows.h>

using std::format;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      stats_tbl(stat_tbl_row, stat_tbl_col),
      record_tbl(),
      startBtn("Start"),
      stopBtn("Stop"),
      cptoclpbdBtn("Copy Stats"),
      reloadBtn("Reload"),
      openCSVBtn("Open data.csv"),
      coin_lbl(),
      st_nd_lbl(),
      result_lbl(),
      time_lbl(),
      data(),
      manual_victoryBtn("VICTORY"),
      manual_defeatBtn("DEFEAT")
{
    // forbid user to modify the table
    stats_tbl.setEditTriggers(QAbstractItemView::NoEditTriggers);
    // hide headers
    stats_tbl.horizontalHeader()->setVisible(false);
    stats_tbl.verticalHeader()->setVisible(false);
    // set grid
    stats_tbl.setShowGrid(false);
    // set cell size
    stats_tbl.setColumnWidth(0, 150);
    stats_tbl.setColumnWidth(1, 50);
    stats_tbl.setColumnWidth(2, 50);
    for (int i = 0; i < stat_tbl_row; ++i)
    {
        stats_tbl.setRowHeight(i, 30);
    }
    // set span
    stats_tbl.setSpan(0, 1, 1, 2);
    stats_tbl.setSpan(3, 1, 1, 2);
    stats_tbl.setSpan(4, 1, 1, 2);
    stats_tbl.setSpan(5, 1, 1, 2);
    // set x, y, width, height
    // stat_tbl.setGeometry({0, 0, 255, 185});
    // set all cells text and align
    struct
    {
        int r;
        int c;
        const char *text;
    } table_arr[] = {
        {0, 0, "总场次"},
        {1, 0, "硬币(赢/输)"},
        {2, 0, "胜负(胜/负)"},
        {3, 0, "赢币胜率"},
        {4, 0, "输币胜率"},
        {5, 0, "综合胜率"},
        {0, 1, ""},
        {1, 1, ""},
        {1, 2, ""},
        {2, 1, ""},
        {2, 2, ""},
        {3, 1, ""},
        {4, 1, ""},
        {5, 1, ""}};
    const size_t n_table_arr = sizeof(table_arr) / sizeof(table_arr[0]);
    for (size_t i = 0; i < n_table_arr; ++i)
    {
        stats_tbl.setItem(table_arr[i].r, table_arr[i].c, new QTableWidgetItem{table_arr[i].text});
        stats_tbl.item(table_arr[i].r, table_arr[i].c)->setTextAlignment(Qt::AlignCenter);
    }

    record_tbl.setEditTriggers(QAbstractItemView::NoEditTriggers);
    record_tbl.setColumnCount(record_tbl_col);
    for (int i = 0; i < record_tbl_col; ++i)
    {
        record_tbl.setColumnWidth(i, 80);
    }
    record_tbl.horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    record_tbl.setHorizontalHeaderLabels({"硬币", "先后", "胜负", "卡组", "时间"});
    // record_tbl.setGeometry(255, 0, 490, 295);

    stopBtn.setDisabled(true);
    startBtn.setEnabled(true);
    startBtn.setPalette(Qt::green);
    stopBtn.setPalette(Qt::gray);

    disable_manual_result_btn(true);

    // set font
    // auto font = QFont("FiraCode Nerd Font Mono", 16);
    auto font = QFont(QFontDatabase::applicationFontFamilies(prog::global::qt_font_id).at(0), 14);
    stats_tbl.setFont(font);
    record_tbl.setFont(font);
    startBtn.setFont(font);
    stopBtn.setFont(font);
    reloadBtn.setFont(font);
    openCSVBtn.setFont(font);
    cptoclpbdBtn.setFont(font);
    coin_lbl.setFont(font);
    st_nd_lbl.setFont(font);
    result_lbl.setFont(font);
    time_lbl.setFont(font);
    manual_victoryBtn.setFont(font);
    manual_defeatBtn.setFont(font);

    // set layout
    auto g_layout1 = new QGridLayout;
    g_layout1->addWidget(&stats_tbl, 0, 0, 3, 6);
    g_layout1->addWidget(&manual_victoryBtn, 3, 0, 1, 3);
    g_layout1->addWidget(&manual_defeatBtn, 3, 3, 1, 3);
    g_layout1->addWidget(&coin_lbl, 4, 0, 1, 2);
    g_layout1->addWidget(&st_nd_lbl, 4, 2, 1, 2);
    g_layout1->addWidget(&result_lbl, 4, 4, 1, 2);
    g_layout1->addWidget(&time_lbl, 5, 0, 1, 6);
    g_layout1->addWidget(&cptoclpbdBtn, 6, 0, 1, 6);
    g_layout1->addWidget(&startBtn, 7, 0, 1, 3);
    g_layout1->addWidget(&stopBtn, 7, 3, 1, 3);
    auto g_layout2 = new QGridLayout;
    g_layout2->addWidget(&record_tbl, 0, 0, 1, 2);
    g_layout2->addWidget(&reloadBtn, 1, 0);
    g_layout2->addWidget(&openCSVBtn, 1, 1);
    auto h_layout1 = new QHBoxLayout;
    h_layout1->addLayout(g_layout1, 2);
    h_layout1->addLayout(g_layout2, 3);
    auto widget = new QWidget;
    widget->setLayout(h_layout1);
    this->setCentralWidget(widget);

    ensure_config();
    load_record_tbl();
    connect_signals();
}

MainWindow::~MainWindow() {}

void MainWindow::connect_signals()
{
    connect(&startBtn, SIGNAL(clicked()), this, SLOT(on_startBtn_clicked()));
    connect(&stopBtn, SIGNAL(clicked()), this, SLOT(on_stopBtn_clicked()));
    connect(&reloadBtn, SIGNAL(clicked()), this, SLOT(on_reloadBtn_clicked()));
    connect(&openCSVBtn, SIGNAL(clicked()), this, SLOT(on_openCSVBtn_clicked()));
    connect(&cptoclpbdBtn, SIGNAL(clicked()), this, SLOT(on_cptoclpbdBtn_clicked()));
    connect(&manual_victoryBtn, SIGNAL(clicked()), this, SLOT(on_manual_victoryBtn_clicked()));
    connect(&manual_defeatBtn, SIGNAL(clicked()), this, SLOT(on_manual_defeatBtn_clicked()));

    static const auto signal_emitter = MySignalEmitter::instance();
    connect(signal_emitter, SIGNAL(matcher_got(MatcherGotType, size_t)),
            this, SLOT(on_emitter_matcher_got(MatcherGotType, size_t)));
    connect(signal_emitter, SIGNAL(matcher_thread_exit(ErrorType)),
            this, SLOT(on_emitter_matcher_thread_exit(ErrorType)));
}

void MainWindow::ensure_config()
{
    stats_tbl.setPalette(QColor{prog::env::config::stats_tbl_background_color.c_str()});
    struct
    {
        int r;
        int c;
    } stats_tbl_active_cell_arr[] = {
        {0, 0},
        {1, 0},
        {2, 0},
        {3, 0},
        {4, 0},
        {5, 0},
        {0, 1},
        {1, 1},
        {1, 2},
        {2, 1},
        {2, 2},
        {3, 1},
        {4, 1},
        {5, 1}};
    const size_t staca_length = sizeof(stats_tbl_active_cell_arr) / sizeof(stats_tbl_active_cell_arr[0]);
    for (size_t i = 0; i < staca_length; ++i)
    {
        stats_tbl.item(stats_tbl_active_cell_arr[i].r,
                       stats_tbl_active_cell_arr[i].c)
            ->setBackground(QColor{prog::env::config::stats_tbl_background_color.c_str()});
        stats_tbl.item(stats_tbl_active_cell_arr[i].r,
                       stats_tbl_active_cell_arr[i].c)
            ->setForeground(QColor{prog::env::config::stats_tbl_foreground_color.c_str()});
    }
}
ErrorType MainWindow::load_record_tbl()
{
    logln(format("loading record from '{}'", prog::env::data_csv_filename));

    record_tbl.clearContents();

    const std::filesystem::path csv_filepath(prog::env::data_csv_filename);

    if (!ensure_csv(std::move(csv_filepath)))
    {
        logln("cannot ensure csv!");
        return ErrorType::ErrEnsureCSV;
    }

    data.load_csv(prog::env::data_csv_filename);
    update_stats_tbl();

    record_tbl.setRowCount(data.rowc);
    for (size_t i = 0; i < data.rowc; ++i)
    {
        record_tbl.setItem(i, 0, new QTableWidgetItem{data.coin_col[i].c_str()});
        record_tbl.setItem(i, 1, new QTableWidgetItem{data.st_nd_col[i].c_str()});
        record_tbl.setItem(i, 2, new QTableWidgetItem{data.result_col[i].c_str()});
        record_tbl.setItem(i, 3, new QTableWidgetItem{data.deck_col[i].c_str()});
        record_tbl.setItem(i, 4, new QTableWidgetItem{data.time_col[i].c_str()});
        update_record_tbl_color(i);
    }

    auto_scroll_record_tbl();

    return ErrorType::OK;
}
void MainWindow::update_record_tbl_color(int row)
{
    record_tbl.item(row, 0)->setBackground(data.coin_col[row] == "赢币" ? Qt::green : Qt::red);
    record_tbl.item(row, 1)->setBackground(data.st_nd_col[row] == "先攻" ? Qt::green : Qt::red);
    record_tbl.item(row, 2)->setBackground(data.result_col[row] == "胜利"   ? Qt::green
                                           : data.result_col[row] == "失败" ? Qt::red
                                                                            : Qt::blue);
    for (int i = 0; i < 3; ++i)
    {
        record_tbl.item(row, i)->setForeground(Qt::black);
    }
}
// ensure the latest item is visible
void MainWindow::auto_scroll_record_tbl()
{
    record_tbl.scrollToItem(record_tbl.item(data.rowc - 1, 0), QAbstractItemView::EnsureVisible);
}
void MainWindow::update_data_and_record_tbl_by_lbl()
{
    auto r = data.rowc;
    auto coin = coin_lbl.text().toStdString();
    auto st_nd = st_nd_lbl.text().toStdString();
    auto result = result_lbl.text().toStdString();
    auto deck = std::string{};
    auto t = time_lbl.text().toStdString();
    data.append_record(coin, st_nd, result, deck, t);
    append_to_csv(prog::env::data_csv_filename,
                  format("{},{},{},{},{},{}\n", r + 1, coin, st_nd, result, deck, t));
    update_stats_tbl();

    record_tbl.insertRow(r);
    record_tbl.setItem(r, 0, new QTableWidgetItem{coin_lbl.text()});
    record_tbl.setItem(r, 1, new QTableWidgetItem{st_nd_lbl.text()});
    record_tbl.setItem(r, 2, new QTableWidgetItem{result_lbl.text()});
    record_tbl.setItem(r, 3, new QTableWidgetItem{""});
    record_tbl.setItem(r, 4, new QTableWidgetItem{time_lbl.text()});
    update_record_tbl_color(r);
    auto_scroll_record_tbl();
}
void MainWindow::clear_lbl()
{
    coin_lbl.clear();
    st_nd_lbl.clear();
    result_lbl.clear();
    time_lbl.clear();
}
void MainWindow::update_stats_tbl()
{
    auto total = data.rowc;
    auto coin_win = data.w_st_wins + data.w_st_loses + data.w_st_others +
                    data.w_nd_wins + data.w_nd_loses + data.w_nd_others;
    auto coin_lose = data.l_st_wins + data.l_st_loses + data.l_st_others +
                     data.l_nd_wins + data.l_nd_loses + data.l_nd_others;
    auto res_win = data.w_st_wins + data.w_nd_wins +
                   data.l_st_wins + data.l_nd_wins;
    auto res_lose = data.w_st_loses + data.w_nd_loses +
                    data.l_st_loses + data.l_nd_loses;
    auto coin_win_res_win = data.w_st_wins + data.w_nd_wins;
    auto coin_lose_res_win = data.l_st_wins + data.l_nd_wins;
    auto win_rate_coin_win = double(coin_win_res_win) / coin_win;
    auto win_rate_coin_lose = double(coin_lose_res_win) / coin_lose;
    auto win_rate_total = double(res_win) / total;
    stats_tbl.item(0, 1)->setText(QString::number(data.rowc));
    stats_tbl.item(1, 1)->setText(QString::number(coin_win));
    stats_tbl.item(1, 2)->setText(QString::number(coin_lose));
    stats_tbl.item(2, 1)->setText(QString::number(res_win));
    stats_tbl.item(2, 2)->setText(QString::number(res_lose));
    stats_tbl.item(3, 1)->setText(coin_win == 0
                                      ? "-"
                                      : std::format("{:5.2f}%", 100 * win_rate_coin_win).c_str());
    stats_tbl.item(4, 1)->setText(coin_lose == 0
                                      ? "-"
                                      : std::format("{:5.2f}%", 100 * win_rate_coin_lose).c_str());
    stats_tbl.item(5, 1)->setText(total == 0
                                      ? "-"
                                      : std::format("{:5.2f}%", 100 * win_rate_total).c_str());
}
void MainWindow::on_startBtn_clicked()
{
    logln("Matcher running...");
    startBtn.setDisabled(true);
    stopBtn.setEnabled(true);
    startBtn.setPalette(Qt::gray);
    stopBtn.setPalette(Qt::red);

    // ~0, i.e. the max value of size_t, represents no manual result
    manual_result_reset();

    task_matcher_thread = std::packaged_task(fn_matcher_thread);
    ret_matcher_thread = task_matcher_thread.get_future();
    thrd_matcher_thread = std::jthread(std::move(task_matcher_thread), std::ref(_a_manual_result));
}
void MainWindow::on_stopBtn_clicked()
{
    logln("Matcher stopping...");
    thrd_matcher_thread.request_stop();
}
void MainWindow::on_openCSVBtn_clicked()
{
    ShellExecuteA(0, "open", prog::env::data_csv_filename.c_str(), 0, 0, SW_SHOWNORMAL);
}
void MainWindow::on_cptoclpbdBtn_clicked()
{
    CopyToClipboard(
        format("总场次\t{}\n硬币(赢/输)\t{}/{}\n胜负(胜/负)\t{}/{}\n赢币胜率\t{}\n输币胜率\t{}\n综合胜率\t{}\n",
               stats_tbl.item(0, 1)->text().toStdString(),
               stats_tbl.item(1, 1)->text().toStdString(),
               stats_tbl.item(1, 2)->text().toStdString(),
               stats_tbl.item(2, 1)->text().toStdString(),
               stats_tbl.item(2, 2)->text().toStdString(),
               stats_tbl.item(3, 1)->text().toStdString(),
               stats_tbl.item(4, 1)->text().toStdString(),
               stats_tbl.item(5, 1)->text().toStdString())
            .c_str());
}

void MainWindow::on_emitter_matcher_got(MatcherGotType got, size_t n)
{
    switch (got)
    {
    case MatcherGotType::Coin:
        clear_lbl();
        coin_lbl.setText({n == 0 ? "赢币" : "输币"});
        break;
    case MatcherGotType::St_nd:
        disable_manual_result_btn(false);
        st_nd_lbl.setText({n == 0 ? "先攻" : "后攻"});
        break;
    case MatcherGotType::Result:
    {
        disable_manual_result_btn(true);
        result_lbl.setText({n == 0 ? "胜利" : "失败"});

        auto t = get_local_time();
        auto t_string = std::format("{:04}{:02}{:02}-{:02}{:02}{:02}",
                                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                    t->tm_hour, t->tm_min, t->tm_sec);
        time_lbl.setText(t_string.c_str());
        update_data_and_record_tbl_by_lbl();
    }
    break;
    default:
        break;
    }
}
void MainWindow::on_emitter_matcher_thread_exit(ErrorType err)
{
    stopBtn.setDisabled(true);
    startBtn.setEnabled(true);
    startBtn.setPalette(Qt::green);
    stopBtn.setPalette(Qt::gray);

    manual_result_reset();
    thrd_matcher_thread.join();

    logln(format("Matcher exited with {}", size_t(err)));
}
void MainWindow::manual_result_reset()
{
    disable_manual_result_btn(true);
    _a_manual_result.store(~0, std::memory_order_release);
}
void MainWindow::on_manual_victoryBtn_clicked()
{
    disable_manual_result_btn(true);
    _a_manual_result.store(0, std::memory_order_release);
}
void MainWindow::on_manual_defeatBtn_clicked()
{
    disable_manual_result_btn(true);
    _a_manual_result.store(1, std::memory_order_release);
}
void MainWindow::disable_manual_result_btn(bool disable)
{
    manual_victoryBtn.setDisabled(disable);
    manual_defeatBtn.setDisabled(disable);
    if (disable)
    {
        manual_victoryBtn.setPalette(Qt::gray);
        manual_defeatBtn.setPalette(Qt::gray);
    }
    else
    {
        manual_victoryBtn.setPalette(Qt::cyan);
        manual_defeatBtn.setPalette(Qt::cyan);
    }
}
void MainWindow::on_reloadBtn_clicked()
{
    load_record_tbl();
}

static auto _matcher_thread_helper(const std::stop_token &stoken, Matcher &matcher,
                                   const std::function<std::optional<cv::Mat>()> &f,
                                   MatcherGotType got, std::atomic_size_t &a_manual_result)
    -> std::expected<size_t, ErrorType>
{
    static const auto signal_emitter = MySignalEmitter::instance();
    while (true)
    {
        // ad-hoc for manual result
        if (got == MatcherGotType::Result)
        {
            auto m_r = a_manual_result.load(std::memory_order_acquire);
            if (m_r != ~0)
            {
                a_manual_result.store(~0, std::memory_order_release);
                signal_emitter->emit_matcher_got(got, m_r);
                return m_r;
            }
        }

        auto match_r = matcher.try_once(f);
        if (match_r.has_value())
        {
            signal_emitter->emit_matcher_got(got, match_r.value());
            return match_r.value();
        }
        else if (stoken.stop_requested())
        {
            signal_emitter->emit_matcher_thread_exit(ErrorType::ErrStopRequested);
            return std::unexpected{ErrorType::ErrStopRequested};
        }
        Sleep(prog::env::config::matcher_sleep_ms);
    }
}
static ErrorType _matcher_thread_emit_exit(ErrorType err)
{
    static const auto signal_emitter = MySignalEmitter::instance();
    signal_emitter->emit_matcher_thread_exit(err);
    return err;
}

static std::expected<std::tuple<std::string, cv::Rect, cv::Rect>, ErrorType>
_determine_resolution(long width, long height)
{
    if (width < 1280)
    {
        return std::unexpected{ErrorType::ErrDeterRes};
    }
    if (width < 1366)
    {
        return std::make_tuple("1280x720", cv::Rect{0, 0, 1280, 720}, cv::Rect{0, 0, 1280, 720});
    }
    if (width < 1440)
    {
        return std::make_tuple("1366x768", cv::Rect{0, 0, 1366, 768}, cv::Rect{0, 0, 1366, 768});
    }
    if (width < 1600)
    {
        return std::make_tuple("1440x810", cv::Rect{0, 0, 1440, 810}, cv::Rect{0, 0, 1440, 810});
    }
    if (width < 1920)
    {
        return std::make_tuple("1600x900", cv::Rect{600, 530, 400, 160}, cv::Rect{550, 300, 570, 380});
    }
    if (width < 2048)
    {
        return std::make_tuple("1920x1080", cv::Rect{700, 630, 500, 200}, cv::Rect{650, 350, 620, 450});
    }
    if (width < 2560)
    {
        return std::make_tuple("2048x1152", cv::Rect{750, 650, 550, 220}, cv::Rect{700, 380, 670, 480});
    }
    if (width < 3200)
    {
        return std::make_tuple("2560x1440", cv::Rect{950, 850, 700, 250}, cv::Rect{880, 450, 840, 600});
    }
    if (width < 3840)
    {
        return std::make_tuple("3200x1800", cv::Rect{1200, 1050, 800, 280}, cv::Rect{1100, 600, 1050, 700});
    }
    return std::make_tuple("3840x2160", cv::Rect{1450, 1300, 950, 300}, cv::Rect{1400, 750, 1150, 700});
}

ErrorType MainWindow::fn_matcher_thread(std::stop_token stoken, std::atomic_size_t &a_manual_result)
{
    static const auto signal_emitter = MySignalEmitter::instance();

    ScreenShot ss;

    if (!ss.is_ok())
    {
        logln("cannot acquire device context, check your monitor");
        return _matcher_thread_emit_exit(ErrorType::ErrDeviceContext);
    }

    HWND hwnd = FindWindowA(NULL, prog::env::capture_window_title.c_str());
    if (!hwnd)
    {
        logln(format("cannot find window '{}'", prog::env::capture_window_title));
        return _matcher_thread_emit_exit(ErrorType::ErrFindWindow);
    }
    auto rect_e = ss.get_window_rect(hwnd);
    if (!rect_e.has_value())
    {
        logln(format("cannot get window rect"));
        return _matcher_thread_emit_exit(rect_e.error());
    }

    double scale = ss.get_screen_scale(true);

    RECT rect = rect_e.value();
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    logln(format("init window size \t: {}x{}", width, height));
    auto resolution_e = _determine_resolution(width, height);
    if (!resolution_e.has_value())
    {
        logln(format("cannot determine resolution"));
        return _matcher_thread_emit_exit(resolution_e.error());
    }
    auto [res_text, crop_coin, crop_result] = resolution_e.value();
    logln(format("determined resolution \t: {}", res_text));

    std::string path{prog::env::opencv_templ_directory + res_text + "\\"};
    if (!check_resources({std::filesystem::path{path + "coin_win.png"},
                          std::filesystem::path{path + "coin_lose.png"},
                          std::filesystem::path{path + "go_first.png"},
                          std::filesystem::path{path + "go_second.png"},
                          std::filesystem::path{path + "victory.png"},
                          std::filesystem::path{path + "defeat.png"}}))
    {
        logln(format(
            "opencv template is missing, or current window resolution '{}' is not supported yet",
            res_text));
        return _matcher_thread_emit_exit(ErrorType::ErrCheckResources);
    }

    auto f_coin = capture_fn_generator(ss, hwnd, scale, crop_coin);
    auto f_result = capture_fn_generator(ss, hwnd, scale, crop_result);

    if constexpr (prog::env::debug::test_capture_flag)
    {
        auto cap_coin = f_coin();
        auto cap_result = f_result();
        logln(format("cap_coin {}exists, cap_result {}exists",
                     cap_coin.has_value() ? "" : "NOT ", cap_result.has_value() ? "" : "NOT "),
              LogLevel::ALWAYS);
        if (cap_coin.has_value())
        {
            cv::imwrite("cap_coin.png", cap_coin.value());
        }
        if (cap_result.has_value())
        {
            cv::imwrite("cap_result.png", cap_result.value());
        }
    }

    Matcher match_coin({(path + "coin_win.png").c_str(), (path + "coin_lose.png").c_str()},
                       prog::env::matcher_threshold,
                       prog::env::debug::matcher_img_log, prog::env::debug::matcher_text_log);
    Matcher match_st_nd({(path + "go_first.png").c_str(), (path + "go_second.png").c_str()},
                        prog::env::matcher_threshold,
                        prog::env::debug::matcher_img_log, prog::env::debug::matcher_text_log);
    Matcher match_result({(path + "victory.png").c_str(), (path + "defeat.png").c_str()},
                         prog::env::matcher_threshold,
                         prog::env::debug::matcher_img_log, prog::env::debug::matcher_text_log);
    while (true)
    {
        auto coin = _matcher_thread_helper(stoken, match_coin, f_coin, MatcherGotType::Coin, a_manual_result);
        if (coin.has_value())
        {
            logln(format("{}", coin.value() == 0 ? "coin win" : "coin lose"));
        }
        else
        {
            return coin.error();
        }
        auto st_nd = _matcher_thread_helper(stoken, match_st_nd, f_coin, MatcherGotType::St_nd, a_manual_result);
        if (st_nd.has_value())
        {
            logln(format("{}", st_nd.value() == 0 ? "go first" : "go second"));
        }
        else
        {
            return st_nd.error();
        }
        auto result = _matcher_thread_helper(stoken, match_result, f_result, MatcherGotType::Result, a_manual_result);
        if (result.has_value())
        {
            logln(format("{}", result.value() == 0 ? "victory" : "defeat"));
        }
        else
        {
            return result.error();
        }
    }
}