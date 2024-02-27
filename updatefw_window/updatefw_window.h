/**
 *  @file updatefw_window.h
 *
 *  @date 2024-02-23
 *
 *  @author Copyright (c) 2024 aron566 <aron566@163.com>.
 *
 *  @brief 更新页窗口
 *
 *  @version V1.0
 */
#ifndef UPDATE_WINDOW_H
#define UPDATE_WINDOW_H
/** Includes -----------------------------------------------------------------*/

/** Private includes ---------------------------------------------------------*/
#include <QWidget>
#include <QFileDialog>/**< 打开文件对话框*/
#include <QFileInfo>/**< 获取文件信息*/
#include <QTemporaryFile>
/** Private defines ----------------------------------------------------------*/
#include "can_driver_model.h"
/** Exported typedefines -----------------------------------------------------*/
namespace Ui {
class updatefw_window;
}

class updatefw_protocol;
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/
/**
 * @brief The updatefw_window class
 */
class updatefw_window : public QWidget
{
    Q_OBJECT

public:
    explicit updatefw_window(QString title, QWidget *parent = nullptr);
    ~updatefw_window();

    /**
     * @brief set_can_driver_obj
     * @param can_driver_obj
     */
    void set_can_driver_obj(can_driver_model *can_driver_obj);

private:
    /*重置发送信息*/
    inline void reset_send_info();

    /*重置接收信息*/
    inline void reset_recv_info();

signals:
    void signal_window_closed();

public slots:
    /* 发送数据发生超时错误 */
    void slot_send_data_timeout_occured();

    /* 接收固件数据 */
    void slot_recv_frimware_data(const char *data, quint16 package_num);

    /* 发送时间戳 */
    void slot_timer_timeout();

    /**
     * @brief 发送进度信号
     * @param current_size 当前大小
     * @param total_size 总大小
     */
    void slot_send_progress(quint32 current_size, quint32 total_size);

    /**
     * @brief 协议栈运行状态信息
     * @param msg 信息
     */
    void slot_protocol_run_step_msg(QString msg);

    /**
     * @brief 读写错误信号
     * @param cmd 命令
     */
    void slot_protocol_rw_err(QString cmd);

private slots:

    void on_frimware_sel_button_clicked();

    void on_start_update_button_clicked();

    void on_start_upload_button_clicked();

    void on_right_now_radioButton_clicked();

    void on_next_reboot_radioButton_clicked();

    void on_app_radioButton_clicked();

    void on_restore_radioButton_clicked();

    void on_boot_radioButton_clicked();

protected:
    /**
     * @brief closeEvent
     * @param event
     */
    virtual void closeEvent(QCloseEvent *event);
private:
    Ui::updatefw_window *ui;

    updatefw_protocol *protocol_stack_obj = nullptr;
private:
    QFile file;             /**< 文件对象*/
    QString filename;       /**< 打开的文件名称*/
    qint64 filesize;        /**< 文件大小信息*/
    qint64 sendfilesize;    /**< 已发送文件大小*/
    qint64 recvfilesize;    /**< 已接收文件大小*/
    qint32 timeout_cnt = 0; /**< 发送超时错误统计*/

    QFile recv_file;        /**< 接收文件对象*/

    QTemporaryFile tmpFile; /**< 临时文件（解密） */
private:
    quint8 right_now_update = 1;
    quint8 download_app_partition = 0;
    bool run_state = false;
private:
    QTimer *timer = nullptr;
    qint32 current_sec = 0;
    qint32 current_min = 0;
};

#endif // UPDATE_WINDOW_H
/******************************** End of file *********************************/
