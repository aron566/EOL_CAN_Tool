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
  ui->shell_textEdit->ensureCursorVisible();

  /* 恢复配置 */
  read_cfg();

  /* 显示logo */
  ui->shell_textEdit->append(logo);
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

/**
 * @brief 窗口显示事件
 * @param event
 */
void debug_window::showEvent(QShowEvent *event)
{
  Q_UNUSED(event)
  ui->shell_textEdit->setFocus();
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

  QTextEdit *text_edit_widget = ui->shell_textEdit;
  QTextCursor txtcur = text_edit_widget->textCursor();

  QKeyEvent *k = static_cast<QKeyEvent *>(event);
  if(nullptr == k)
  {
    return QWidget::eventFilter(target, event);
  }

  qDebug() << "send key" << k->key() << k->text().toLatin1();

  /* 直接发送按键 */
//  if(127 < (*k->text().toLatin1().data()))
//  {
    emit signal_send_command_char(*k->text().toLatin1().data());
    return true;
//  }

  switch(k->key())
  {
    case Qt::Key_Return:
    case Qt::Key_Enter:
      {
        text_edit_widget->moveCursor(QTextCursor::End);
        text_edit_widget->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        QString msg = text_edit_widget->textCursor().selectedText();

        /* 添加到历史命令列表 */
        if(history_cmd.isEmpty() || QString::compare(msg, history_cmd.back()) != 0)
        {
          history_cmd.append(msg);
        }
        if(msg.compare("clear") == 0)
        {
          text_edit_widget->setText("");
          text_edit_widget->setTextColor(ui->color_list_comboBox->currentText());
          text_edit_widget->setHtml("<body bgcolor=\"#000000\"></body>");
          text_edit_widget->append("clear");
          minTextCurse = text_edit_widget->textCursor().position();
        }
        else
        {
          send_command_port(msg);
        }

        history_cmd_num = history_cmd.size();
        text_edit_widget->append("");
        txtcur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
        ui->shell_textEdit->setTextCursor(txtcur);
        minTextCurse = text_edit_widget->textCursor().position();
        return true;
      }

    case Qt::Key_Up:
      {
        QTextCursor tc = text_edit_widget->textCursor();

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
        lastTextCurse = text_edit_widget->textCursor().position();
        history_cmd_num--;
        return true;
      }

    case Qt::Key_Down:
      {
        QTextCursor tc = text_edit_widget->textCursor();

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
        text_edit_widget->insertPlainText(history_cmd[history_cmd_num-1]);
        lastTextCurse = text_edit_widget->textCursor().position();
        history_cmd_num++;
        return true;
      }

    case Qt::Key_Tab:
      {
        QTextCursor txtcur = text_edit_widget->textCursor();
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
          txtcur = text_edit_widget->textCursor();
          txtcur.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, ins.length());
          txtcur.removeSelectedText();
          ui->shell_textEdit->insertPlainText(find_quick_complet_cmd(msg.mid(msg.lastIndexOf(' ') + 1)));
        }
        return true;
      }

    case Qt::Key_Backspace:
      {
        QTextCursor tc = text_edit_widget->textCursor();
        if(tc.position() <= minTextCurse)
        {
          return true;
        }
      }
      break;

    case Qt::Key_Left:
      {
        if(minTextCurse == text_edit_widget->textCursor().position())
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
    quick_complets.append("clear");
    ui->quick_compleat_plainTextEdit->setPlainText(QString("clear\n"));
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
//  emit signal_send_command(text);
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
  ui->shell_textEdit->insertPlainText("\n");
  minTextCurse = ui->shell_textEdit->textCursor().position();
}

void debug_window::slot_time_out(void)
{

}

void debug_window::rec_shell_data(const quint8 *data, quint32 data_len)
{
  Q_UNUSED(data_len)

  /* 读取数据 */
  char strbuf[512] = {0};
  memcpy(strbuf, data, data_len);

  /* 检测是否是清屏命令 */
  QString str = QString::asprintf("%s", strbuf);
  qDebug() << data_len << str;

  for(quint32 i = 0; i < data_len; i++)
  {
    switch(strbuf[i])
    {
      /* 检查是否有退格键 */
      case '\b':
        {
          ui->shell_textEdit->moveCursor(QTextCursor::End);
          ui->shell_textEdit->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
          QString msg = ui->shell_textEdit->textCursor().selectedText();
          ui->shell_textEdit->textCursor().removeSelectedText();
          msg = msg.left(msg.length() - 1);
          ui->shell_textEdit->moveCursor(QTextCursor::End);
          ui->shell_textEdit->insertPlainText(msg);
        }
        break;

      case '\r':
        {

        }
        break;

      default:
        {
          ui->shell_textEdit->moveCursor(QTextCursor::End);
          ui->shell_textEdit->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
          QString msg = ui->shell_textEdit->textCursor().selectedText();
          ui->shell_textEdit->textCursor().removeSelectedText();
          msg += QString::asprintf("%c", strbuf[i]);
          ui->shell_textEdit->moveCursor(QTextCursor::End);
          ui->shell_textEdit->insertPlainText(msg);
        }
        break;
    }

    /* 检查上一行是否是清屏 */
    ui->shell_textEdit->moveCursor(QTextCursor::End);
    ui->shell_textEdit->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    str = ui->shell_textEdit->textCursor().selectedText();
    if("\u001B[2J\u001B[1H" == str)
    {
      ui->shell_textEdit->clear();
      ui->shell_textEdit->moveCursor(QTextCursor::Start);
      ui->shell_textEdit->setHtml("<body bgcolor=\"#000000\"></body>");
      /* 设置颜色 */
      ui->shell_textEdit->setTextColor(ui->color_list_comboBox->currentText());
      ui->shell_textEdit->insertPlainText(logo);
      return;
    }
  }

  ui->shell_textEdit->moveCursor(QTextCursor::End);
  minTextCurse = ui->shell_textEdit->textCursor().position();
}

void debug_window::on_case_sensitive_checkBox_stateChanged(int arg1)
{
  Q_UNUSED(arg1)
}
