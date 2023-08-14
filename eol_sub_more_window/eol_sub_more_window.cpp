/**
  *  @file eol_sub_more_window.cpp
  *
  *  @date 2023年08月14日 15:18:52 星期一
  *
  *  @author aron566
  *
  *  @copyright Copyright (c) 2022 aron566 <aron566@163.com>.
  *
  *  @brief None.
  *
  *  @details None.
  *
  *  @version v1.0.0 aron566 2023.08.14 15:18 初始版本.
  */
/** Includes -----------------------------------------------------------------*/
#include <QFile>
#include <QDebug>
/** Private includes ---------------------------------------------------------*/
#include "eol_sub_more_window.h"
#include "ui_eol_sub_more_window.h"

/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define EOL_DTC_BIT0_MAP  (1U << 0U)
#define EOL_DTC_BIT1_MAP  (1U << 1U)
#define EOL_DTC_BIT2_MAP  (1U << 2U)
#define EOL_DTC_BIT3_MAP  (1U << 3U)
#define EOL_DTC_BIT4_MAP  (1U << 4U)
#define EOL_DTC_BIT5_MAP  (1U << 5U)
#define EOL_DTC_BIT6_MAP  (1U << 6U)
#define EOL_DTC_BIT7_MAP  (1U << 7U)

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

eol_sub_more_window::eol_sub_more_window(QString title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::eol_sub_more_window)
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

eol_sub_more_window::~eol_sub_more_window()
{
  delete ui;
}

void eol_sub_more_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}

void eol_sub_more_window::on_get_err_pushButton_clicked()
{
  emit signal_get_dtc_err_status();
}

void eol_sub_more_window::on_clear_err_pushButton_clicked()
{
  emit signal_clear_dtc_err_status();
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
 *
 *       Public code
 *
 ********************************************************************************
 */

void eol_sub_more_window::set_dtc_err_status(const quint8 *data, quint32 len)
{
  if(7U > len)
  {
    return;
  }

  ui->bit0_checkBox->setChecked(data[0] & EOL_DTC_BIT0_MAP);
  ui->bit1_checkBox->setChecked(data[1] & EOL_DTC_BIT1_MAP);
  ui->bit2_checkBox->setChecked(data[2] & EOL_DTC_BIT2_MAP);
  ui->bit3_checkBox->setChecked(data[3] & EOL_DTC_BIT3_MAP);
  ui->bit4_checkBox->setChecked(data[4] & EOL_DTC_BIT4_MAP);
  ui->bit5_checkBox->setChecked(data[5] & EOL_DTC_BIT5_MAP);
  ui->bit6_checkBox->setChecked(data[6] & EOL_DTC_BIT6_MAP);
}
/******************************** End of file *********************************/
