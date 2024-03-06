/**
 *  @file tool_window.hpp
 *
 *  @date 2024年01月18日 11:12:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-01-18 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 */
#ifndef TOOL_WINDOW_H
#define TOOL_WINDOW_H
/** Includes -----------------------------------------------------------------*/
#include <QDebug>
#include <QWidget>
/** Private includes ---------------------------------------------------------*/
#include "middleware/serial_port_plotter/serial_port_plotter.hpp"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

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
/******************************** End of file *********************************/



