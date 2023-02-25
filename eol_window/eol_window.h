#ifndef EOL_WINDOW_H
#define EOL_WINDOW_H

#include <QWidget>
#include <QTimer>
#include <QList>
#include <QFile>
#include <eol_protocol.h>

namespace Ui {
class eol_window;
}

class eol_window : public QWidget
{
  Q_OBJECT

public:
  explicit eol_window(QString titile, QWidget *parent = nullptr);
  ~eol_window();
protected:
    /**
     * @brief closeEvent
     * @param event
     */
    virtual void closeEvent(QCloseEvent *event);
private:
  Ui::eol_window *ui;

  QTimer *timer_obj = nullptr;

private:
  quint32 time_cnt = 0;

  typedef struct
  {
    QString file_path;
    QString file_name;
    QString show_info;
    int table_type;
    int data_type;
  }TABLE_INFO_Typedef_t;
  QList <TABLE_INFO_Typedef_t>table_list;

  QString current_file_path;
  QFile current_file;
  QString current_file_name;
  quint64 current_filesize;

  QStringList num_str_list;
private:
  void timer_init();

  /**
   * @brief 重置显示
   */
  void reset_ui_info();

  /**
   * @brief csv文件解析
   * @param 文件路径
   */
  void csv_analysis(QString &file_path);
signals:
  void signal_eol_window_closed();

private slots:
  void slot_timeout();
  void slot_protocol_complete();
  void on_file_sel_pushButton_clicked();
  void on_upload_pushButton_clicked();
  void on_update_pushButton_clicked();
  void on_add_list_pushButton_clicked();
  void on_clear_list_pushButton_clicked();
};

#endif // EOL_WINDOW_H
