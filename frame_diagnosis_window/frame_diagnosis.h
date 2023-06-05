#ifndef FRAME_DIAGNOSIS_H
#define FRAME_DIAGNOSIS_H

#include <QWidget>
#include <QList>

namespace Ui {
class frame_diagnosis;
}

class frame_diagnosis : public QWidget
{
  Q_OBJECT

public:
  explicit frame_diagnosis(QString title, QWidget *parent = nullptr);
  ~frame_diagnosis();

protected:

  /**
   * @brief closeEvent
   * @param event
   */
  virtual void closeEvent(QCloseEvent *event);

public:

  /**
   * @brief 添加消息到表
   * @param id can id
   * @param data 数据
   * @param len 数据长度
   * @param direct 0发送 1接收
   * @param channel_num 通道号
   * @param protocol_type 协议类型 0can 1canfd
   * @param dt 当前时间
   */
  void add_msg_to_table(uint16_t id, const quint8 *data, quint32 len, \
    quint8 direct, quint32 channel_num, quint8 protocol_type, quint64 ms);

  /**
   * @brief 清除统计信息
   */
  void clear();
signals:
  /**
   * @brief 发送窗口关闭信号
   */
  void signal_window_closed();

private:
  Ui::frame_diagnosis *ui;

private:
  typedef struct
  {
    quint16 id;
    quint8 channel_num;
    quint8 direct;
    quint8 protocol_type;
    quint8 data[64];
    quint32 cnt;
    quint32 repeat_cnt;
    double fps;
    double cycle_time_ms;
    quint64 last_time_ms;
  }CAN_MSG_LIST_Typedef_t;

  QList <CAN_MSG_LIST_Typedef_t> can_msg_list;
};

#endif // FRAME_DIAGNOSIS_H
