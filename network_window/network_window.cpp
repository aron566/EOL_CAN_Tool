/**
 *  @file network_window.cpp
 *
 *  @date 2023年11月24日 15:24:54 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief 网络调试窗口.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.11.24 15:24 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-11-24 <td>v0.0.1  <td>aron566 <td>初始版本
 *  <tr><td>2023-12-01 <td>v0.0.2  <td>aron566 <td>rts增加接收端口
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <QWheelEvent>
#include <QSettings>
/** Private includes ---------------------------------------------------------*/
#include "network_window.h"
#include "ui_network_window.h"
#include "utilities/utility.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define SHOW_MSG_SAVE_NUM_MAX     200U                    /**< 最大显示消息数 */
#define SHOW_MSG_ONE_SCORLL       (5U)                    /**< 上翻每次刷新列表数 */
#define SAVE_MSG_BUF_MAX          (1024U*512U*1U)         /**< 最大缓存消息数 */

#define SHOW_LINE_CHAR_NUM_MAX    (1024U)                 /**< 一行最大显示多少字符 */
#define SHOW_CHAR_TIMEOUT_MS_MAX  (1000U)                 /**< 最大等待无换行符时间ms */

#define CONFIG_VER_STR            "0.0.3"                 /**< 配置文件版本 */
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

network_window::network_window(QString title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::network_window)
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

  /* 启动定时器 */
  timer_init();

  /* 禁用端口输入 */
  ui->port_lineEdit->setEnabled(false);

  ui->client_plainTextEdit->setVisible(true);
  ui->server_plainTextEdit->setVisible(true);
  ui->client_plainTextEdit->setUndoRedoEnabled(false);
  ui->server_plainTextEdit->setUndoRedoEnabled(false);

  /* 限制显示行数 */
  ui->client_plainTextEdit->setReadOnly(true);
  ui->server_plainTextEdit->setReadOnly(true);
  ui->client_plainTextEdit->document()->setMaximumBlockCount(SHOW_MSG_SAVE_NUM_MAX);
  ui->server_plainTextEdit->document()->setMaximumBlockCount(SHOW_MSG_SAVE_NUM_MAX);

  /* 设置高亮器 */
  ch1_line_highlighter.setDocument(ui->client_plainTextEdit->document());
  ch2_line_highlighter.setDocument(ui->server_plainTextEdit->document());
  ch1_line_highlighter.set_text_color("Tx", Qt::red);
  ch2_line_highlighter.set_text_color("Tx", Qt::red);
  ch1_line_highlighter.set_text_color("send", Qt::red);
  ch2_line_highlighter.set_text_color("send", Qt::red);

  /* 设置提示值 */
  ui->hex_lineEdit->setPlaceholderText("05 66");
  ui->ip_lineEdit->setPlaceholderText("127.0.0.1");
  ui->display_str_id_limit_lineEdit->setPlaceholderText("255.255.255.255");

  /* 设置悬浮提示 */
  ui->display_str_id_limit_lineEdit->setToolTip(tr("255.255.255.255 show all network str msg"));

  /* 读取参数 */
  read_cfg();
}

network_window::~network_window()
{
  /* 保存参数 */
  save_cfg();

  /* 关闭 */
  for(qint32 i = 0; i < (qint32)NETWORK_DEIVCE_MAX; i++)
  {
    network_stop((NETWORK_DEVICE_Typedef_t)i);
  }

  delete ui;
}

void network_window::timer_init()
{
  timer_obj = new QTimer(this);
  timer_obj->setInterval(1);
  connect(timer_obj, &QTimer::timeout, this, &network_window::slot_timeout);
  timer_obj->start();
}

void network_window::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)

  this->hide();
  emit signal_window_closed();
}

void network_window::wheelEvent(QWheelEvent *event)
{
#if 1
  //  qDebug() << "wheel event angleDelta" << event->angleDelta().y();
  //  qDebug() << "wheel event phase" << event->phase();
  //  qDebug() << "wheel event inverted" << event->inverted();
  //  qDebug() << "wheel event pixelDelta" << event->pixelDelta().y();
  //  qDebug() << "wheel event globalPosition " << event->globalPosition().x() << event->globalPosition().y();

  int xp1, yp1, xp2, yp2;
  quint32 *pchx_scroll_cnt = nullptr;
  QList<SHOW_MSG_Typedef_t> *pList = nullptr;
  QPlainTextEdit *text_edit_widget = nullptr;
  QPlainTextEdit *text_edit_widget_temp = ui->client_plainTextEdit;
  QPoint A = QWidget::mapToGlobal(text_edit_widget_temp->pos());
  xp1 = A.x();
  yp1 = A.y();
  xp2 = text_edit_widget_temp->rect().width() + xp1;
  yp2 = text_edit_widget_temp->rect().height() + yp1;

  if(event->globalPosition().x() >= (qreal)xp1 &&
      event->globalPosition().x() <= (qreal)xp2 &&
      event->globalPosition().y() >= (qreal)yp1 &&
      event->globalPosition().y() <= (qreal)yp2)
  {
    //    qDebug() << "ch1";
    text_edit_widget = ui->client_plainTextEdit;
    pchx_scroll_cnt = &ch1_scroll_cnt;
    pList = &ch1_show_msg_list;
  }

  text_edit_widget_temp = ui->server_plainTextEdit;
  A = QWidget::mapToGlobal(text_edit_widget_temp->pos());
  xp1 = A.x();
  yp1 = A.y();
  xp2 = text_edit_widget_temp->rect().width() + xp1;
  yp2 = text_edit_widget_temp->rect().height() + yp1;

  if(event->globalPosition().x() >= (qreal)xp1 &&
      event->globalPosition().x() <= (qreal)xp2 &&
      event->globalPosition().y() >= (qreal)yp1 &&
      event->globalPosition().y() <= (qreal)yp2)
  {
    //    qDebug() << "ch2";
    text_edit_widget = ui->server_plainTextEdit;
    pchx_scroll_cnt = &ch2_scroll_cnt;
    pList = &ch2_show_msg_list;
  }

  if(nullptr == text_edit_widget)
  {
    return;
  }

  for(quint32 i = 0; i < SHOW_MSG_ONE_SCORLL; i++)
  {
    /* 向下滚动 */
    if(event->angleDelta().y() < 0)
    {
      /* 是否能下翻 */
      if((*pchx_scroll_cnt) + 1 > (quint32)pList->size())
      {
        //        qDebug() << " down " << (*pchx_scroll_cnt) << pList->size();
        return;
      }
      *pchx_scroll_cnt = (*pchx_scroll_cnt) + 1;
      quint32 bottom_index = (*pchx_scroll_cnt) - 1;

      /* 刷新当前消息到尾部 */
      update_show_msg(text_edit_widget, pList, bottom_index, true);
    }

    /* 向上滚动 */
    if(event->angleDelta().y() > 0)
    {
      quint32 top_index = 0;
      /* 找到顶部索引 */
      if((*pchx_scroll_cnt) >= SHOW_MSG_SAVE_NUM_MAX)
      {
        top_index = (*pchx_scroll_cnt) - SHOW_MSG_SAVE_NUM_MAX;
      }
      else
      {
        //        qDebug() << " up " << (*pchx_scroll_cnt) << SHOW_MSG_SAVE_NUM_MAX;
        return;
      }

      /* 是否能上翻 */
      if(top_index > 0)
      {
        top_index--;
        *pchx_scroll_cnt = (*pchx_scroll_cnt) - 1;
      }
      else
      {
        //        qDebug() << " up top_index" << top_index;
        return;
      }

      /* 刷新当前消息到尾部 */
      update_show_msg(text_edit_widget, pList, top_index, false);
    }
  }

#endif
  QWidget::wheelEvent(event);
}

void network_window::save_cfg()
{
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  /* addr */
  setting.setValue("network_window_v" CONFIG_VER_STR "/ip_addr", ui->ip_lineEdit->text());
  /* 端口 */
  setting.setValue("network_window_v" CONFIG_VER_STR "/port", ui->port_lineEdit->text());
  /* 数据 */
  QString plaintext = ui->hex_lineEdit->text();
  plaintext.replace(' ', ',');;
  setting.setValue("network_window_v" CONFIG_VER_STR "/hexdata_edit", plaintext);
  setting.setValue("network_window_v" CONFIG_VER_STR "/strdata_edit", ui->str_lineEdit->text());
  setting.sync();
}

void network_window::read_cfg()
{
  QFile file("./eol_tool_cfg.ini");
  if(false == file.exists())
  {
    return;
  }
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  if(false == setting.contains("network_window_v" CONFIG_VER_STR "/ip_addr"))
  {
    qDebug() << "err network_window config not exist";
    return;
  }

  /* addr */
  ui->ip_lineEdit->setText(setting.value("network_window_v" CONFIG_VER_STR "/ip_addr").toString());
  /* 端口 */
  ui->port_lineEdit->setText(setting.value("network_window_v" CONFIG_VER_STR "/port").toString());
  /* hex数据 */
  QString plaintext = setting.value("network_window_v" CONFIG_VER_STR "/hexdata_edit").toString();
  QString data_str = plaintext.replace(',', ' ');
  ui->hex_lineEdit->setText(data_str);
  /* str数据 */
  plaintext = setting.value("network_window_v" CONFIG_VER_STR "/strdata_edit").toString();
  ui->str_lineEdit->setText(plaintext);
  setting.sync();
}

void network_window::update_show_msg(QPlainTextEdit *text_edit_widget, QList<SHOW_MSG_Typedef_t> *pList, quint32 show_index, bool downward_flag)
{
  SHOW_MSG_Typedef_t show_messagex = pList->value(show_index);

  /* 下翻 */
  if(downward_flag)
  {
    /* 插入到底部，行限制下自动删行 */
    text_edit_widget->appendPlainText(show_messagex.str);

    // text_edit_widget->moveCursor(QTextCursor::End);
    // QTextCursor cursor = text_edit_widget->textCursor();
    // cursor.insertText(show_messagex.str + '\n');
    // text_edit_widget->setTextCursor(cursor);

    // text_edit_widget->moveCursor(QTextCursor::Start);
    // text_edit_widget->moveCursor(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    // text_edit_widget->moveCursor(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    // //    qDebug() << "sel1 :" << text_edit_widget->textCursor().selectedText();
    // text_edit_widget->textCursor().removeSelectedText();
    // text_edit_widget->moveCursor(QTextCursor::End);
  }
  /* 上翻 */
  else
  {
    /* 删除尾部行数据 */
    text_edit_widget->moveCursor(QTextCursor::End);
    text_edit_widget->moveCursor(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor);
    text_edit_widget->moveCursor(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    //    qDebug() << "sel2 :" << text_edit_widget->textCursor().selectedText();
    text_edit_widget->textCursor().removeSelectedText();

    /* 回到头部显示，并插入数据 */
    text_edit_widget->moveCursor(QTextCursor::Start);
    QTextCursor cursor = text_edit_widget->textCursor();
    cursor.insertText(show_messagex.str + '\n');
    text_edit_widget->setTextCursor(cursor);
  }
}

bool network_window::ch1_show_msg_is_empty()
{
  if(ch1_add_msg_index == ch1_show_msg_index)
  {
    return true;
  }
  return false;
}

bool network_window::ch2_show_msg_is_empty()
{
  if(ch2_add_msg_index == ch2_show_msg_index)
  {
    return true;
  }
  return false;
}

quint32 network_window::get_show_index(quint32 current_show_index, quint32 totaol_size)
{
  quint32 index = 0;
  quint32 remain = totaol_size - current_show_index;
  if(SAVE_MSG_BUF_MAX <= current_show_index)
  {
    index = SAVE_MSG_BUF_MAX - remain;
    return index;
  }
  index = current_show_index % SAVE_MSG_BUF_MAX;
  return index;
}

void network_window::show_txt()
{
  /* 显示 */
  QPlainTextEdit *text_edit_widget = nullptr;
  SHOW_MSG_Typedef_t show_messagex;

  if(ch1_show_msg_is_empty() == false)
  {
    quint32 show_index = get_show_index(ch1_show_msg_index, ch1_add_msg_index);
    show_messagex = ch1_show_msg_list.value(show_index);
    ch1_show_msg_index++;
    ch1_scroll_cnt = show_index + 1;

    text_edit_widget = ui->client_plainTextEdit;
    text_edit_widget->appendPlainText(show_messagex.str);
  }

  if(ch2_show_msg_is_empty() == false)
  {
    quint32 show_index = get_show_index(ch2_show_msg_index, ch2_add_msg_index);
    show_messagex = ch2_show_msg_list.value(show_index);
    ch2_show_msg_index++;
    ch2_scroll_cnt = show_index + 1;

    text_edit_widget = ui->server_plainTextEdit;
    text_edit_widget->appendPlainText(show_messagex.str);
  }
}

bool network_window::char2str(const quint8 *data, quint32 data_len, QString &msg)
{
  if(0U == data_len)
  {
    return false;
  }
  char str_buf[65] = {0};
  size_t size = data_len > 64U ? 64U : data_len;

  /* 剔除无效字符 */
  for(size_t i = 0; i < size; i++)
  {
    if(data[i] == '\0')
    {
      size = i;
      break;
    }
  }

  /* 查看字符的尾部是否是换行符 */
  if(data[size - 1U] == '\r' || data[size - 1U] == '\n')
  {
    memcpy(str_buf, data, size);
    /* 去除尾部换行 */
    if(str_buf[size - 1U] == '\r' || str_buf[size - 1U] == '\n')
    {
      str_buf[size - 1U] = '\0';
    }
    if(str_buf[size - 2U] == '\r' || str_buf[size - 2U] == '\n')
    {
      str_buf[size - 2U] = '\0';
    }
    QString str = QString::asprintf("%s", str_buf);
    if(show_line_str.isEmpty() == false)
    {
      show_line_str.append(str);
      msg.append(show_line_str);
      //      qDebug() << "尾部有换行拼接" << a << ":" << show_line_str;
      show_line_str.clear();
    }
    else
    {
      //      qDebug() << "尾部有换行" << a << ":" << str;
      msg.append(str);
    }
    return true;
  }
  /* 找到最后一个\r\n的index */
  else
  {
    size_t index = 0U;
    for(size_t i = 0; i < size; i++)
    {
      if('\r' == data[i] || '\n' == data[i])
      {
        index = i;
      }
    }
    /* 找不到，无换行符 */
    if(0U == index)
    {
      memcpy(str_buf, data, size);
      QString str = QString::asprintf("%s", str_buf);
      show_line_str.append(str);
      last_show_line_str_time_ms = current_show_line_str_time_ms;
      if(true == show_line_str_force)
      {
        show_line_str_force = false;
        msg.append(show_line_str);
        //        qDebug() << "无换行符强制" << a << ":" << show_line_str;
        show_line_str.clear();
        return true;
      }
      return false;
    }
    /* 找到，中间有换行符 */
    else
    {
      /* 输出时换行符 */
      memcpy(str_buf, data, index);
      if(str_buf[index - 1U] == '\r' || str_buf[index - 1U] == '\n')
      {
        str_buf[index - 1U] = '\0';
      }
      QString str = QString::asprintf("%s", str_buf);
      if(show_line_str.isEmpty() == false)
      {
        show_line_str.append(str);
        msg.append(show_line_str);
        //        qDebug() << "中间有换行符拼接" << a << ":" << show_line_str;
        show_line_str.clear();
      }
      else
      {
        //        qDebug() << "中间有换行符" << a << ":" << str;
        msg.append(str);
      }
      /* 剩余部分加到下一行显示 */
      memset(str_buf, 0, sizeof(str_buf));
      memcpy(str_buf, &data[index + 1U], data_len - index - 1U);
      str = QString::asprintf("%s", str_buf);
      //      qDebug() << "中间有换行符，剩余部分" << a << ":" << str;
      show_line_str.append(str);
      last_show_line_str_time_ms = current_show_line_str_time_ms;
    }
  }
  return true;
}

qint32 network_window::get_device_obj(NETWORK_DEVICE_Typedef_t device_type)
{
  for(qint32 i = 0; i < network_device_list.size(); i++)
  {
    if(network_device_list.value(i).device_type == device_type)
    {
      return i;
    }
  }
  return -1;
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

void network_window::set_network_par(const QString &ip, const QString &port,
                                     network_driver_model::NETWORK_WORK_ROLE_Typedef_t role,
                                     network_driver_model::NETWORK_TYPE_Typedef_t net_type,
                                     NETWORK_DEVICE_Typedef_t device_type, const QString &_port)
{
  NETWORK_DEVICE_INFO_Typedef_t network_info;
  network_info.ip = ip;
  network_info.port = port;
  network_info._port = _port;
  network_info.role = role;
  network_info.net_type = net_type;
  network_info.device_type = device_type;
  network_info.network_driver_obj = nullptr;
  network_info.network_driver_rec_obj = nullptr;

  /* 查重 */
  NETWORK_DEVICE_INFO_Typedef_t t;

  /* 显示 */
  for(qint32 i = 0; i < network_device_list.size(); i++)
  {
    t = network_device_list.value(i);
    qDebug() << "[" << i << "]" << ip << port << role << net_type << device_type;
  }

  for(qint32 i = 0; i < network_device_list.size(); i++)
  {
    t = network_device_list.value(i);
    if(t.device_type == device_type)
    {
      /* 先关闭网络 */
      if(false == network_stop(t.device_type))
      {
        qDebug() << "switch network failed";
        return;
      }

      /* 修改配置 */
      network_device_list.replace(i, network_info);
      return;
    }
  }
  network_device_list.append(network_info);
}

bool network_window::network_start(NETWORK_DEVICE_Typedef_t device_type)
{
  qint32 index = get_device_obj(device_type);
  if(-1 == index)
  {
    return false;
  }

  NETWORK_DEVICE_INFO_Typedef_t t = network_device_list.value(index);

  /* udp */
  if(t.net_type == network_driver_model::NETWORK_UDP_TYPE)
  {
    if(nullptr == t.network_driver_obj)
    {
      t.network_driver_obj = new network_driver_udp(this);
    }
    /* rts需要设置作为服务器接收端口 */
    if(RTS_NETWORK_DEVICE == device_type)
    {
      if(nullptr == t.network_driver_rec_obj)
      {
        t.network_driver_rec_obj = new network_driver_udp(this);
      }
      connect(t.network_driver_rec_obj, &network_driver_model::signal_show_message, this, &network_window::slot_show_message);
      connect(t.network_driver_rec_obj, &network_driver_model::signal_show_thread_message, this, &network_window::slot_show_message_block, Qt::BlockingQueuedConnection);
      connect(t.network_driver_rec_obj, &network_driver_model::signal_show_message_bytes, this, &network_window::slot_show_message_bytes);
      QString rec_ip_str = "0.0.0.0";
      QString rec_port_str = QString::number(t._port.toInt());
      if(false == t.network_driver_rec_obj->network_init(rec_ip_str, rec_port_str, network_driver_model::NETWORK_SERVER_ROLE, t.net_type))
      {
        return false;
      }
      if(false == t.network_driver_rec_obj->network_start())
      {
        return false;
      }
    }
  }
  /* tcp */
  else if(t.net_type == network_driver_model::NETWORK_TCP_TYPE)
  {
    if(nullptr == t.network_driver_obj)
    {
      t.network_driver_obj = new network_driver_tcp(this);
    }
  }
  else
  {
    /* to do nothing */
    return false;
  }
  /* 修改配置 */
  network_device_list.replace(index, t);
  connect(t.network_driver_obj, &network_driver_model::signal_show_message, this, &network_window::slot_show_message);
  connect(t.network_driver_obj, &network_driver_model::signal_show_thread_message, this, &network_window::slot_show_message_block, Qt::BlockingQueuedConnection);
  connect(t.network_driver_obj, &network_driver_model::signal_show_message_bytes, this, &network_window::slot_show_message_bytes);
  if(false == t.network_driver_obj->network_init(t.ip, t.port, t.role, t.net_type))
  {
    return false;
  }
  if(false == t.network_driver_obj->network_start())
  {
    return false;
  }

  /* 发送启动信号 */
  emit signal_network_start(device_type);
  return true;
}

bool network_window::network_stop(NETWORK_DEVICE_Typedef_t device_type)
{
  qint32 index = get_device_obj(device_type);
  if(-1 == index)
  {
    return false;
  }

  NETWORK_DEVICE_INFO_Typedef_t t = network_device_list.value(index);

  if(nullptr == t.network_driver_obj)
  {
    return false;
  }

  /* RTS需关闭服务器接收端口 */
  if(RTS_NETWORK_DEVICE == device_type)
  {
    if(nullptr != t.network_driver_rec_obj)
    {
      t.network_driver_rec_obj->network_stop();
    }
  }

  /* 发送停止信号 */
  emit signal_network_stop(device_type);
  return t.network_driver_obj->network_stop();
}

void network_window::slot_show_message(const QString &message, quint32 channel_num, \
                                    quint8 direct, const quint8 *data, quint32 data_len, QString ip)
{
  QString show_message;

  /* 时间戳 */
  if(ui->display_time_stamp_checkBox->isChecked())
  {
    QDateTime dt = QDateTime::currentDateTime();
    show_message += dt.toString("hh:mm:ss.zzz");
  }

  /* 显示字符 */
  if(ui->display_str_checkBox->isChecked())
  {
    if(((quint32)ui->display_ch_comboBox->currentIndex() == channel_num || ui->display_ch_comboBox->currentText() == "ALL")
        && 1U == direct
        && 0U < data_len)
    {
      /* ip限制 */
      if(ui->display_str_id_limit_lineEdit->text() != "255.255.255.255"
          && ui->display_str_id_limit_lineEdit->text() != ip)
      {
        return;
      }
      /* 是否需要转发图表 */
      if(ui->net_wave_checkBox->isChecked())
      {
        QByteArray a = QByteArray((const char *)data, data_len);
        emit signal_net_wave_msg(a);
      }
      if(false == char2str(data, data_len, show_message))
      {
        return;
      }
    }
    else
    {
      show_message.append(message);
    }
  }
  else
  {
    show_message.append(message);
  }

  SHOW_MSG_Typedef_t msg;
  msg.channel_num = channel_num;
  msg.str = show_message;
  msg.direct = direct;

  /* 限制缓冲区大小 */
  if(0 == channel_num)
  {
    if((quint32)ch1_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
    {
      ch1_show_msg_list.removeFirst();
    }
    ch1_show_msg_list.append(msg);
    ch1_add_msg_index++;
    goto __show_msg;
  }

  if((quint32)ch2_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
  {
    ch2_show_msg_list.removeFirst();
  }
  ch2_show_msg_list.append(msg);
  ch2_add_msg_index++;

__show_msg:

  /* 显示 */
  show_txt();
}

void network_window::slot_show_message_block(const QString &message, quint32 channel_num, quint8 direct, const quint8 *data, quint32 data_len, QString ip)
{
  /* 线程刷新显示 */
  QString show_message;

  /* 时间戳 */
  if(ui->display_time_stamp_checkBox->isChecked())
  {
    QDateTime dt = QDateTime::currentDateTime();
    show_message += dt.toString("hh:mm:ss.zzz");
  }

  /* 显示字符 */
  if(ui->display_str_checkBox->isChecked())
  {
    if(((quint32)ui->display_ch_comboBox->currentIndex() == channel_num || ui->display_ch_comboBox->currentText() == "ALL")
        && 1U == direct
        && 0U < data_len)
    {
      /* ip限制 */
      if(ui->display_str_id_limit_lineEdit->text() != "255.255.255.255"
          && ui->display_str_id_limit_lineEdit->text() != ip)
      {
        return;
      }
      if(false == char2str(data, data_len, show_message))
      {
        return;
      }
    }
    else
    {
      show_message.append(message);
    }
  }
  else
  {
    show_message.append(message);
  }

  SHOW_MSG_Typedef_t msg;
  msg.channel_num = channel_num;
  msg.str = show_message;
  msg.direct = direct;

  /* 限制缓冲区大小 */
  if(0 == channel_num)
  {
    if((quint32)ch1_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
    {
      ch1_show_msg_list.removeFirst();
    }
    ch1_show_msg_list.append(msg);
    ch1_add_msg_index++;

    goto __show_msg;
  }

  if((quint32)ch2_show_msg_list.size() >= SAVE_MSG_BUF_MAX)
  {
    ch2_show_msg_list.removeFirst();
  }
  ch2_show_msg_list.append(msg);
  ch2_add_msg_index++;

__show_msg:

  /* 显示 */
  show_txt();
}

void network_window::slot_timeout()
{
  /* 定时发送 */
  if(ui->timer_checkBox->isChecked())
  {
    on_send_pushButton_clicked();
  }

  show_line_str_force = true;/* 强制输出字符 */
  current_show_line_str_time_ms++;
  /* 检查没有换行符的字符显示 */
  if((current_show_line_str_time_ms - last_show_line_str_time_ms) > SHOW_CHAR_TIMEOUT_MS_MAX || (quint32)show_line_str.size() > SHOW_LINE_CHAR_NUM_MAX)
  {
    show_line_str_force = true;
    last_show_line_str_time_ms = current_show_line_str_time_ms;
  }
}

void network_window::slot_show_message_bytes(quint32 bytes, quint32 channel_num, quint8 direct)
{
  Q_UNUSED(channel_num)
  if(1U == direct)
  {
    rx_frame_cnt++;
    ui->rx_frame_num_label->setNum((int)rx_frame_cnt);
    rx_byte_cnt += bytes;
    ui->rx_byte_num_label->setNum((int)rx_byte_cnt);
  }

  if(0U == direct)
  {
    tx_frame_cnt++;
    ui->tx_frame_num_label->setNum((int)tx_frame_cnt);
    tx_byte_cnt += bytes;
    ui->tx_byte_num_label->setNum((int)tx_byte_cnt);
  }
}

void network_window::on_hex_lineEdit_editingFinished()
{

}

void network_window::on_str_lineEdit_editingFinished()
{

}

void network_window::on_period_lineEdit_editingFinished()
{
  timer_obj->setInterval(ui->period_lineEdit->text().toUInt() == 0 ? 1 : ui->period_lineEdit->text().toUInt());
}

void network_window::on_send_pushButton_clicked()
{
  /* 地址 ip_lineEdit 协议 net_protocol_comboBox role_comboBox */
  /* 发送类型 */
  qint32 send_type = ui->send_type_comboBox->currentIndex();
  network_driver_model::NETWORK_WORK_ROLE_Typedef_t role = (network_driver_model::NETWORK_WORK_ROLE_Typedef_t)ui->role_comboBox->currentIndex();
  network_driver_model::NETWORK_TYPE_Typedef_t net_type = (network_driver_model::NETWORK_TYPE_Typedef_t)ui->net_type_comboBox->currentIndex();
  NETWORK_DEVICE_Typedef_t device_type = (NETWORK_DEVICE_Typedef_t)ui->device_comboBox->currentIndex();
  qint32 index = get_device_obj(device_type);
  if(-1 == index)
  {
    return;
  }

  NETWORK_DEVICE_INFO_Typedef_t t = network_device_list.value(index);
  network_driver_model *pobj = t.network_driver_obj;

  if(nullptr == pobj)
  {
    return;
  }
  switch(send_type)
  {
    /* hex */
    case 0:
      {
        QString data = utility::line_data2split(ui->hex_lineEdit->text());
        QStringList data_list = data.split(' ');
        quint32 data_size = (quint32)data_list.length();
        quint8 *pdata = new quint8[data_size];
        bool ok;
        for(quint32 i = 0; i < data_size; i++)
        {
          pdata[i] = data_list.value(i).toUShort(&ok, 16);
        }
        pobj->network_send_data(pdata, data_size, ui->ip_lineEdit->text(),
                                ui->port_lineEdit->text(), role, net_type);
        delete[] pdata;
      }
      break;

    /* str */
    case 1:
      {
        pobj->network_send_data((const quint8 *)ui->str_lineEdit->text().toUtf8().data(),
                                (quint32)ui->str_lineEdit->text().size(),
                                ui->ip_lineEdit->text(),
                                ui->port_lineEdit->text(), role, net_type);
      }
      break;

    default:
      break;
  }
}

void network_window::on_clear_pushButton_clicked()
{
  ui->server_plainTextEdit->clear();
  ui->client_plainTextEdit->clear();
  rx_frame_cnt = 0;
  tx_frame_cnt = 0;
  rx_byte_cnt = 0;
  tx_byte_cnt = 0;
  ui->tx_frame_num_label->setNum(0);
  ui->rx_frame_num_label->setNum(0);
  ui->tx_byte_num_label->setNum(0);
  ui->rx_byte_num_label->setNum(0);
}

void network_window::on_export_txt_pushButton_clicked()
{
  /* 导出 */

  /* 设置导出路径 */
  /* 选择文件存储区域 */
  /* 参数：父对象，标题，默认路径，格式 */
  QString path = QFileDialog::getSaveFileName(this, tr("Save  "), "../", tr("TXT (*.txt)"));
  if(path.isEmpty() == true)
  {
    return;
  }

  QFile export_txt_file;
  /* 先关闭 */
  if(export_txt_file.isOpen() == true)
  {
    export_txt_file.close();
  }

  /* 关联文件名 */
  export_txt_file.setFileName(path);

  /* 打开文件，只写方式 */
  if(export_txt_file.open(QIODevice::WriteOnly) == false)
  {
    return;
  }

  QString txt;
  SHOW_MSG_Typedef_t msg;
  QList<SHOW_MSG_Typedef_t> *pmsg_list;

  /* 轮询显示通道 */
  for(int i = 0; i < ui->display_ch_comboBox->count() - 1; i++)
  {
    if(0 == i)
    {
      pmsg_list = &ch1_show_msg_list;
    }
    else
    {
      pmsg_list = &ch2_show_msg_list;
    }
    txt.clear();
    txt.append(QString::asprintf("====CH %d Message====\r\n", i));
    export_txt_file.write(txt.toUtf8());
    for(qint32 line = 0; line < pmsg_list->size(); line++)
    {
      export_txt_file.write(pmsg_list->value(line).str.toUtf8() + "\r\n");
    }
  }

  export_txt_file.close();
}


void network_window::on_crc_pushButton_clicked()
{
  if(true == ui->hex_lineEdit->text().isEmpty())
  {
    return;
  }
  if(0 == ui->crc_comboBox->currentIndex())
  {
    return;
  }

  bool ok;
  quint8 temp_data[2048];

  QString str = utility::line_data2split(ui->hex_lineEdit->text());
  QRegExp split_rx("\\s+");
  QStringList data_list = str.split(split_rx, Qt::SkipEmptyParts);

  quint32 len = (quint32)data_list.length() > sizeof(temp_data) ? sizeof(temp_data) : (quint32)data_list.length();

  for(quint32 i = 0; i < len; i++)
  {
    temp_data[i] = (quint8)data_list[i].toUShort(&ok, 16);
  }

  switch(ui->crc_comboBox->currentIndex())
  {
    case 1:
      {
        quint16 crc = utility::get_modbus_crc16_with_tab(temp_data, len);
        data_list.append(QString::asprintf("%02X", crc & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 8) & 0xFF));
        ui->hex_lineEdit->setText(data_list.join(" "));
      }
      break;

    case 2:
      {
        quint32 crc = utility::get_crc32_with_tab(temp_data, len);
        data_list.append(QString::asprintf("%02X", crc & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 8) & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 16) & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 24) & 0xFF));
        ui->hex_lineEdit->setText(data_list.join(" "));
      }
      break;

    case 3:
      {
        quint8 crc = utility::get_data_sum(temp_data, len);
        data_list.append(QString::asprintf("%02X", crc & 0xFF));
        ui->hex_lineEdit->setText(data_list.join(" "));
      }
      break;

    case 4:
      {
        quint32 crc = utility::get_crc32_with_tab1(temp_data, len);
        data_list.append(QString::asprintf("%02X", crc & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 8) & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 16) & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 24) & 0xFF));
        ui->hex_lineEdit->setText(data_list.join(" "));
      }
      break;

    case 5:
      {
        quint32 crc = utility::get_crc32_with_tab2(temp_data, len);
        data_list.append(QString::asprintf("%02X", crc & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 8) & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 16) & 0xFF));
        data_list.append(QString::asprintf("%02X", (crc >> 24) & 0xFF));
        ui->hex_lineEdit->setText(data_list.join(" "));
      }
      break;

    default:
      break;
  }
}

void network_window::on_role_comboBox_currentIndexChanged(int index)
{
  switch (index)
  {
    /* 服务器 */
    case 0:
      {
        ui->port_lineEdit->setEnabled(false);
        ui->ip_lineEdit->setEnabled(true);
      }
      break;

    /* 客户端 */
    case 1:
      {
        if(ui->net_type_comboBox->currentText() == "TCP")
        {
          ui->ip_lineEdit->setEnabled(false);
          ui->port_lineEdit->setEnabled(false);
        }
        else
        {
          ui->port_lineEdit->setEnabled(true);
        }
      }
      break;
    default:
      break;
  }
}

void network_window::on_net_type_comboBox_currentIndexChanged(int index)
{
  Q_UNUSED(index)
  on_role_comboBox_currentIndexChanged(ui->role_comboBox->currentIndex());
}

/******************************** End of file *********************************/
