#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class debug_window;
}

class debug_window : public QWidget
{
  Q_OBJECT

public:
  explicit debug_window(QString title, QWidget *parent = nullptr);
  ~debug_window();

protected:
    /**
     * @brief closeEvent
     * @param event
     */
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
//    virtual void contextMenuEvent(QContextMenuEvent *event) override;
private:
  signals:
    /**
     * @brief 窗口关闭信号
     */
    void signal_window_closed();

    /**
     * @brief 发送命令信号
     * @param text 命令
     */
    void signal_send_command(QString text);
    void signal_send_command_char(char c);
public:

    /**
     * @brief 接收shell数据
     * @param data 数据
     * @param data_len 数据长度
     */
    void rec_shell_data(const quint8 *data, quint32 data_len);

private slots:
    void on_addquick_compelat_pushButton_clicked();

    void on_del_quick_compelat_pushButton_clicked();

    void on_quick_compleat_plainTextEdit_cursorPositionChanged();

    void on_color_list_comboBox_currentTextChanged(const QString &arg1);

private:
  Ui::debug_window *ui;

private:
  /**
   * @brief 定时器初始化
   */
  void timer_init();

  /**
   * @brief eventFilter
   * @param target
   * @param event
   * @return
   */
  bool eventFilter(QObject* target,QEvent * event) override;

  /**
   * @brief 发送命令
   * @param text 命令字符串
   */
  void send_command_port(QString &text);

  /**
   * @brief 恢复参数
   */
  void read_cfg();

  /**
   * @brief 保存参数
   */
  void save_cfg();

  /**
   * @brief 查找快捷指令
   * @param str 指令字段
   * @return 完整指令，为空未找到
   */
  const QString &find_quick_complet_cmd(const QString &str);
private slots:

  void slot_time_out(void);

  void on_case_sensitive_checkBox_stateChanged(int arg1);

private:
  const QString emptyStr = "";

  const QString logo = (
    "   ______  __                          _______               __     \r\n"
    "  |      ||  |--..-----..-----..-----.|_     _|.-----..----.|  |--. \r\n"
    "  |   ---||     ||  -__||     ||  _  |  |   |  |  -__||  __||     | \r\n"
    "  |______||__|__||_____||__|__||___  |  |___|  |_____||____||__|__| \r\n"
    "                               |_____|\r\n"
    "Build:       " __DATE__ " " __TIME__ "\r\n"
    "Version:     v0.0.1\r\n"
    "Copyright:   (c) 2023 Shenzhen Cheng-Tech Co.,Ltd.\r\n");

  QStringList quick_complets;/**< 快捷指令 */

  QStringList history_cmd;/**< 历史命令 */

  int history_cmd_num;/**< 历史命令个数 */
  int minTextCurse;
  int lastTextCurse;
  QTimer *timer_obj = nullptr;
};

#endif // DEBUG_WINDOW_H
