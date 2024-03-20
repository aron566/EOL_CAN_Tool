/**
 *  @file blf2asc.hpp
 *
 *  @date 2024年03月13日 11:12:45 星期一
 *
 *  @author aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-03-13 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 */
#ifndef BLF2ASC_H
#define BLF2ASC_H
/** Includes -----------------------------------------------------------------*/

/** Private includes ---------------------------------------------------------*/
#include <QObject>
#include <QStringList>
#include <QDebug>
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

class blf2asc : public QObject
{
  Q_OBJECT

public:
  blf2asc(QObject *parent = nullptr);
  ~blf2asc();

private:
  QString blf_name_str;
  QString asc_name_str;
  bool run_status = true;
  bool is_filter = false;
  QStringList fifter;

private:
  void runConveter();
signals:
  void sigStatus(int);
  void signal_status(int);
private slots:
  void on_pushButton_sleBlf_clicked();
  void on_pushButton_selAsc_clicked();
  void on_pushButton_converter_clicked();
  void on_recConveterState(int);
  void on_recInitState(int);

  void closeEvent();

};

#endif
/******************************** End of file *********************************/
