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

signals:

  /**
   * @brief 发送波形数据
   * @param data
   */
  void signal_wave_data(QByteArray data);

public slots:

  /**
   * @brief 接收波形数据
   * @param data
   */
  void slot_wave_data(QByteArray data);

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
