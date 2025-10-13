#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLabel>

#include <thread>
#include <future>
#include <atomic>

#include "utils.hpp"

enum class MatcherGotType
{
    Coin,
    St_nd,
    Result,
};

// static member function cannot emit a signal, so use this instance to forward it
class MySignalEmitter : public QObject
{
    Q_OBJECT
public:
    static MySignalEmitter *instance()
    {
        static MySignalEmitter emitter;
        return &emitter;
    }
signals:
    void matcher_got(MatcherGotType got, size_t n) const;
    void matcher_thread_exit(ErrorType err) const;

public:
    void emit_matcher_got(MatcherGotType got, size_t n) const
    {
        emit matcher_got(got, n);
    }
    void emit_matcher_thread_exit(ErrorType err) const
    {
        emit matcher_thread_exit(err);
    }

private:
    MySignalEmitter() {}
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    static const size_t stat_tbl_row = 6;
    static const size_t stat_tbl_col = 3;
    static const size_t record_tbl_col = 5;

    QTableWidget stats_tbl, record_tbl;
    QPushButton startBtn, stopBtn, cptoclpbdBtn, reloadBtn, openCSVBtn;
    QPushButton manual_0Btn, manual_1Btn;
    QLabel coin_lbl, st_nd_lbl, result_lbl, time_lbl;

    std::packaged_task<ErrorType(std::stop_token, std::atomic_size_t &)> task_matcher_thread;
    std::future<ErrorType> ret_matcher_thread;
    std::jthread thrd_matcher_thread;

    DataBase data;

    std::atomic_size_t _a_manual;

    void connect_signals();
    ErrorType load_record_tbl();
    void update_data_and_record_tbl_by_lbl();
    void clear_lbl();
    void update_stats_tbl();
    void update_record_tbl_color(int row);

    static ErrorType fn_matcher_thread(std::stop_token stoken, std::atomic_size_t &a_manual);

    void auto_scroll_record_tbl();
    void disable_manual_btn(bool disable);

    void manual_reset();
    
    void ensure_config();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:

private slots:
    void on_startBtn_clicked();
    void on_stopBtn_clicked();
    void on_openCSVBtn_clicked();
    void on_cptoclpbdBtn_clicked();
    void on_emitter_matcher_got(MatcherGotType got, size_t n);
    void on_emitter_matcher_thread_exit(ErrorType err);
    void on_reloadBtn_clicked();
    void on_manual_0Btn_clicked();
    void on_manual_1Btn_clicked();
};

#endif // MAINWINDOW_H