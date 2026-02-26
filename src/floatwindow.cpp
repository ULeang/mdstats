#include "floatwindow.hpp"
#include <qnamespace.h>
#include <QHeaderView>
#include <QScreen>
#include <QTableView>
#include <QTableWidgetItem>
#include "prog.hpp"
#include "prog_config.hpp"
#include "statstable.hpp"
#include "utils.hpp"

FloatWindow::FloatWindow(QAbstractItemModel *stats_model, QWidget *parent)
  : p_this(nullptr), mouse_pressed(false), mouse_init_pos(), QWidget(parent) {
  QTableView  *table   = new QTableView;
  QHBoxLayout *hlayout = new QHBoxLayout;

  setAttribute(Qt::WA_TranslucentBackground);

  auto x = prog::env::config::misc_float_window_init_geometry_x;
  auto y = prog::env::config::misc_float_window_init_geometry_y;
  if (prog::env::config::misc_float_window_init_geometry_rel_to_md) {
    HWND hwnd = FindWindowA(NULL, prog::env::capture_window_title.c_str());
    if (hwnd) {
      RECT rect;  // left, top, right, bottom
      if (GetWindowRect(hwnd, &rect)) {
        // get scale
        QScreen *screen = this->screen();
        if (!screen) {
          logln("FATAL: cannot get screen", LogLevel::ALWAYS);
          exit(-1);
        }
        qreal scaleFactor = screen->devicePixelRatio();

        x += rect.left / scaleFactor;
        y += rect.top / scaleFactor;
      }
    }
  }
  move(x, y);
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

  table->setModel(stats_model);

  table->horizontalHeader()->setHidden(true);
  table->verticalHeader()->setHidden(true);
  table->setShowGrid(false);

  // set span
  auto spans = MyModule::StatsTable::spans();
  auto rowc  = stats_model->rowCount();
  auto colc  = stats_model->columnCount();
  for (size_t r = 0; r < rowc; ++r) {
    size_t actual_col = 0;
    for (auto span : spans[r]) {
      if (span != 1) {
        table->setSpan(r, actual_col, 1, span);
      }
      actual_col += span;
    }
  }

  auto   cols_width     = MyModule::StatsTable::cols_width();
  size_t cols_width_sum = 0;
  for (size_t i = 0; i < colc; ++i) {
    table->setColumnWidth(i, cols_width[i]);
    cols_width_sum += cols_width[i];
  }
  auto row_height = MyModule::StatsTable::rows_height();
  for (size_t i = 0; i < rowc; ++i) {
    table->setRowHeight(i, row_height);
  }
  table->setFixedWidth(cols_width_sum + 2);
  table->setFixedHeight(row_height * rowc + 2);

  size_t  stats_tbl_updated = 0;
  QString stats_tbl_style_sheet{};
  if (prog::env::config::preprocessed::stats_tbl_color_background_float.typeId() == 0x1003) {
    stats_tbl_style_sheet.append(QString{"background-color : %1"}.arg(
      prog::env::config::stats_tbl_color_background_float.c_str()));
    ++stats_tbl_updated;
  }
  if (prog::env::config::preprocessed::stats_tbl_color_foreground_float.typeId() == 0x1003) {
    if (stats_tbl_updated != 0) {
      stats_tbl_style_sheet.append(';');
    }
    stats_tbl_style_sheet.append(
      QString{"color : %1"}.arg(prog::env::config::stats_tbl_color_foreground_float.c_str()));
    ++stats_tbl_updated;
  }
  if (stats_tbl_updated != 0) {
    table->setStyleSheet(stats_tbl_style_sheet);
  }
  // table->setStyleSheet("QTableView{background-color:transparent}");
  table->setFont(prog::global::font);  // should be set after qss

  table->setAttribute(Qt::WA_TransparentForMouseEvents);

  hlayout->addWidget(table);

  setLayout(hlayout);
}
FloatWindow::~FloatWindow() {
  if (p_this != nullptr) {
    *p_this = nullptr;
  }
}
void FloatWindow::set_self_ptr(FloatWindow **p_this) {
  this->p_this = p_this;
}

void FloatWindow::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {  // 如果鼠标左键按下
    mouse_pressed  = true;
    mouse_init_pos = event->pos();          // 记录当前的点击坐标
  }
}

void FloatWindow::mouseMoveEvent(QMouseEvent *event) {
  if (mouse_pressed) {                                        // 如果鼠标左键按下
    this->move(event->pos() - mouse_init_pos + this->pos());  // 窗口移动
  }
}

// 鼠标释放
void FloatWindow::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mouse_pressed = false;
  }
}
