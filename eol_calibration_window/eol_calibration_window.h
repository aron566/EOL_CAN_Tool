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

public slots:
  /**
   * @brief 添加配置文件信息到链表
   * @param info
   */
  void slot_profile_info_update(eol_protocol::CALIBRATION_PROFILE_INFO_Typedef_t &info)
  {
    calibration_profile_info_list.append(info);
  }

  /**
   * @brief 清除配置信息
   */
  void slot_clear_profile_info()
  {
    calibration_profile_info_list.clear();
  }

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

  /**
   * @brief 刷新目标列表
   * @param profile_id 目标所属配置id
   * @param obj_num 目标数量
   * @param data 目标信息数据
   */
  void refresh_obj_list_info(quint8 profile_id, quint16 obj_num, const quint8 *data);
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

  void on_add_pushButton_clicked();

  void on_reset_pushButton_clicked();

  void on_test_start_pushButton_clicked();

  void on_distance_comboBox_currentTextChanged(const QString &arg1);

  void on_profile_id_comboBox_currentIndexChanged(int index);

  void on_refresh_time_lineEdit_editingFinished();

  void on_read_rcs_offset_pushButton_clicked();

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

  QTimer *timer_obj = nullptr;

  /* 错误计数 */
  uint8_t err_cnt = 0;

  /* 帧计数 */
  quint32 frame_cnt = 0;
  quint32 time_s = 0;
  quint32 time_ms = 0;
  double fps = 0;

  /* 校准配置信息 */
  QList<eol_protocol::CALIBRATION_PROFILE_INFO_Typedef_t>calibration_profile_info_list;
};

#endif // EOL_CALIBRATION_WINDOW_H
