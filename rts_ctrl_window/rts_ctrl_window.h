/**
 *  @file rts_ctrl_window.hpp
 *
 *  @date 2023年12月04日 13:49:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief rts手动控制页面.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-12-04 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 */
#ifndef RTS_CTRL_WINDOW_H
#define RTS_CTRL_WINDOW_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
//#include <limits.h> /**< need variable max value    */
//#include <stdalign.h> /**< need alignof    */
//#include <stdarg.h> /**< need va_start    */
//#include <ctype.h> /**< need isalpha isdigit */
//#include <stdatomic.h> /**< need atomic_compare_exchange_weak */
//#include <assert.h> /**< need assert( a > b ); */
//#include <setjmp.h> /**< need jmp_buf buf setjmp(buf); longjmp(buf,1) */
/** Private includes ---------------------------------------------------------*/
#include <QWidget>
#include <QDebug>

#include "rts_protocol/rts_protocol.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

namespace Ui {
class rts_ctrl_window;
}

class rts_ctrl_window : public QWidget
{
  Q_OBJECT

public:
  explicit rts_ctrl_window(QString title, QWidget *parent = nullptr);
  ~rts_ctrl_window();

  /**
   * @brief set_rts_protocol_obj 设置
   * @param rts_protocol_
   */
  void set_rts_protocol_obj(rts_protocol *rts_protocol_)
  {
    rts_protocol_obj = rts_protocol_;
  }

protected:

  /**
   * @brief closeEvent
   * @param event
   */
  virtual void closeEvent(QCloseEvent *event);

signals:
  /**
   * @brief 发送窗口关闭信号
   */
  void signal_window_closed();

public slots:
  void on_set_pushButton_clicked();

  /**
   * @brief 协议栈无回复超时
   * @param sec 秒
   */
  void slot_protocol_timeout(quint32 sec);

  /**
   * @brief 从机返回错误消息
   * @param error_msg 错误消息
   */
  void slot_protocol_error_occur(quint8 error_msg);

  /**
   * @brief signal_protocol_rw_err 读写错误信号
   * @param cmd 命令
   */
  void slot_protocol_rw_err(QString cmd);

private:
  Ui::rts_ctrl_window *ui;

  rts_protocol *rts_protocol_obj = nullptr;
};

#endif // RTS_CTRL_WINDOW_H
/******************************** End of file *********************************/
