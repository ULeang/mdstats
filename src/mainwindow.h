#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTableView>
#include <QThread>
#include <QVBoxLayout>

#include "databasemodel.hpp"
#include "delegate.hpp"
#include "matcher.hpp"
#include "utils.hpp"

class MainWindow : public QMainWindow {
  Q_OBJECT
private:
  QTableView  *stats_tbl, *record_tbl;
  MyDelegate  *delegate0, *delegate1, *delegate2, *delegate3, *delegate4;
  QPushButton *startBtn, *stopBtn, *cptoclpbdBtn, *reloadBtn, *openCSVBtn, *saveAsBtn,
    *clearrecordBtn;
  QPushButton *manual_0Btn, *manual_1Btn;
  QPushButton *open_config_Btn, *reload_config_Btn;
  QLabel      *coin_lbl, *st_nd_lbl, *result_lbl, *time_lbl;
  QLabel      *corrupted_csv_lbl;
  QHBoxLayout *h_layout1;
  QVBoxLayout *v_layout1;
  QGridLayout *g_layout1, *g_layout2, *g_layout3;
  QWidget     *widget;

  DataBase *data;

  QThread       *matcher_thread;
  MatcherWorker *matcher;

  void reset();
  void construct_all();
  void destruct_all();

  void      connect_signals();
  ErrorType load_database();
  ErrorType load_database(std::string data_csv_full_name);
  void      add_record_to_db_by_lbl();
  void      clear_lbl();

  void auto_scroll_record_tbl();
  void disable_manual_btn(bool disable);
  void start_stop_switch(bool start);
  void set_qss();

  void manual_reset();

  void ensure_config();

  bool query_open_masterduel();

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

signals:
  void start_matcher();

private slots:
  void on_startBtn_clicked();
  void on_stopBtn_clicked();
  void on_openCSVBtn_clicked();
  void on_cptoclpbdBtn_clicked();
  void on_reloadBtn_clicked();
  void on_saveAsBtn_clicked();
  void on_clearrecordBtn_clicked();
  void on_manual_0Btn_clicked();
  void on_manual_1Btn_clicked();
  void on_open_config_Btn_clicked();
  void on_reload_config_Btn_clicked();
  void on_matcher_got_match_step(MatcherGotType got, size_t n);
  void on_matcher_exited(ErrorType err);
  void on_database_warning_corrupted_csv(std::filesystem::path path);
  void on_database_good_csv(std::filesystem::path path);
};

#endif  // MAINWINDOW_H
