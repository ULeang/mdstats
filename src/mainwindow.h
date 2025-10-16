#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QTableView>
#include <QThread>

#include "utils.hpp"
#include "databasemodel.hpp"
#include "matcher.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QTableView stats_tbl, record_tbl;
    QPushButton startBtn, stopBtn, cptoclpbdBtn, reloadBtn, openCSVBtn;
    QPushButton manual_0Btn, manual_1Btn;
    QLabel coin_lbl, st_nd_lbl, result_lbl, time_lbl;
    QLabel corrupted_csv_lbl;

    DataBase data;

    QThread matcher_thread;
    MatcherWorker matcher;

    void connect_signals();
    ErrorType load_database();
    void add_record_to_db_by_lbl();
    void clear_lbl();

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
    void on_reloadBtn_clicked();
    void on_manual_0Btn_clicked();
    void on_manual_1Btn_clicked();
    void on_matcher_got_match_step(MatcherGotType got, size_t n);
    void on_matcher_exited(ErrorType err);
    void on_database_warning_corrupted_csv(std::filesystem::path path);
    void on_database_good_csv(std::filesystem::path path);
};

#endif // MAINWINDOW_H