#ifndef MORE_WINDOW_H
#define MORE_WINDOW_H

#include <QWidget>
#include <QTimer>
#include "eol_window.h"
#include "can_driver.h"

namespace Ui {
class more_window;
}

class more_window : public QWidget
{
    Q_OBJECT

public:
  explicit more_window(QString title, QWidget *parent = nullptr);
  ~more_window();

  /**
   * @brief 设置can驱动
   * @param can_driver_
   */
  void set_can_driver_obj(can_driver *can_driver_ = nullptr)
  {
    can_driver_obj = can_driver_;
    connect(can_driver_obj, &can_driver::signal_show_message_bytes, this, &more_window::slot_show_message_bytes);
    connect(can_driver_obj, &can_driver::signal_show_message, this, &more_window::slot_show_message);
    /* 线程同步 */
    connect(can_driver_obj, &can_driver::signal_show_thread_message, this, &more_window::slot_show_message_block, Qt::BlockingQueuedConnection);
  }

  /**
   * @brief 设置通道数量
   * @param channel_num 通道数量
   */
  void set_channel_num(quint8 channel_num);
public:
  eol_window *eol_ui = nullptr;
protected:
    /**
     * @brief closeEvent
     * @param event
     */
    virtual void closeEvent(QCloseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
//    virtual void showEvent(QShowEvent *event);
  /**
     * @brief 定时器初始化
     */
    void timer_init();
signals:
    void signal_more_window_closed();

private slots:
    void on_frame_type_comboBox_currentIndexChanged(int index);

    void on_protocol_comboBox_currentIndexChanged(int index);

    void on_canfd_pluse_comboBox_currentIndexChanged(int index);

    void on_id_lineEdit_textChanged(const QString &arg1);

    void on_data_lineEdit_textChanged(const QString &arg1);

    void on_eol_test_pushButton_2_clicked();

    void on_clear_pushButton_clicked();

    void on_send_pushButton_clicked();

    void on_period_lineEdit_textChanged(const QString &arg1);

private slots:
    void slot_timeout();
    void on_display_mask_lineEdit_textChanged(const QString &arg1);

    void on_mask_en_checkBox_clicked(bool checked);
    void slot_show_message_bytes(quint8 bytes, quint32 channel_num, quint8 direct);
    void slot_show_message(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data = nullptr, quint32 data_len = 0);
    void slot_show_message_block(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data = nullptr, quint32 data_len = 0);
private:
    Ui::more_window *ui;

private:
    can_driver *can_driver_obj = nullptr;
    QTimer *timer_obj = nullptr;
    quint32 rx_frame_cnt = 0;
    quint32 tx_frame_cnt = 0;
    quint32 rx_byte_cnt = 0;
    quint32 tx_byte_cnt = 0;
    quint32 last_canid_mask = 0;
    bool last_canid_mask_en = false;

    typedef struct
    {
      QString str;
      quint32 channel_num;
      quint8 direct;
    }SHOW_MSG_Typedef_t;

    QList<SHOW_MSG_Typedef_t>show_msg_list;

    quint32 ch1_show_msg_cnt = 0;
    quint32 ch2_show_msg_cnt = 0;
};

#endif // MORE_WINDOW_H
