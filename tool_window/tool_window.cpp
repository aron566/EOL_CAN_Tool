/**
 *  @file tool_window.cpp
 *
 *  @date 2024年01月18日 11:11:54 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 *
 *  @brief 更多工具窗口.
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
/** Private includes ---------------------------------------------------------*/
#include "tool_window.h"
#include "ui_tool_window.h"
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

tool_window::tool_window(QString title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tool_window)
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

  /* 初始化数据曲线工具 */
  serial_port_plotter_init();
}

tool_window::~tool_window()
{
  delete ui;
}

void tool_window::serial_port_plotter_init()
{
  serial_port_plotter_win = new serial_port_plotter();

  connect(this, &tool_window::signal_wave_data, serial_port_plotter_win, &serial_port_plotter::read_wave_data);
}

void tool_window::on_data_wave_pushButton_clicked()
{
  serial_port_plotter_win->show();
  if(true == serial_port_plotter_win->isMinimized())
  {
    serial_port_plotter_win->showNormal();
  }
  serial_port_plotter_win->activateWindow();
}

void tool_window::slot_wave_data(QByteArray data)
{
  emit signal_wave_data(data);
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/
/******************************** End of file *********************************/

