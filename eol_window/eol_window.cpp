#include "eol_window.h"
#include "qtimer.h"
#include "ui_eol_window.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

eol_window::eol_window(QString titile, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::eol_window)
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
  this->setWindowTitle(titile);

  /* 初始化定时器 */
  timer_init();

  /* 初始化状态 */
  reset_ui_info();
}

eol_window::~eol_window()
{
  delete ui;
}

void eol_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_eol_window_closed();
}

void eol_window::timer_init()
{
  timer_obj = new QTimer(this);
  connect(timer_obj, &QTimer::timeout, this, &eol_window::slot_timeout);
  timer_obj->setInterval(1000);
}

void eol_window::slot_timeout()
{
  time_cnt++;
  ui->time_cnt_val_label->setNum((int)time_cnt);
}

void eol_window::slot_protocol_complete()
{
  QMessageBox message(QMessageBox::Information, tr("通知"), tr("<font size='10' color='green'>传输完成！</font>"), QMessageBox::Yes, nullptr);
  message.exec();
}


void eol_window::reset_ui_info()
{
  ui->transfer_progressBar->setValue(0);
  ui->bytes_lcdNumber->display(0);
  ui->frame_lcdNumber->display(0);
  ui->time_cnt_val_label->setNum(0);
  ui->error_num_val_label->setNum(0);
  ui->update_pushButton->setEnabled(false);
}

void eol_window::csv_analysis(QString &file_path)
{
  if(file_path.isEmpty())
  {
    return;
  }

  /* 选择文件前先关闭 */
  if(current_file.isOpen() == true)
  {
    current_file.close();
  }
  current_file.setFileName(file_path);
  bool is_ok = current_file.open(QIODevice::ReadOnly);
  if(is_ok == false)
  {
    qDebug() << "只读打开文件失败 78";
    return;
  }

}

void eol_window::on_file_sel_pushButton_clicked()
{
  /* 选择文件前先关闭 */
  if(current_file.isOpen() == true)
  {
    current_file.close();
  }

  /* 选择文件 */
  current_file_path = QFileDialog::getOpenFileName(this, tr("Open File"), "../", tr("csv (*.csv);;BIN (*.bin)"));
  if(current_file_path.isEmpty() == false)
  {
      current_file_name.clear();
      current_filesize = 0;
      /* 获取文件信息 */
      QFileInfo info(current_file_path);
      current_file_name = info.fileName();
      current_filesize = info.size();

      /* 检查非法空格字段 */
      current_file_name.replace(QChar(' '), QChar('_'));

#if 0
      /* 检查文件名后缀 */
      QString suffix = info.suffix();
      if("htpkg" == suffix)
      {
        /* 解密 */
        /* 创建临时文件 */
        tmpFile.remove();
        if(tmpFile.open() == false)
        {
          LOG_DEBUG << "open tmp file faild";
          return;
        }
        cmd_process cmd_process_obj;
        if(cmd_process_obj.cmd_process_start(filepath, tmpFile.fileName(), cmd_process::DECRYPT_MODE) != cmd_process::CMD_PROCESS_OK)
        {
          LOG_DEBUG << "DECRYPT Faild";
          return;
        }
        /* 设置解密后的文件名 */
        filename = info.completeBaseName() + tr(".") + tr("bin");
        filepath = tmpFile.fileName();
      }
#endif
      /* 设置待写入表信息 */
      if(table_list.size() == 0)
      {
        ui->file_name_lineEdit->setText(current_file_name);
        ui->file_size_lineEdit->setText(QString("%1").arg(current_filesize));

        /* 设置文件名 */
        current_file.setFileName(current_file_path);
      }

      /* 允许点击更新 */
      ui->update_pushButton->setEnabled(true);
  }
  else
  {
    qDebug() << "打开文件错误！62";
  }
}


void eol_window::on_upload_pushButton_clicked()
{
  /* 查重 */
  for(qint32 i = 0; i < table_list.size(); i++)
  {
    qDebug() << table_list.value(i).file_path;
    qDebug() << table_list.value(i).file_name;
    qDebug() << table_list.value(i).table_type;
  }
}


void eol_window::on_update_pushButton_clicked()
{
  if(table_list.isEmpty())
  {
    return;
  }
  /* 启动计时 */
  timer_obj->start();

  /* 解析 */

}

void eol_window::on_add_list_pushButton_clicked()
{
  TABLE_INFO_Typedef_t table;

  /* 查重 */
  for(qint32 i = 0; i < table_list.size(); i++)
  {
    if(current_file_path == table_list.value(i).file_path)
    {
      return;
    }
  }

  table.table_type = ui->table_type_comboBox->currentIndex();
  table.data_type = ui->data_type_comboBox->currentIndex();
  table.file_name = current_file_name;
  table.file_path = current_file_path;
  table.show_info = QString("%1 -- %2\r\n").arg(table_list.size() + 1).arg(current_file_name);
  table_list.append(table);

  ui->transfer_list_val_label->clear();
  QString show_info;
  show_info.clear();
  for(qint32 i = 0; i < table_list.size(); i++)
  {
    show_info += table_list.value(i).show_info;
  }
  ui->transfer_list_val_label->setText(show_info);
}


void eol_window::on_clear_list_pushButton_clicked()
{
  /* 清空传输列表 */
  table_list.clear();
  ui->transfer_list_val_label->clear();
}

