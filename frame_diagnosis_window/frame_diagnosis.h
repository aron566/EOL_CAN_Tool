#ifndef FRAME_DIAGNOSIS_H
#define FRAME_DIAGNOSIS_H

#include <QWidget>

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
   */
  void add_msg_to_table(uint16_t id, const quint8 *data, quint32 len);

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

};

#endif // FRAME_DIAGNOSIS_H
