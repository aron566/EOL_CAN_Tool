#include "frame_diagnosis.h"
#include "ui_frame_diagnosis.h"
#include <QFile>

frame_diagnosis::frame_diagnosis(QString title, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::frame_diagnosis)
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

frame_diagnosis::~frame_diagnosis()
{
  delete ui;
}

void frame_diagnosis::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}

void frame_diagnosis::add_msg_to_table(uint16_t id, const quint8 *data, quint32 len)
{

}

void frame_diagnosis::clear()
{

}
