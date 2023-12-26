/**
 *  @file rts_ctrl_window.cpp
 *
 *  @date 2023年12月04日 13:47:54 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief rts手动控制页面.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.12.04 13:47 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-12-04 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QFile>
#include <QFileDialog>
/** Private includes ---------------------------------------------------------*/
#include "rts_ctrl_window.h"
#include "ui_rts_ctrl_window.h"

/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/

/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/

/** Private function prototypes ----------------------------------------------*/

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

rts_ctrl_window::rts_ctrl_window(QString title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::rts_ctrl_window)
{
  ui->setupUi(this);

  /* Apply style sheet */
  QFile file(":/qdarkstyle/dark/style.qss");
  if(file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    this->setStyleSheet(file.readAll());
    file.close();
  }

  /* 设置标题 */
  this->setWindowTitle(title);

  /* 设置提示值 */
  ui->freq_lineEdit->setPlaceholderText("76.300");
  ui->range_lineEdit->setPlaceholderText("20.000");
  ui->speed_lineEdit->setPlaceholderText("0.000");
  ui->rcs_lineEdit->setPlaceholderText("10.000");
}

rts_ctrl_window::~rts_ctrl_window()
{
  delete ui;
}

void rts_ctrl_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}
/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

void rts_ctrl_window::on_set_pushButton_clicked()
{
  rts_protocol::RTS_TASK_LIST_Typedef_t task;

  /* 设置频率 */
  if(false == ui->freq_lineEdit->text().isEmpty())
  {
    task.cmd = RTS_SET_FREQUENCY_PORT(ui->freq_lineEdit->text());
    rts_protocol_obj->rts_master_common_rw_device(task);
  }

  /* 设置距离、速度 、rcs */
  task.cmd = RTS_SET_TARGET_PORT(ui->range_lineEdit->text().toUtf8().data(),
                                 ui->speed_lineEdit->text().toUtf8().data(),
                                 ui->rcs_lineEdit->text().toUtf8().data());
  rts_protocol_obj->rts_master_common_rw_device(task);

  /* 启动协议栈 */
  rts_protocol_obj->start_task();
}

/**
   * @brief 协议栈无回复超时
   * @param sec 秒
   */
void rts_ctrl_window::slot_protocol_timeout(quint32 sec)
{
  ui->result_label->setText(tr("result:timeout %1 sec").arg(sec));
}

/**
   * @brief 从机返回错误消息
   * @param error_msg 错误消息
   */
void rts_ctrl_window::slot_protocol_error_occur(quint8 error_msg)
{
  if(0U == error_msg)
  {
    ui->result_label->setText("result:ok");
  }
  else
  {
    ui->result_label->setText("result:err");
  }
}

/**
   * @brief signal_protocol_rw_err 读写错误信号
   * @param cmd 命令
   */
void rts_ctrl_window::slot_protocol_rw_err(QString cmd)
{
  ui->result_label->setText(tr("result:cmd [%1] retry err").arg(cmd));
}
/******************************** End of file *********************************/




