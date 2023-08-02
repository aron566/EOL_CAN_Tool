/**
  *  @file updater_window.cpp
  *
  *  @date 2023年08月01日 13:05:52 星期二
  *
  *  @author aron566
  *
  *  @copyright Copyright (c) 2022 aron566 <aron566@163.com>.
  *
  *  @brief 更新窗口.
  *
  *  @details None.
  *
  *  @version v1.0.0 aron566 2023.08.01 13:05 初始版本.
  *           v1.0.1 aron566 2023.08.02 10:28 增加启动检查版本开关，修复路径为中文错误.
  *           v1.0.2 aron566 2023.08.02 15:30 修复获取文件名异常问题强制使用连接尾部的文件名
  */
/** Includes -----------------------------------------------------------------*/
#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QDebug>
/** Private includes ---------------------------------------------------------*/
#include "updater_window.h"
#include "ui_updater_window.h"
#include "utility.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define CONFIG_VER_STR            "0.0.1"               /**< 配置文件版本 */
/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
static QString updater_url = "https://raw.githubusercontent.com/"
                             "aron566/EOL_CAN_Tool_Package/master/packge_release/"
                             "updates.json";
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

updater_window::updater_window(QString title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::updater_window)
{
  ui->setupUi(this);

  /* 设置标题 */
  this->setWindowTitle(title);

  /* Apply style sheet */
  QFile file(":/qdarkstyle/dark/style.qss");
  if(file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
      this->setStyleSheet(file.readAll());
      file.close();
  }

  /* QSimpleUpdater is single-instance */
  updater_obj = QSimpleUpdater::getInstance();

  /* Check for updates when the "Check For Updates" button is clicked */
  connect(updater_obj, &QSimpleUpdater::checkingFinished, this, &updater_window::slot_update_changelog);
  connect(updater_obj, &QSimpleUpdater::appcastDownloaded, this, &updater_window::slot_display_appcast);
  connect(updater_obj, &QSimpleUpdater::downloadFinished, this, &updater_window::slot_download_finished);

  /* React to button clicks */
  connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(resetFields()));
  connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->checkButton, SIGNAL(clicked()), this, SLOT(checkForUpdates()));

  /* Resize the dialog to fit */
  setMinimumSize(minimumSizeHint());
  resize(minimumSizeHint());

  /* Reset the UI state */
  resetFields();

  /* 读取配置 */
  read_cfg();

  /* 启动时检查更新 */
  if(Qt::Checked == ui->startup_check_en_checkBox->checkState())
  {
    checkForUpdates();
  }
}

updater_window::~updater_window()
{
  /* 保存参数 */
  save_cfg();

  delete ui;
}

void updater_window::save_cfg()
{
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  /* 启动时检查更新 */
  setting.setValue("updater_window_v" CONFIG_VER_STR "/startup_check_update", (int)ui->startup_check_en_checkBox->checkState());
  /* 下载保存路径 */
  setting.setValue("updater_window_v" CONFIG_VER_STR "/download_dir", download_dir);
  /* url 自定义应用程序更新地址 */
  setting.setValue("updater_window_v" CONFIG_VER_STR "/updater_url", updater_url);
  /* version 自定义应用程序当前版本 */
  setting.setValue("updater_window_v" CONFIG_VER_STR "/installedVersion", ui->installedVersion->text());
  /* customAppcast 自定义应用程序 */
  setting.setValue("updater_window_v" CONFIG_VER_STR "/customAppcast", (int)ui->customAppcast->checkState());
  /* enableDownloader 启用下载器 */
  setting.setValue("updater_window_v" CONFIG_VER_STR "/enableDownloader", (int)ui->enableDownloader->checkState());
  /* showAllNotifcations 显示所有的通知 */
  setting.setValue("updater_window_v" CONFIG_VER_STR "/showAllNotifcations", (int)ui->showAllNotifcations->checkState());
  /* showUpdateNotifications 显示更新通知 */
  setting.setValue("updater_window_v" CONFIG_VER_STR "/showUpdateNotifications", (int)ui->showUpdateNotifications->checkState());
  /* mandatoryUpdate 强制更新 */
  setting.setValue("updater_window_v" CONFIG_VER_STR "/mandatoryUpdate", (int)ui->mandatoryUpdate->checkState());
  setting.sync();
}

void updater_window::read_cfg()
{
  QFile file("./eol_tool_cfg.ini");
  if(false == file.exists())
  {
      return;
  }
  QSettings setting("./eol_tool_cfg.ini", QSettings::IniFormat);
  setting.setIniCodec("UTF-8");
  if(false == setting.contains("updater_window_v" CONFIG_VER_STR "/updater_url"))
  {
    qDebug() << "err updater_window config not exist";
    return;
  }
  /* 启动时检查更新 */
  ui->startup_check_en_checkBox->setCheckState((Qt::CheckState)setting.value("updater_window_v" CONFIG_VER_STR "/startup_check_update").toInt());
  download_dir = setting.value("updater_window_v" CONFIG_VER_STR "/download_dir").toString();
  updater_url = setting.value("updater_window_v" CONFIG_VER_STR "/updater_url").toString();
  ui->installedVersion->setText(setting.value("updater_window_v" CONFIG_VER_STR "/installedVersion").toString());
  /* 首次运行不信任配置文件中的版本号，进行检查版本号 */
  if(qApp->applicationVersion() != ui->installedVersion->text())
  {
    ui->installedVersion->setText(qApp->applicationVersion());
  }

  ui->customAppcast->setCheckState((Qt::CheckState)setting.value("updater_window_v" CONFIG_VER_STR "/customAppcast").toInt());
  ui->enableDownloader->setCheckState((Qt::CheckState)setting.value("updater_window_v" CONFIG_VER_STR "/enableDownloader").toInt());
  ui->showAllNotifcations->setCheckState((Qt::CheckState)setting.value("updater_window_v" CONFIG_VER_STR "/showAllNotifcations").toInt());
  ui->showUpdateNotifications->setCheckState((Qt::CheckState)setting.value("updater_window_v" CONFIG_VER_STR "/showUpdateNotifications").toInt());
  ui->mandatoryUpdate->setCheckState((Qt::CheckState)setting.value("updater_window_v" CONFIG_VER_STR "/mandatoryUpdate").toInt());
  setting.sync();
}

void updater_window::resetFields()
{
  ui->installedVersion->setText(qApp->applicationVersion());
  ui->customAppcast->setChecked(false);
  ui->enableDownloader->setChecked(true);
  ui->showAllNotifcations->setChecked(false);
  ui->showUpdateNotifications->setChecked(true);
  ui->mandatoryUpdate->setChecked(false);
}

void updater_window::checkForUpdates()
{
  /* Get settings from the UI */
  QString version = ui->installedVersion->text();
  bool customAppcast = ui->customAppcast->isChecked();
  bool downloaderEnabled = ui->enableDownloader->isChecked();
  bool notifyOnFinish = ui->showAllNotifcations->isChecked();
  bool notifyOnUpdate = ui->showUpdateNotifications->isChecked();
  bool mandatoryUpdate = ui->mandatoryUpdate->isChecked();

  /* Apply the settings */
  updater_obj->setModuleVersion(updater_url, version);
  updater_obj->setNotifyOnFinish(updater_url, notifyOnFinish);
  updater_obj->setNotifyOnUpdate(updater_url, notifyOnUpdate);
  updater_obj->setUseCustomAppcast(updater_url, customAppcast);
  updater_obj->setDownloaderEnabled(updater_url, downloaderEnabled);
  updater_obj->setMandatoryUpdate(updater_url, mandatoryUpdate);

  /* 检查下载路径 */
  if(download_dir.isEmpty() == true)
  {
    /* 选择文件存储区域 */
    /* 参数：父对象，标题，默认路径 */
    download_dir = QFileDialog::getExistingDirectory(this, tr("Choice Updater Save Dirctory"), "../");
    if(download_dir.isEmpty() == true)
    {
      return;
    }

    qDebug() << "中文UTF8编码显示路径：" << download_dir;
  }
  updater_obj->setDownloadDir(updater_url, download_dir);

  /* Check for updates */
  updater_obj->checkForUpdates(updater_url);
}

void updater_window::slot_update_changelog(const QString &url)
{
  if (url != updater_url)
  {
    return;
  }
  QString change_log = updater_obj->getChangelog(url);
  if(true == change_log.isEmpty())
  {
    ui->changelogText->setText("please check network");
    return;
  }
  else
  {
    ui->changelogText->setText(change_log);
  }
  QString lasted_version = updater_obj->getLatestVersion(updater_url);
  emit signal_lasted_version_info(lasted_version, change_log);
}

void updater_window::slot_display_appcast(const QString &url, const QByteArray &reply)
{
  if (url != updater_url)
  {
    return;
  }
  QString text = "This is the downloaded appcast: <p><pre>" + QString::fromUtf8(reply)
                 + "</pre></p><p> If you need to store more information on the "
                   "appcast (or use another format), just use the "
                   "<b>QSimpleUpdater::setCustomAppcast()</b> function. "
                   "It allows your application to interpret the appcast "
                   "using your code and not QSU's code.</p>";

  ui->changelogText->setText(text);
}

/**
 * @brief slot_download_finished
 * @param url
 * @param filepath
 */
void updater_window::slot_download_finished(const QString &url, const QString &filepath)
{
  if (url != updater_url)
  {
    return;
  }
  qDebug() << "download finished url:" << filepath << "exe:" << qApp->applicationDirPath();
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
 *
 *       Public code
 *
 ********************************************************************************
 */
/******************************** End of file *********************************/
