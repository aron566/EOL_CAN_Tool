#ifndef MORE_WINDOW_H
#define MORE_WINDOW_H

#include <QWidget>
#include <QTimer>
#include <QTextEdit>

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
    virtual void wheelEvent(QWheelEvent *event);
//    virtual void showEvent(QShowEvent *event);
  /**
     * @brief 定时器初始化
     */
    void timer_init();

    /**
     * @brief 待显示是否为空
     * @return true空
     */
    bool ch1_show_msg_is_empty();
    bool ch2_show_msg_is_empty();

    /**
     * @brief show_txt
     */
    void show_txt();

    /**
     * @brief 获取显示的索引
     * @param current_show_index
     * @param totaol_size
     * @return 显示索引
     */
    quint32 get_show_index(quint32 current_show_index, quint32 totaol_size);

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

    QList<SHOW_MSG_Typedef_t>ch1_show_msg_list;
    QList<SHOW_MSG_Typedef_t>ch2_show_msg_list;

private:
    /**
     * @brief update_show_msg 显示指定索引消息
     * @param text_edit_widget 控件
     * @param pList 消息链表
     * @param show_index 索引
     * @param downward_flag true 下翻标识 false 上翻标识
     */
    void update_show_msg(QTextEdit *text_edit_widget, QList<SHOW_MSG_Typedef_t> *pList, quint32 show_index, bool downward_flag);


private:
    /* 已显示消息 */
    quint32 ch1_show_msg_index = 0;
    quint32 ch2_show_msg_index = 0;
    /* 已添加消息 */
    quint32 ch1_add_msg_index = 0;
    quint32 ch2_add_msg_index = 0;

    /* 滚动计数 */
    quint32 ch1_scroll_cnt = 0;
    /* 滚动计数 */
    quint32 ch2_scroll_cnt = 0;
};

#endif // MORE_WINDOW_H
