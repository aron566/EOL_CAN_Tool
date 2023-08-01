#ifndef UPDATER_WINDOW_H
#define UPDATER_WINDOW_H

#include <QWidget>
#include <QString>
#include "QSimpleUpdater.h"

namespace Ui {
class updater_window;
}

class updater_window : public QWidget
{
    Q_OBJECT

public:
    explicit updater_window(QString title, QWidget *parent = nullptr);
    ~updater_window();

private:
    Ui::updater_window *ui;
    QSimpleUpdater *updater_obj = nullptr;

    QString download_dir = "";/**< 文件保存路径 */
private:
    /**
   * @brief read_cfg
   */
    void read_cfg();

    /**
   * @brief save_cfg
   */
    void save_cfg();

private slots:
    /**
     * @brief slot_update_changelog
     * @param url
     */
    void slot_update_changelog(const QString &url);

    /**
     * @brief slot_display_appcast
     * @param url
     * @param reply
     */
    void slot_display_appcast(const QString &url, const QByteArray &reply);

    /**
     * @brief slot_download_finished
     * @param url
     * @param filepath
     */
    void slot_download_finished(const QString &url, const QString &filepath);

    void resetFields();

    void checkForUpdates();
};

#endif // UPDATER_WINDOW_H
