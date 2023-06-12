#ifndef TOOL_WINDOW_H
#define TOOL_WINDOW_H

#include <QWidget>
#include "middleware/serial_port_plotter/serial_port_plotter.hpp"

namespace Ui {
class tool_window;
}

class tool_window : public QWidget
{
  Q_OBJECT

public:
  explicit tool_window(QString title, QWidget *parent = nullptr);
  ~tool_window();

private slots:
  void on_data_wave_pushButton_clicked();

private:
  Ui::tool_window *ui;

  serial_port_plotter *serial_port_plotter_win = nullptr;
private:
  /**
   * @brief 数据曲线工具初始化
   */
  void serial_port_plotter_init();
};

#endif // TOOL_WINDOW_H
