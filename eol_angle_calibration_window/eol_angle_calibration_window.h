#ifndef EOL_ANGLE_CALIBRATION_WINDOW_H
#define EOL_ANGLE_CALIBRATION_WINDOW_H

#include <QWidget>
#include <QList>
#include "eol_protocol.h"
#include <QTimer>
namespace Ui {
class eol_angle_calibration_window;
}

class eol_angle_calibration_window : public QWidget
{
  Q_OBJECT

public:
  explicit eol_angle_calibration_window(QString title, QWidget *parent = nullptr);
  ~eol_angle_calibration_window();

  /**
   * @brief 设置eol协议栈对象
   * @param obj 协议栈对象
   */
  void set_eol_protocol_obj(eol_protocol *obj = nullptr);
protected:
    /**
     * @brief closeEvent
     * @param event
     */
    virtual void closeEvent(QCloseEvent *event) override;

private:

  /**
   * @brief 定时器初始化
   */
  void timer_init();

signals:
  /**
   * @brief 窗口关闭信号
   */
  void signal_window_closed();

private slots:
  /**
   * @brief 接收数据
   * @param reg_addr 寄存器地址
   * @param data 数据地址
   * @param data_size 数据长度
   */
  void slot_rw_device_ok(quint8 reg_addr, const quint8 *data, quint16 data_size);

  /**
   * @brief 从机读写无反应信号
   * @param reg 读写寄存器地址
   * @param command 读写标识
   */
  void slot_protocol_rw_err(quint8 reg, quint8 command);

  /**
   * @brief 定时器超时
   */
  void slot_timeout();
  void on_add_config_pushButton_clicked();

  void on_clear_config_pushButton_clicked();

  void on_start_pushButton_clicked();

private:
  Ui::eol_angle_calibration_window *ui;

private:
  QTimer *timer_obj = nullptr;
  eol_protocol *eol_protocol_obj = nullptr;
};

#endif // EOL_ANGLE_CALIBRATION_WINDOW_H
