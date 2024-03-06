/**
 *  @file eol_sub_window.hpp
 *
 *  @date 2024年01月18日 11:12:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-01-18 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 */
#ifndef EOL_SUB_WINDOW_H
#define EOL_SUB_WINDOW_H
/** Includes -----------------------------------------------------------------*/
#include <QWidget>
/** Private includes ---------------------------------------------------------*/
#include "eol_protocol.h"
#include "eol_sub_more_window/eol_sub_more_window.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

namespace Ui {
class eol_sub_window;
}

class eol_sub_window : public QWidget
{
  Q_OBJECT

public:
  explicit eol_sub_window(QString title, QWidget *parent = nullptr);
  ~eol_sub_window();

  /**
   * @brief 设置eol协议栈对象
   * @param obj 协议栈对象
   */
  void set_eol_protocol_obj(eol_protocol *obj = nullptr);
protected:
  /**
     * @brief closeEvent
     * @param event
     */
  virtual void closeEvent(QCloseEvent *event) override;
private:

  /**
   * @brief 读写窗口下级窗口初始化
   * @param title
   */
  void rw_more_window_init(QString title);

  /**
   * @brief 更新dtc错误状态
   */
  void update_dtc_err_state();

  /**
   * @brief 清除dtc错误状态
   */
  void clear_dtc_err_state();
signals:
  /**
   * @brief 窗口关闭信号
   */
  void signal_window_closed();

private slots:
  /**
   * @brief 读写设备正常
   * @param reg 寄存器
   * @param data 读出的数据，为nullptr代表写设备成功
   * @param data_len 代表数据长度，为0时代表写设备成功
   */
  void slot_rw_device_ok(quint8 reg, const quint8 *data = nullptr, quint16 data_len = 0);

  /**
   * @brief 从机读写无反应
   * @param reg 读写寄存器地址
   * @param command 读写标识
   */
  void slot_protocol_rw_err(quint8 reg, quint8 command);
private slots:

  void on_test_pushButton_clicked();

  void on_write_pushButton_clicked();

  void on_more_pushButton_clicked();

private:
  Ui::eol_sub_window *ui;

  eol_sub_more_window *eol_sub_more_window_obj = nullptr;

private:
  eol_protocol *eol_protocol_obj = nullptr;

private:
  quint8 version_info[28];/**< 版本号 */
};

#endif // EOL_SUB_WINDOW_H
/******************************** End of file *********************************/
