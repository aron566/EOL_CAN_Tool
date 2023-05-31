#include "debug_window.h"
#include "ui_debug_window.h"
#include <QFile>

debug_window::debug_window(QString title, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::debug_window)
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

debug_window::~debug_window()
{
  delete ui;
}
