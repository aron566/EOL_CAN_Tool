#include "eol_angle_calibration_window.h"
#include "ui_eol_angle_calibration_window.h"
#include <QFile>

eol_angle_calibration_window::eol_angle_calibration_window(QString title, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::eol_angle_calibration_window)
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

  /* 初始化定时器 */
  timer_init();
}

eol_angle_calibration_window::~eol_angle_calibration_window()
{
  delete ui;
}

/**
 * @brief 窗口关闭事件
 * @param event
 */
void eol_angle_calibration_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}

/**
 * @brief 定时器初始化
 */
void eol_angle_calibration_window::timer_init()
{
  timer_obj = new QTimer(this);
  timer_obj->setInterval(25);
  connect(timer_obj, &QTimer::timeout, this, &eol_angle_calibration_window::slot_timeout);
}

/**
 * @brief 设置eol协议栈对象
 * @param obj
 */
void eol_angle_calibration_window::set_eol_protocol_obj(eol_protocol *obj)
{
  if(nullptr == obj)
  {
    return;
  }
  eol_protocol_obj = obj;
  /* 数据接收 */
//  connect(eol_protocol_obj, &eol_protocol::signal_rw_device_ok, this, &eol_calibration_window::slot_rw_device_ok);
  connect(eol_protocol_obj, &eol_protocol::signal_protocol_rw_err, this, &eol_angle_calibration_window::slot_protocol_rw_err, Qt::BlockingQueuedConnection);
  connect(eol_protocol_obj, &eol_protocol::signal_rw_device_ok, this, &eol_angle_calibration_window::slot_rw_device_ok, Qt::BlockingQueuedConnection);
}

/**
 * @brief 添加配置
 */
void eol_angle_calibration_window::on_add_config_pushButton_clicked()
{

}

/**
 * @brief 清除配置
 */
void eol_angle_calibration_window::on_clear_config_pushButton_clicked()
{

}

/**
 * @brief 启动校准
 */
void eol_angle_calibration_window::on_start_pushButton_clicked()
{

}


/**
 * @brief 定时器
 */
void eol_angle_calibration_window::slot_timeout()
{
  if(nullptr == eol_protocol_obj)
  {
    return;
  }

  /* 检测是否刷新 */
  if(true == eol_protocol_obj->task_is_runing())
  {
    return;
  }

  /* 启动eol线程 */
  eol_protocol_obj->start_task();
}

/**
 * @brief 数据接收
 */
void eol_angle_calibration_window::slot_rw_device_ok(quint8 reg_addr, const quint8 *data, quint16 data_size)
{
  switch(reg_addr)
  {
    case EOL_W_SAVE_PAR_REG:
      break;

    case EOL_RW_PROFILE_ID_REG:
      break;

    /* 角度校准 */
    case EOL_W_2DFFT_CONDITION_REG:
    case EOL_R_2DFFT_DATA_REG:

      break;
    case EOL_RW_RCS_OFFSET_REG:
    case EOL_W_PAR_RESET_REG:
    case EOL_RW_CALI_MODE_REG:
      break;
  }
}

/**
 * @brief 从机读写无反应信号
 */
void eol_angle_calibration_window::slot_protocol_rw_err(quint8 reg, quint8 command)
{
  switch(reg)
  {
    case EOL_W_SAVE_PAR_REG:
      break;
    /* rcs校准，目标测试 */
    case EOL_R_OBJ_LIST_REG:
      {
        if(false == timer_obj->isActive())
        {
          return;
        }
        eol_protocol::EOL_TASK_LIST_Typedef_t task;
        task.run_state = true;
        task.param = nullptr;

        /* 读取2DFFT */
        task.reg = EOL_R_2DFFT_DATA_REG;
        task.command = eol_protocol::EOL_READ_CMD;
        task.buf[0] = 0;
        task.len = 0;
        eol_protocol_obj->eol_master_common_rw_device(task);
      }
      break;

    case EOL_RW_PROFILE_ID_REG:

    /* 角度校准 */
    case EOL_W_2DFFT_CONDITION_REG:
    case EOL_R_2DFFT_DATA_REG:

    case EOL_RW_RCS_OFFSET_REG:
    case EOL_W_PAR_RESET_REG:
    case EOL_RW_CALI_MODE_REG:
      break;
  }
}
