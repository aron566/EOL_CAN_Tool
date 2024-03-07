/**
 *  @file eol_sub_window.cpp
 *
 *  @date 2024年01月18日 11:11:54 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 *
 *  @brief eol信息读写窗口.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2024.01.18 12:11 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-01-18 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QFile>
/** Private includes ---------------------------------------------------------*/
#include "eol_sub_window.h"
#include "ui_eol_sub_window.h"
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

  /* 下级窗口初始化 */
  rw_more_window_init(tr("EOL CAN Tool - Device Info RW ..."));
}

eol_sub_window::~eol_sub_window()
{
  delete eol_sub_more_window_obj;
  delete ui;
}

void eol_sub_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}

void eol_sub_window::rw_more_window_init(QString title)
{
  eol_sub_more_window_obj = new eol_sub_more_window(title);
  connect(eol_sub_more_window_obj, &eol_sub_more_window::signal_window_closed, this, [=]
          {
            this->show();
            if(true == this->isMinimized())
            {
              this->showNormal();
            }
            this->activateWindow();
          });
  connect(eol_sub_more_window_obj, &eol_sub_more_window::signal_get_dtc_err_status, this, [=]
          {
            this->update_dtc_err_state();
          });
  connect(eol_sub_more_window_obj, &eol_sub_more_window::signal_clear_dtc_err_status, this, [=]
          {
            this->clear_dtc_err_state();
          });
}

void eol_sub_window::update_dtc_err_state()
{
  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 读取dtc错误码 */
  task.reg = EOL_RW_DTC_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

void eol_sub_window::clear_dtc_err_state()
{
  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 清除dtc错误码 */
  task.reg = EOL_RW_DTC_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = 1;
  task.len = 1;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

void eol_sub_window::set_eol_protocol_obj(eol_protocol *obj)
{
  if(nullptr == obj)
  {
    return;
  }
  eol_protocol_obj = obj;

  connect(eol_protocol_obj, &eol_protocol::signal_rw_device_ok, this, &eol_sub_window::slot_rw_device_ok, Qt::QueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_protocol_rw_err, this, &eol_sub_window::slot_protocol_rw_err, Qt::BlockingQueuedConnection);
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

void eol_sub_window::on_test_pushButton_clicked()
{
  ui->ver_test_lineEdit->clear();
  ui->sn_write_lineEdit->clear();
  ui->sn_read_lineEdit->clear();

  ui->vcan_test_lineEdit->clear();
  ui->hw_ver_lineEdit->clear();
  ui->soft_ver_lineEdit->clear();
  ui->calibration_ver_lineEdit->clear();
  ui->usd_hw_ver_lineEdit->clear();
  ui->usd_soft_ver_lineEdit->clear();
  ui->usd_boot_ver_lineEdit->clear();

  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  /* 软硬件版本号 */
  task.reg = EOL_RW_VERSION_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* VCAN测试 */
  task.reg = EOL_W_VCAN_TEST_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = 1;
  task.len = 1;
  task.com_hw = eol_protocol::EOL_CAN_HW;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* SN读写 */
  task.reg = EOL_RW_SN_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 引脚状态 */
  task.reg = EOL_R_MOUNTID_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 设置需要获取的电压类型 */
  if(ui->voltage_type_comboBox->currentText() != "NULL")
  {
    task.reg = EOL_RW_VOLTAGE_REG;
    task.command = eol_protocol::EOL_WRITE_CMD;
    task.buf[0] = (quint8)ui->voltage_type_comboBox->currentText().toUShort();
    task.len = 1;
    eol_protocol_obj->eol_master_common_rw_device(task);
  }
  /* 电压 */
  task.reg = EOL_RW_VOLTAGE_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 芯片SN */
  task.reg = EOL_R_CHIP_SN_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* PMIC SN */
  task.reg = EOL_R_PMIC_SN_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* client did */
  task.reg = EOL_R_CLIENT_DID_VER_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 读取dtc错误码 */
  task.reg = EOL_RW_DTC_REG;
  task.command = eol_protocol::EOL_READ_CMD;
  task.buf[0] = 0;
  task.len = 0;
  eol_protocol_obj->eol_master_common_rw_device(task);

  /* 看门狗测试 */
  task.reg = EOL_W_WDG_REG;
  task.command = eol_protocol::EOL_WRITE_CMD;
  task.buf[0] = (quint8)ui->wdg_opt_comboBox->currentIndex();
  task.len = 1;
  eol_protocol_obj->eol_master_common_rw_device(task);

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
        ui->ver_test_lineEdit->setText(ui->ver_test_lineEdit->text() + tr("write version ok "));
        break;

      /* VCAN测试 */
      case EOL_W_VCAN_TEST_REG:
        ui->vcan_test_lineEdit->setText(ui->vcan_test_lineEdit->text() + tr("write vcan ok "));
        break;

      case EOL_RW_SN_REG      :
        ui->sn_write_lineEdit->setText(ui->sn_write_lineEdit->text() + tr("write sn ok "));
        break;

      case EOL_W_WDG_REG      :
        ui->wdg_opt_status_label->setText(ui->wdg_opt_comboBox->currentText() + tr(" ok"));
        break;

      /* 写spi测试 */
      case EOL_RW_SPI_REG:
        {

        }
        break;

      /* 写i2c测试 */
      case EOL_RW_I2C_REG:
        {

        }
        break;

      /* 清除dtc测试 */
      case EOL_RW_DTC_REG:
        {
          /* ok，触发一次更新 */
          update_dtc_err_state();
        }
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
      /* 版本号 */
      case EOL_RW_VERSION_REG :
        {
          str.clear();
          quint16 index = 0;
          for(quint16 i = 0; i < 4; i++)
          {
            str += QString::asprintf("%02X", data[index]);
            index++;
          }
          ui->hw_ver_lineEdit->setText(str);

          str.clear();
          for(quint16 i = 0; i < 4; i++)
          {
            str += QString::asprintf("%02X", data[index]);
            index++;
          }
          ui->soft_ver_lineEdit->setText(str);

          str.clear();
          for(quint16 i = 0; i < 2; i++)
          {
            str += QString::asprintf("%02X", data[index]);
            index++;
          }
          ui->calibration_ver_lineEdit->setText(str);

          str.clear();
          for(quint16 i = 0; i < 5; i++)
          {
            str += QString::asprintf("%02X", data[index]);
            index++;
          }
          ui->usd_hw_ver_lineEdit->setText(str);

          str.clear();
          for(quint16 i = 0; i < 6; i++)
          {
            str += QString::asprintf("%02X", data[index]);
            index++;
          }
          ui->usd_soft_ver_lineEdit->setText(str);

          str.clear();
          for(quint16 i = 0; i < 7; i++)
          {
            str += QString::asprintf("%02X", data[index]);
            index++;
          }
          ui->usd_boot_ver_lineEdit->setText(str);

          ui->ver_test_lineEdit->setText(tr("read version ok "));

          /* 回写 */
          eol_protocol::EOL_TASK_LIST_Typedef_t task;
          task.param = nullptr;

          task.reg = reg;
          task.command = eol_protocol::EOL_WRITE_CMD;
          memcpy(task.buf, data, data_len);
          memcpy(version_info, data, data_len);
          task.len = data_len;
          eol_protocol_obj->eol_master_common_rw_device(task);

          /* 启动eol线程 */
          eol_protocol_obj->start_task();
        }
        break;

      /* SN测试 */
      case EOL_RW_SN_REG      :
        {
          str.clear();
          for(quint16 i = 0; i < data_len; i++)
          {
            if(0U == data[i])
            {
              qDebug() << str << "sn true len" << i;
              break;
            }
            str += QString::asprintf("%c", data[i]);
          }
          qDebug() << str << "sn len" << data_len;
          ui->sn_number_lineEdit->setText(str);
          ui->sn_read_lineEdit->setText(tr("read sn ok "));

          /* 回写 */
          eol_protocol::EOL_TASK_LIST_Typedef_t task;
          task.param = nullptr;

          task.reg = reg;
          task.command = eol_protocol::EOL_WRITE_CMD;
          memcpy(task.buf, data, data_len);
          task.len = data_len;
          eol_protocol_obj->eol_master_common_rw_device(task);

          /* 启动eol线程 */
          eol_protocol_obj->start_task();
        }
        break;

      /* MOUNTID测试 */
      case EOL_R_MOUNTID_REG   :
        {
          str.clear();
          ui->pin7_lineEdit->setText(QString::asprintf("%02X", data[0]));
          ui->pin8_lineEdit->setText(QString::asprintf("%02X", data[1]));
        }
        break;

      /* 读写电压 */
      case EOL_RW_VOLTAGE_REG:
        {
          quint16 voltage = 0;
          memcpy(&voltage, data, sizeof(voltage));
          qDebug() << "voltage                   " << voltage;
          ui->voltage_lineEdit->setText(QString::asprintf("%fv", (float)voltage / 10.f));
        }
        break;

      /* 读取芯片SN */
      case EOL_R_CHIP_SN_REG:
        {
          quint8 Bytes = data[0];
          str.clear();
          for(quint16 i = 0; i < Bytes; i++)
          {
            str += QString::asprintf("%02X", data[i + 1]);
          }
          qDebug() << "chip sn                   " << Bytes;
          ui->chip_sn_lineEdit->setText(str);
        }
        break;

      /* 读取pmicSN */
      case EOL_R_PMIC_SN_REG:
        {
          quint8 Bytes = data[0];
          str.clear();
          for(quint16 i = 0; i < Bytes; i++)
          {
            str += QString::asprintf("%02X", data[i + 1]);
          }
          ui->pmic_sn_lineEdit->setText(str);
        }
        break;

      /* 读取客户did版本信息，硬件 软件 */
      case EOL_R_CLIENT_DID_VER_REG:
        {
          quint8 Bytes = data[0];
          str.clear();
          for(quint16 i = 0; i < Bytes; i++)
          {
            str += QString::asprintf("%02X", data[i + 1]);
          }
          ui->did_hw_sf_ver_lineEdit->setText(str);
        }
        break;

      /* 读spi测试 */
      case EOL_RW_SPI_REG:
        {

        }
        break;

      /* 读i2c测试 */
      case EOL_RW_I2C_REG:
        {

        }
        break;

      /* 读dtc测试 */
      case EOL_RW_DTC_REG:
        {
          eol_sub_more_window_obj->set_dtc_err_status(data, data_len);
        }
        break;

      default:
        break;
    }
  }
}

void eol_sub_window::slot_protocol_rw_err(quint8 reg, quint8 command)
{
  switch(reg)
  {
    case EOL_RW_VERSION_REG :

      break;
    case EOL_W_VCAN_TEST_REG:

      break;
    case EOL_RW_SN_REG      :
      if(command)
      {

      }
      break;
    case EOL_R_MOUNTID_REG   :
      qDebug() << "EOL_R_MOUNTID_REG err";
      break;

    case EOL_RW_VOLTAGE_REG   :
      qDebug() << "EOL_RW_VOLTAGE_REG err";
      break;

    case EOL_R_CHIP_SN_REG   :
      qDebug() << "EOL_R_CHIP_SN_REG err";
      break;

    case EOL_R_PMIC_SN_REG   :
      qDebug() << "EOL_R_PMIC_SN_REG err";
      break;

    case EOL_R_CLIENT_DID_VER_REG   :
      qDebug() << "EOL_R_CLIENT_DID_VER_REG err";
      break;
  }
}

void eol_sub_window::on_write_pushButton_clicked()
{
  /* 写入测试 */
  bool write_en_flag = false;
  eol_protocol::EOL_TASK_LIST_Typedef_t task;
  task.param = nullptr;

  if(false == ui->calibration_ver_lineEdit->text().isEmpty())
  {
    write_en_flag = true;
    /* 写入校准版本 */
    task.reg = EOL_RW_VERSION_REG;
    task.command = eol_protocol::EOL_WRITE_CMD;
    bool ok;
    quint16 ver = ui->calibration_ver_lineEdit->text().toUShort(&ok, 16);
    memcpy(version_info + 8, &ver, sizeof(ver));
    memcpy(task.buf, version_info, sizeof(version_info));
    task.len = sizeof(version_info);
    eol_protocol_obj->eol_master_common_rw_device(task);
  }

  if(false == ui->sn_number_lineEdit->text().isEmpty())
  {
    write_en_flag = true;
    /* 写入SN */
    task.reg = EOL_RW_SN_REG;
    task.command = eol_protocol::EOL_WRITE_CMD;
    QByteArray p_sn = ui->sn_number_lineEdit->text().toLatin1();
    memcpy(task.buf, p_sn.data(), ui->sn_number_lineEdit->text().size());
    task.len = ui->sn_number_lineEdit->text().size();
    eol_protocol_obj->eol_master_common_rw_device(task);
  }

  if(true == write_en_flag)
  {
    /* 保存 */
    task.reg = EOL_W_SAVE_PAR_REG;
    task.command = eol_protocol::EOL_WRITE_CMD;
    task.buf[0] = 1;
    task.len = 1;
    eol_protocol_obj->eol_master_common_rw_device(task);

    /* 启动eol线程 */
    eol_protocol_obj->start_task();
  }
}

void eol_sub_window::on_more_pushButton_clicked()
{
  eol_sub_more_window_obj->show();
  if(true == eol_sub_more_window_obj->isMinimized())
  {
    eol_sub_more_window_obj->showNormal();
  }
  eol_sub_more_window_obj->activateWindow();
}

/******************************** End of file *********************************/
