#ifndef EOL_SUB_MORE_WINDOW_H
#define EOL_SUB_MORE_WINDOW_H

#include <QWidget>

namespace Ui {
class eol_sub_more_window;
}

class eol_sub_more_window : public QWidget
{
  Q_OBJECT

public:
  explicit eol_sub_more_window(QString title, QWidget *parent = nullptr);
  ~eol_sub_more_window();

  /**
   * @brief 设置dtc错误码
   * @param data 错误状态
   * @param len 数据长度
   */
  void set_dtc_err_status(const quint8 *data, quint32 len);
private:

signals:
  /**
   * @brief 窗口关闭信号
   */
  void signal_window_closed();

  /**
   * @brief 获取dtc错误状态信号
   */
  void signal_get_dtc_err_status();

  /**
   * @brief 清除dtc错误状态信号
   */
  void signal_clear_dtc_err_status();
protected:
  /**
     * @brief closeEvent
     * @param event
     */
  virtual void closeEvent(QCloseEvent *event) override;

private slots:
  void on_get_err_pushButton_clicked();

  void on_clear_err_pushButton_clicked();

private:
  Ui::eol_sub_more_window *ui;
};

#endif // EOL_SUB_MORE_WINDOW_H
