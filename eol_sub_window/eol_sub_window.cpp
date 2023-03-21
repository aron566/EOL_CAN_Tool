#include "eol_sub_window.h"
#include "ui_eol_sub_window.h"
#include <QFile>

eol_sub_window::eol_sub_window(QString title, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::eol_sub_window)
{
  ui->setupUi(this);

  /* Apply style sheet */
  QFile file(":/qdarkstyle/dark/style.qss");
  if(file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    this->setStyleSheet(file.readAll());
    file.close();
  }

  /* 设置窗口标题 */
  this->setWindowTitle(title);
}

eol_sub_window::~eol_sub_window()
{
  delete ui;
}

void eol_sub_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}

void eol_sub_window::set_eol_protocol_obj(eol_protocol *obj)
{
  if(nullptr == obj)
  {
    return;
  }
  eol_protocol_obj = obj;

  connect(eol_protocol_obj, &eol_protocol::signal_rw_device_ok, this, &eol_sub_window::slot_rw_device_ok);
  connect(eol_protocol_obj, &eol_protocol::signal_protocol_rw_err, this, &eol_sub_window::slot_protocol_rw_err);
}

void eol_sub_window::on_test_pushButton_clicked()
{
  ui->ver_test_lineEdit->clear();
  ui->sn_write_lineEdit->clear();
  ui->sn_read_lineEdit->clear();

  ui->hw_ver_lineEdit->clear();
  ui->soft_ver_lineEdit->clear();
  ui->calibration_ver_lineEdit->clear();
  ui->usd_hw_ver_lineEdit->clear();
  ui->usd_soft_ver_lineEdit->clear();
  ui->usd_boot_ver_lineEdit->clear();

  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.run_state = true;
  task.param = nullptr;

  /* 软硬件版本号 */
  task.reg = EOL_RW_VERSION_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* VCAN测试 */
//  task.reg = EOL_RW_VERSION_REG;
//  task.command = eol_protocol::EOL_READ_CMD;
//  task.buf[0] = 0;
//  task.len = 0;
//  eol_protocol_obj->eol_master_common_rw_device(task);

  /* SN读写 */
  task.reg = EOL_RW_SN_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 引脚状态 */

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

void eol_sub_window::slot_rw_device_ok(quint8 reg, const quint8 *data, quint16 data_len)
{
  QString str;
  if(nullptr == data)
  {
    /* 写结果 */
    switch(reg)
    {
      case EOL_RW_VERSION_REG :
        ui->ver_test_lineEdit->setText(ui->ver_test_lineEdit->text() + tr("write version ok"));
        break;
      case EOL_RW_SN_REG      :
        ui->sn_write_lineEdit->setText(ui->sn_write_lineEdit->text() + tr("write sn ok"));
        break;
      default                 :
        break;
    }

  }
  else
  {
    /* 读结果 */
    switch(reg)
    {
      case EOL_RW_VERSION_REG :
        {
          for(quint16 i = 0; i < 4; i++)
          {
            str += QString::asprintf("%02X", data[i]);
          }
          ui->hw_ver_lineEdit->setText(str);
          str.clear();
          for(quint16 i = 4; i < 8; i++)
          {
            str += QString::asprintf("%02X", data[i]);
          }
          ui->soft_ver_lineEdit->setText(str);
          str.clear();
          for(quint16 i = 8; i < 10; i++)
          {
            str += QString::asprintf("%02X", data[i]);
          }
          ui->calibration_ver_lineEdit->setText(str);
          str.clear();
          for(quint16 i = 10; i < 15; i++)
          {
            str += QString::asprintf("%02X", data[i]);
          }
          ui->usd_hw_ver_lineEdit->setText(str);
          str.clear();
          for(quint16 i = 15; i < 21; i++)
          {
            str += QString::asprintf("%02X", data[i]);
          }
          ui->usd_soft_ver_lineEdit->setText(str);
          str.clear();
          for(quint16 i = 21; i < 28; i++)
          {
            str += QString::asprintf("%02X", data[i]);
          }
          ui->usd_boot_ver_lineEdit->setText(str);

          ui->ver_test_lineEdit->setText(tr("read version ok "));

          /* 回写 */
          eol_protocol::EOL_TASK_LIST_Typedef_t task;
          task.run_state = true;
          task.param = nullptr;

          task.reg = reg;
          task.command = eol_protocol::EOL_WRITE_CMD;
          memcpy(task.buf, data, data_len);
          task.len = data_len;
          eol_protocol_obj->eol_master_common_rw_device(task);
        }

        break;
      case EOL_R_VCAN_TEST_REG:
        ui->vcan_test_lineEdit->setText(tr("read vcan ok "));
        break;
      case EOL_RW_SN_REG      :
        {
          str.clear();
          for(quint16 i = 0; i < 24; i++)
          {
            str += QString::asprintf("%02X", data[i]);
          }
          ui->sn_number_lineEdit->setText(str);
          ui->sn_read_lineEdit->setText(tr("read sn ok "));

          /* 回写 */
          eol_protocol::EOL_TASK_LIST_Typedef_t task;
          task.run_state = true;
          task.param = nullptr;

          task.reg = reg;
          task.command = eol_protocol::EOL_WRITE_CMD;
          memcpy(task.buf, data, data_len);
          task.len = data_len;
          eol_protocol_obj->eol_master_common_rw_device(task);
        }

        break;
      case EOL_R_PIN7_8_REG   :

        break;
    }
    /* 启动eol线程 */
    eol_protocol_obj->start_task();
  }
}

void eol_sub_window::slot_protocol_rw_err(quint8 reg, quint8 command)
{
  switch(reg)
  {
    case EOL_RW_VERSION_REG :

      break;
    case EOL_R_VCAN_TEST_REG:

      break;
    case EOL_RW_SN_REG      :

      break;
    case EOL_R_PIN7_8_REG   :

      break;
  }
}
