#include "eol_calibration_window.h"
#include "ui_eol_calibration_window.h"
#include <QFile>

eol_calibration_window::eol_calibration_window(QString title, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::eol_calibration_window)
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

  /* 设置提示值 */
  ui->mag_threshold_max_lineEdit->setPlaceholderText("20m105 - 50m85");
  ui->mag_threshold_min_lineEdit->setPlaceholderText("20m90 - 50m70");
  ui->snr_threshold_max_lineEdit->setPlaceholderText("20m70 - 50m50");
  ui->snr_threshold_min_lineEdit->setPlaceholderText("20m45 - 50m30");
  ui->rts_lineEdit->setPlaceholderText("15 - 25 - 35");
}

eol_calibration_window::~eol_calibration_window()
{
  delete ui;
}

/**
 * @brief 窗口关闭事件
 * @param event
 */
void eol_calibration_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}

/**
 * @brief 设置eol协议栈对象
 * @param obj
 */
void eol_calibration_window::set_eol_protocol_obj(eol_protocol *obj)
{
  if(nullptr == obj)
  {
    return;
  }
  eol_protocol_obj = obj;
  /* 数据接收 */
}

/**
 * @brief 添加阈值到列表
 */
void eol_calibration_window::on_add_pushButton_clicked()
{
  THRESHOLD_SET_INFO_Typedef_t threshold_info;
  threshold_info.distance_m = (quint8)ui->distance_comboBox->currentText().toUShort();
  threshold_info.mag_dB_threshold_down = (qint8)ui->mag_threshold_min_lineEdit->text().toShort();
  threshold_info.mag_dB_threshold_up = (qint8)ui->mag_threshold_max_lineEdit->text().toShort();
  threshold_info.snr_dB_threshold_down = (qint8)ui->snr_threshold_min_lineEdit->text().toShort();
  threshold_info.snr_dB_threshold_up = (qint8)ui->snr_threshold_max_lineEdit->text().toShort();
  threshold_info.rts_dBsm = (quint8)ui->rts_lineEdit->text().toUShort();

  threshold_info.str = QString::asprintf("%um,mag:>%u <%u,snr:>%u <%u,rts:%u\r\n", \
                                         threshold_info.distance_m, \
                                         threshold_info.mag_dB_threshold_down, \
                                         threshold_info.mag_dB_threshold_up, \
                                         threshold_info.snr_dB_threshold_down, \
                                         threshold_info.snr_dB_threshold_up, \
                                         threshold_info.rts_dBsm);
  threshold_list.append(threshold_info);
  QString str;
  ui->threshold_list_label->clear();
  for(qint32 i = 0; i < threshold_list.size(); i++)
  {
    str += threshold_list.value(i).str;
  }
  ui->threshold_list_label->setText(str);
}

/**
 * @brief 清空阈值列表
 */
void eol_calibration_window::on_reset_pushButton_clicked()
{
  /* 清空传输列表 */
  threshold_list.clear();
  ui->threshold_list_label->clear();
}

/**
 * @brief 启动测试
 */
void eol_calibration_window::on_test_start_pushButton_clicked()
{
  if(threshold_list.size() == 0)
  {
    return;
  }

  /* 启动mag snr rcs 数值监测任务，及校准任务 */

}


void eol_calibration_window::slot_rec_data(quint8 reg_addr, const quint8 *data, quint16 data_size)
{

}
