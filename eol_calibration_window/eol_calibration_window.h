#ifndef EOL_CALIBRATION_WINDOW_H
#define EOL_CALIBRATION_WINDOW_H

#include <QWidget>
#include <QString>
#include <QList>
#include "eol_protocol.h"

namespace Ui {
class eol_calibration_window;
}

class eol_calibration_window : public QWidget
{
  Q_OBJECT

public:
  explicit eol_calibration_window(QString title, QWidget *parent = nullptr);
  ~eol_calibration_window();

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
   * @brief 接收数据
   * @param reg_addr 寄存器地址
   * @param data 数据地址
   * @param data_size 数据长度
   */
  void slot_rec_data(quint8 reg_addr, const quint8 *data, quint16 data_size);

  void on_add_pushButton_clicked();

  void on_reset_pushButton_clicked();

  void on_test_start_pushButton_clicked();

private:
  Ui::eol_calibration_window *ui;

private:
  eol_protocol *eol_protocol_obj = nullptr;

private:
  typedef struct
  {
    QString str;
    quint8 distance_m;
    qint8 mag_dB_threshold_down;
    qint8 mag_dB_threshold_up;
    qint8 snr_dB_threshold_down;
    qint8 snr_dB_threshold_up;
    quint8 rts_dBsm;
  }THRESHOLD_SET_INFO_Typedef_t;
  QList <THRESHOLD_SET_INFO_Typedef_t> threshold_list;/**< 阈值列表 */
};

#endif // EOL_CALIBRATION_WINDOW_H
