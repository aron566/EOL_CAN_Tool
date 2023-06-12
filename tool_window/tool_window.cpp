#include "tool_window.h"
#include "ui_tool_window.h"

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
}

void tool_window::on_data_wave_pushButton_clicked()
{
  serial_port_plotter_win->show();
}
