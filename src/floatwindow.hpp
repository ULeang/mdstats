#ifndef FLOATWINDOW_HPP
#define FLOATWINDOW_HPP

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPushButton>
#include <QTableView>
#include <QThread>
#include <QVBoxLayout>

class FloatWindow : public QWidget {
  Q_OBJECT
private:
  FloatWindow **p_this;

  double screen_scale;
  HWND   hwnd_md;
  bool   mouse_pressed;
  QPoint mouse_init_pos;

public:
  FloatWindow(QAbstractItemModel *stats_model, QWidget *parent = nullptr);
  ~FloatWindow();

  void set_self_ptr(FloatWindow **p_this);

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

signals:

private slots:
};

#endif  // FLOATWINDOW_HPP
