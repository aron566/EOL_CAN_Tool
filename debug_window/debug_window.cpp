#include "debug_window.h"
#include "ui_debug_window.h"
#include <QFile>
#include <QSettings>
#include <QDebug>

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

  /* 定时器初始化 */
  timer_init();

  /* 设置默认值 */
  ui->case_sensitive_checkBox->setCheckState(Qt::CheckState::Checked);
  ui->shell_textEdit->installEventFilter(this);
  ui->shell_textEdit->setHtml("<body bgcolor=\"#000000\"></body>");
  minTextCurse = ui->shell_textEdit->textCursor().position();

  QString logo = ("  _____   _   _   _____   __   _   _____   _____   _____   _____   _   _        _____   _       _\r\n"
    " /  ___| | | | | | ____| |  \\ | | /  ___| |_   _| | ____| /  ___| | | | |      /  ___| | |     | |\r\n"
    " | |     | |_| | | |__   |   \\| | | |       | |   | |__   | |     | |_| |      | |     | |     | |\r\n"
    " | |     |  _  | |  __|  | |\\   | | |  _    | |   |  __|  | |     |  _  |      | |     | |     | |\r\n"
    " | |___  | | | | | |___  | | \\  | | |_| |   | |   | |___  | |___  | | | |      | |___  | |___  | |\r\n"
    " \\_____| |_| |_| |_____| |_|  \\_| \\_____/   |_|   |_____| \\_____| |_| |_|      \\_____| |_____| |_|");

  ui->shell_textEdit->insertPlainText(logo + "\r\nStart Cmd Tool!\r\n");

  /* 恢复配置 */
  read_cfg();
}

debug_window::~debug_window()
{
  delete ui;
}

void debug_window::timer_init(void)
{
  timer_obj = new QTimer(this);
  timer_obj->setInterval(1);
  connect(timer_obj, &QTimer::timeout, this, &debug_window::slot_time_out);
}

/**
 * @brief 窗口关闭事件
 * @param event
 */
void debug_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}


bool debug_window::eventFilter(QObject *target, QEvent *event)
{
  /* 检查是否是目标控件 */
  if(target != ui->shell_textEdit)
  {
    return QWidget::eventFilter(target, event);
  }

  /* 检测是否是输入事件 */
  if(event->type() != QEvent::KeyPress)
  {
    return QWidget::eventFilter(target, event);
  }

  QTextCursor txtcur = ui->shell_textEdit->textCursor();

  if(ui->shell_textEdit->textCursor().position() < minTextCurse)
  {
    return true;
  }

  QKeyEvent *k = static_cast<QKeyEvent *>(event);
  if(nullptr == k)
  {
    return QWidget::eventFilter(target, event);
  }

  switch(k->key())
  {
    case Qt::Key_Return:
    case Qt::Key_Enter:
      {
        txtcur.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        txtcur.setPosition(minTextCurse, QTextCursor::KeepAnchor);
        QString msg = txtcur.selectedText();
        qDebug() << "selset " << msg;

        /* 添加到历史命令列表 */
        if(history_cmd.isEmpty() || QString::compare(msg, history_cmd.back()) != 0)
        {
          history_cmd.append(msg);
        }
        if(msg.compare("clean") == 0)
        {
          ui->shell_textEdit->setText("");
          ui->shell_textEdit->setTextColor(ui->color_list_comboBox->currentText());
          ui->shell_textEdit->setHtml("<body bgcolor=\"#000000\"></body>");
          ui->shell_textEdit->append("clean");
          minTextCurse = ui->shell_textEdit->textCursor().position();
        }
        else
        {
          send_command_port(msg);
        }

        history_cmd_num = history_cmd.size();
        ui->shell_textEdit->append("");
        txtcur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
        ui->shell_textEdit->setTextCursor(txtcur);
        minTextCurse = ui->shell_textEdit->textCursor().position();
        return true;
      }

    case Qt::Key_Up:
      {
        QTextCursor tc = ui->shell_textEdit->textCursor();

        if(history_cmd.size() == 0)
        {
          return true;
        }

        tc.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
        tc.setPosition(minTextCurse,QTextCursor::KeepAnchor);
        tc.removeSelectedText();
        if(history_cmd_num == 0)
        {
          history_cmd_num = history_cmd.size();
        }
        if(history_cmd_num > history_cmd.size())
        {
          history_cmd_num = 1;
        }
        ui->shell_textEdit->insertPlainText(history_cmd[history_cmd_num - 1]);
        lastTextCurse = ui->shell_textEdit->textCursor().position();
        history_cmd_num--;
        return true;
      }

    case Qt::Key_Down:
      {
        QTextCursor tc = ui->shell_textEdit->textCursor();

        if(history_cmd.size() == 0)
        {
          return true;
        }
        tc.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
        tc.setPosition(minTextCurse,QTextCursor::KeepAnchor);
        tc.removeSelectedText();
        if(history_cmd_num == 0)
        {
          history_cmd_num = history_cmd.size();
        }
        if(history_cmd_num > history_cmd.size())
        {
          history_cmd_num = 1;
        }
        ui->shell_textEdit->insertPlainText(history_cmd[history_cmd_num-1]);
        lastTextCurse = ui->shell_textEdit->textCursor().position();
        history_cmd_num++;
        return true;
      }

    case Qt::Key_Tab:
      {
        QTextCursor txtcur = ui->shell_textEdit->textCursor();
        txtcur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        QString msg = txtcur.selectedText();
        int index = msg.lastIndexOf(' ');
        if(index == -1)
        {
          index = 0;
        }
        else
        {
          index++;
        }
        QString ins = msg.mid(index);
        const QString insertStr = find_quick_complet_cmd(ins);
        if(insertStr != "")
        {
          txtcur = ui->shell_textEdit->textCursor();
          txtcur.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor,ins.length());
          txtcur.removeSelectedText();
          ui->shell_textEdit->insertPlainText(find_quick_complet_cmd(msg.mid(msg.lastIndexOf(' ') + 1)));
        }
        return true;
      }

    case Qt::Key_Backspace:
      {
        QTextCursor tc = ui->shell_textEdit->textCursor();
        if(tc.position() <= minTextCurse)
        {
          return true;
        }
      }
      break;

    case Qt::Key_Left:
      {
        if(minTextCurse == ui->shell_textEdit->textCursor().position())
        {
          return true;
        }
      }
      break;

    default:
      break;
  }
  return QWidget::eventFilter(target, event);
}


void debug_window::save_cfg()
{
  QSettings setting("./debug_window_cfg.ini", QSettings::IniFormat);
  setting.setValue("open_case_sensitive", (int)ui->case_sensitive_checkBox->checkState());
  setting.setValue("textcolor", ui->color_list_comboBox->currentText());
  /* 快捷命令 */
  QString plaintext = ui->quick_compleat_plainTextEdit->toPlainText();
  plaintext.remove(' ');
  plaintext.replace('\n', ',');;
  setting.setValue("quick_complets_list", plaintext);
  setting.sync();
}

void debug_window::read_cfg()
{
  QFile file("./debug_window_cfg.ini");
  if(false == file.exists())
  {
    /* 添加快捷指令 */
    quick_complets.clear();
    quick_complets.append("clean");
    ui->quick_compleat_plainTextEdit->setPlainText(QString("clean\n"));
    return;
  }
  QSettings setting("./debug_window_cfg.ini", QSettings::IniFormat);
  ui->case_sensitive_checkBox->setCheckState((Qt::CheckState)setting.value("open_case_sensitive").toInt());
  ui->color_list_comboBox->setCurrentText(setting.value("textcolor").toString());
  ui->shell_textEdit->setTextColor(ui->color_list_comboBox->currentText());
  /* 快捷命令 */
  QString plaintext = setting.value("quick_complets_list").toString();
  quick_complets = plaintext.split(',');
  plaintext.replace(',', '\n');
  ui->quick_compleat_plainTextEdit->setPlainText(plaintext);
  setting.sync();
}

const QString & debug_window::find_quick_complet_cmd(const QString &str)
{
  int retindex = -1;
  /* 区分大小写 */
  Qt::CaseSensitivity case_sensitive;
  if(true == ui->case_sensitive_checkBox->isChecked())
  {
    case_sensitive = Qt::CaseSensitive;
  }
  else
  {
    case_sensitive = Qt::CaseInsensitive;
  }

  for(int i = 0; i < quick_complets.size(); i++)
  {
    if(quick_complets.value(i).indexOf(str, 0, case_sensitive) == 0)
    {
      if(retindex == -1)
      {
        retindex = i;
      }
      else
      {
        retindex = -1;
        break;
      }
    }
  }

  if(retindex == -1)
  {
    return emptyStr;
  }
  else
  {
    return quick_complets[retindex];
  }
}

void debug_window::send_command_port(QString &text)
{
  for(int i = 0; i < text.length(); i++)
  {
    if(text.at(i) == '\\' && (i + 1) < text.length())
    {
      if(text.at(i + 1) == '\\')
      {
        text.remove(i + 1, 1);
      }
      else if(text.at(i + 1) == 't')
      {
        text.replace(i, 1, '\t');
        text.remove(i + 1, 1);
      }
      else if (text.at(i + 1) == 'n')
      {
        text.replace(i, 1, '\n');
        text.remove(i + 1, 1);
      }
    }
  }
  text.append("\r\n");

  /* 发送 */
  emit signal_send_command(text);
}

void debug_window::on_addquick_compelat_pushButton_clicked()
{
  QString plaintext = ui->quick_compleat_plainTextEdit->toPlainText();
  plaintext.remove(' ');

  QStringList sliplist = plaintext.split('\n');

  /* 区分大小写 */
  Qt::CaseSensitivity case_sensitive;
  if(true == ui->case_sensitive_checkBox->isChecked())
  {
    case_sensitive = Qt::CaseSensitive;
  }
  else
  {
    case_sensitive = Qt::CaseInsensitive;
  }

  for(int i = 0; i < sliplist.size(); i++)
  {
    bool is_exist = false;
    if(sliplist[i].size() == 0)
    {
      continue;
    }
    for(int j = 0; j < quick_complets.size(); j++)
    {
      if(0 == QString::compare(sliplist[i], quick_complets.value(j), case_sensitive))
      {
        is_exist = true;
        break;
      }
    }
    if(is_exist == true)
    {
      continue;
    }
    else
    {
      quick_complets.append(sliplist[i]);
    }
  }
  /* 保存参数 */
  save_cfg();
}


void debug_window::on_del_quick_compelat_pushButton_clicked()
{
  QString selecttext = ui->quick_compleat_plainTextEdit->textCursor().selectedText() ;
  selecttext.remove(' ');
  selecttext.remove(',');
  selecttext.remove('\n');
  quick_complets.removeAt( quick_complets.indexOf(selecttext));
  ui->quick_compleat_plainTextEdit->textCursor().removeSelectedText();
}

void debug_window::on_quick_compleat_plainTextEdit_cursorPositionChanged()
{
  if(ui->shell_textEdit->textCursor().position() < minTextCurse)
  {
    ui->shell_textEdit->setReadOnly(true);
  }
  else
  {
    ui->shell_textEdit->setReadOnly(false);
  }
}

void debug_window::on_color_list_comboBox_currentTextChanged(const QString &arg1)
{
  ui->shell_textEdit->setTextColor(arg1);
  QTextCursor tc = ui->shell_textEdit->textCursor();
  tc.movePosition(QTextCursor::End);
  ui->shell_textEdit->insertPlainText("text color change :" + arg1 + "\n");
  minTextCurse = ui->shell_textEdit->textCursor().position();
}

void debug_window::slot_time_out(void)
{
  if(receiverBuff[0] != '\0')
  {
    QTextCursor tc = ui->shell_textEdit->textCursor();
    tc.movePosition(QTextCursor::End);
    ui->shell_textEdit->setTextCursor(tc);
    ui->shell_textEdit->insertPlainText( QString::fromLocal8Bit(receiverBuff));
    minTextCurse = ui->shell_textEdit->textCursor().position();
  }
  else
  {
    /* 读取数据 */
    QString str = QString::fromLocal8Bit("rec");
    QTextCursor tc = ui->shell_textEdit->textCursor();
    tc.movePosition(QTextCursor::End);
    ui->shell_textEdit->setTextCursor(tc);
    ui->shell_textEdit->insertPlainText(str);
    minTextCurse = ui->shell_textEdit->textCursor().position();
  }
  timer_obj->stop();
}

void debug_window::slot_rec_shell_data(const quint8 *data, quint32 data_len)
{
  Q_UNUSED(data_len)

  /* 读取数据 */
  QString str = QString::fromLocal8Bit((const char *)data);
  QTextCursor tc = ui->shell_textEdit->textCursor();
  tc.movePosition(QTextCursor::End);
  ui->shell_textEdit->setTextCursor(tc);
  ui->shell_textEdit->insertPlainText(str);
  minTextCurse = ui->shell_textEdit->textCursor().position();
}

void debug_window::on_case_sensitive_checkBox_stateChanged(int arg1)
{
  Q_UNUSED(arg1)
}
