#ifndef EOL_SUB_WINDOW_H
#define EOL_SUB_WINDOW_H

#include <QWidget>
#include "eol_protocol.h"

namespace Ui {
class eol_sub_window;
}

class eol_sub_window : public QWidget
{
  Q_OBJECT

public:
  explicit eol_sub_window(QString title, QWidget *parent = nullptr);
  ~eol_sub_window();

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
signals:
  /**
   * @brief 窗口关闭信号
   */
  void signal_window_closed();

private slots:
  /**
   * @brief 读写设备正常
   * @param reg 寄存器
   * @param data 读出的数据，为nullptr代表写设备成功
   * @param data_len 代表数据长度，为0时代表写设备成功
   */
  void slot_rw_device_ok(quint8 reg, const quint8 *data = nullptr, quint16 data_len = 0);

  /**
   * @brief 从机读写无反应
   * @param reg 读写寄存器地址
   * @param command 读写标识
   */
  void slot_protocol_rw_err(quint8 reg, quint8 command);
private slots:

  void on_test_pushButton_clicked();

  void on_write_pushButton_clicked();

private:
  Ui::eol_sub_window *ui;

private:
  eol_protocol *eol_protocol_obj = nullptr;

private:
  quint8 version_info[28];/**< 版本号 */
};

#endif // EOL_SUB_WINDOW_H
