/**
 *  @file blf2asc.cpp
 *
 *  @date 2024年03月13日 11:11:54 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2024 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2024.03.13 12:11 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2024-03-13 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
#include <qfiledialog.h>
#include <qstring.h>
#include <qapplication.h>
#include <windows.h>
#include "binlog.h" /* BL    */
#include <tchar.h>  /* RTL   */
#include <qtextstream.h>
#include <QFile>
#include <QtConcurrent>
#include <QDebug>
#include <QMessageBox>
/** Private includes ---------------------------------------------------------*/
#include "blf2asc.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/

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

QStringList         Mon  = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
QStringList         Week = {"Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat"};
QMap<uint, QString> DriverErrCode{
                                  {0,   "timeout during board initialization"                          },
                                  {1,   "no events in the rx queue / no event available for dvGetEvent"},
                                  {2,   "tx queue full, tx request refused"                            },
                                  {3,   "unknown Controller-Nr."                                       },
                                  {4,   "timeout during command"                                       },
                                  {5,   "DPRAM-Overflow"                                               },
                                  {6,   "not allowed event in dvPutCommand"                            },
                                  {8,   "driver detected another hardware"                             },
                                  {9,   "parameter error in dvMeasureInit"                             },
                                  {10,  "parameter error in dvMeasureInit and dvPutCommand"            },
                                  {11,  "not (yet) implemented function in this version of driver"     },
                                  {12,  "82526: no access to imp"                                      },
                                  {14,  "last msg wasn't transferred"                                  },
                                  {100, "unknown send id (FullCAN only)"                               },
                                  {101, "rx queue overrun"                                             },
                                  {102, "chip state busoff"                                            },
                                  {103, "chip state error passive"                                     },
                                  {104, "chip state error active"                                      },
                                  {105, "rx register overrrun (BasicCan only)"                         },
                                  {106, "at bootup the firmware couldn't access the controller"        },
                                  {107, "no valid dpram address in dvBoardInit"                        },
                                  {108, "no interrupt from CANIB received"                             },
                                  {109, "wrong modulnumber"                                            },
                                  {110, "wrong pointer to source buffer"                               },
                                  {111, "address > CANIB_SRAM_SIZE"                                    },
                                  {112, "address + size > CANIB_SRAM_SIZE"                             },
                                  {113, "CAN-Nr. <> 0 && CAN-Nr. <> 1"                                 },
                                  {114, "FIFO-Entry > 16 Byte"                                         },
                                  {115, "see drvspec"                                                  },
                                  };
template <class T>
T Range(T n, T tmin, T tmax)
{
  if (n < tmin)
    return tmin;
  else if (n > tmax)
    return tmax;
  else
    return n;
}
blf2asc::blf2asc(QObject *parent)
    : QObject(parent)
{

}

blf2asc::~blf2asc()
{
}

void blf2asc::on_pushButton_sleBlf_clicked()
{
  // m_sBlfName = QFileDialog::getOpenFileName(this, "选择BLF文件", QCoreApplication::applicationDirPath(), "BLF文件(*blf *BLF)");
  if (m_sBlfName.isEmpty())
    return;
}

void blf2asc::runConveter()
{
  QByteArray dd        = m_sBlfName.toLocal8Bit();
  LPCSTR     pFileName = dd.data();
  DWORD      dwWritten;
  LONG64     sta = 0;
  QFile      file(m_sAscName);

  DWORD *pRead = new DWORD();
  *pRead       = 1;
  {
    HANDLE              hFile;
    VBLObjectHeaderBase base;
    // VBLCANMessage message;
    VBLCANMessage2_t       message2;
    VBLCANDriverErrorExt_t erromessage;
    VBLCANErrorFrameExt    errofreammessage;
    VBLCANDriverStatistic  statistic;
    VBLEnvironmentVariable variable;
    VBLEthernetFrame       ethframe;
    VBLAppText             appText;
    VBLFileStatisticsEx    statistics = {sizeof(statistics)};
    BOOL                   bSuccess;

    if (NULL == pRead)
    {
      emit sigInit(-88);
      return;
    }

    *pRead = 0;

    /* open file */
    hFile = BLCreateFile(pFileName, GENERIC_READ);

    if (INVALID_HANDLE_VALUE == hFile)
    {
      emit sigInit(-88);
      return;
    }

    BLGetFileStatisticsEx(hFile, &statistics);
    int  n  = statistics.mObjectCount / 100;
    int  mn = 0;
    emit sigInit(n);
    emit sigStatus(0);
    bSuccess = TRUE;

    bool isok = file.open(QIODevice::WriteOnly);
    if (!isok)
    {
      emit sigInit(-88);
      return;
    }
    QTextStream filestream(&file);
    /*QString data = "date " + Mon.at(statistics.mMeasurementStartTime.wDayOfWeek-1) + " "
        + Week.at(statistics.mMeasurementStartTime.wMonth-1)
        + " " + QString::number(statistics.mMeasurementStartTime.wDay)
        + " " + QString::number(statistics.mMeasurementStartTime.wHour)
        + ":" + QString::number(statistics.mMeasurementStartTime.wMinute)
        + ":" + QString::number(statistics.mMeasurementStartTime.wSecond)
        + "." + QString::number(statistics.mMeasurementStartTime.wMilliseconds)
        + "  " + QString::number(statistics.mMeasurementStartTime.wYear) + "\nbase hex  timestamps absolute\ninternal events logged\n// version N/A\n";*/

    QString data = "date " + Mon.at(Range(statistics.mMeasurementStartTime.wMonth - 1, 0, 11)) + " " + QString::number(statistics.mMeasurementStartTime.wDay) + " " + Week.at(Range((int)statistics.mMeasurementStartTime.wDayOfWeek, 0, 6)) + " " + QString::number(statistics.mMeasurementStartTime.wHour) + ":" + QString::number(statistics.mMeasurementStartTime.wMinute) + ":" + QString::number(statistics.mMeasurementStartTime.wSecond) + "." + QString::number(statistics.mMeasurementStartTime.wMilliseconds) + "  " + QString::number(statistics.mMeasurementStartTime.wYear) + "\nbase hex  timestamps absolute\ninternal events logged\n// version N/A\n";
    filestream << data;
    /* read base object header from file */
    while (RunState && BLPeekObject(hFile, &base))
    {
      switch (base.mObjectType)
      {
        case BL_OBJ_TYPE_CAN_MESSAGE:
          {
            message2.mHeader.mBase = base;
            bSuccess               = BLReadObjectSecure(hFile, &message2.mHeader.mBase, sizeof(message2));
            /* free memory for the CAN message */
            if (!message2.mDLC)
              continue;
            if (bSuccess)
            {
              uint64_t id;
              // CANoe的人在扩展帧ID上加了0x80000000的，所以要处理一下
              if (message2.mID > 0x80000000)
                id = message2.mID & 0x7FFFFFFF;
              else
                id = message2.mID;
              /*printf("%ld 0x%x %2x %2x %2x %2x %2x %2x %2x %2x\n", message.mHeader.mObjectTimeStamp, (message.mID - 0x7FFFFFFF), message.mData[0],
                message.mData[1], message.mData[2], message.mData[3], message.mData[4], message.mData[5], message.mData[6], message.mData[7]);*/
              QString dir = CAN_MSG_DIR(message2.mFlags) == 0 ? "\tRx" : "\tTx";

              QString str = QString("%1 %2  %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16 %17")
                                .arg(QString::number(message2.mHeader.mObjectTimeStamp / 1000000000.0, 'f', 6))
                                .arg(QString::number(message2.mChannel))
                                .arg("0x" + QString::number(id, 16).toUpper())
                                .arg(dir)
                                .arg("d")
                                .arg(QString::number(message2.mDLC))
                                .arg(QString::number(message2.mData[0], 16).toUpper(), 2, QLatin1Char('0'))
                                .arg(QString::number(message2.mData[1], 16).toUpper(), 2, QLatin1Char('0'))
                                .arg(QString::number(message2.mData[2], 16).toUpper(), 2, QLatin1Char('0'))
                                .arg(QString::number(message2.mData[3], 16).toUpper(), 2, QLatin1Char('0'))
                                .arg(QString::number(message2.mData[4], 16).toUpper(), 2, QLatin1Char('0'))
                                .arg(QString::number(message2.mData[5], 16).toUpper(), 2, QLatin1Char('0'))
                                .arg(QString::number(message2.mData[6], 16).toUpper(), 2, QLatin1Char('0'))
                                .arg(QString::number(message2.mData[7], 16).toUpper(), 2, QLatin1Char('0'))
                                .arg("Length = " + QString::number(message2.mFrameLength))
                                .arg("BitCount = " + QString::number(message2.mBitCount))
                                .arg("ID = " + QString::number(id) + "x") +
                            "\n";
              if (isFilter)
              {
                if (fifter.size() > 0)
                {
                  for (int i = 0; i < fifter.size(); i++)
                  {
                    if (QString::number(id, 16).toUpper() == fifter.at(i).toUpper())
                    {
                      filestream << str;
                    }
                  }
                }
              }
              else
              {
                filestream << str;
              }

              BLFreeObject(hFile, &message2.mHeader.mBase);
            }
          }
          break;
        case BL_OBJ_TYPE_CAN_STATISTIC:
          statistic.mHeader.mBase = base;
          bSuccess                = BLReadObjectSecure(hFile, &statistic.mHeader.mBase, sizeof(statistic));
          if (bSuccess)
          {
            QString str = QString("%1 %2  %3 %4 %5 %6 %7 %8 %9")
                              .arg(QString::number(statistic.mHeader.mObjectTimeStamp / 1000000000.0, 'f', 6))
                              .arg(QString::number(statistic.mChannel))
                              .arg("Statistic: D " + QString::number(statistic.mStandardDataFrames))
                              .arg("R " + QString::number(statistic.mStandardRemoteFrames))
                              .arg("XD " + QString::number(statistic.mExtendedDataFrames))
                              .arg("RD " + QString::number(statistic.mExtendedRemoteFrames))
                              .arg("E " + QString::number(statistic.mErrorFrames))
                              .arg("O " + QString::number(statistic.mOverloadFrames))
                              .arg("B " + QString::number(statistic.mBusLoad / 100.0, 'f', 2) + "%\n");
            filestream << str;
          }
          BLFreeObject(hFile, &statistic.mHeader.mBase);
          break;
        case BL_OBJ_TYPE_CAN_ERROR_EXT:
          errofreammessage.mHeader.mBase = base;
          bSuccess                       = BLReadObjectSecure(hFile, &errofreammessage.mHeader.mBase, sizeof(errofreammessage));
          if (bSuccess)
          {
            QString str = QString("%1 %2 %3 %4 Flags = 0x%5 CodeExt = 0x%6 Code = 0x%7 ID = 0x%8 Position = %9 Length = %10 Data = 0x%11 0x%12 0x%13 0x%14 0x%15 0x%16 0x%17 0x%18")
                              .arg(QString::number(errofreammessage.mHeader.mObjectTimeStamp / 1000000000.0, 'f', 6))
                              .arg(QString::number(errofreammessage.mChannel))
                              .arg("ErrorFrame")
                              .arg(QString::number(errofreammessage.mFlags, 16))   //+ "\n";
                              .arg(QString::number(errofreammessage.mFlagsExt, 16))
                              .arg(QString::number(errofreammessage.mECC, 16))
                              .arg(QString::number(errofreammessage.mID, 16))
                              .arg(QString::number(errofreammessage.mDLC, 16))
                              .arg(QString::number(errofreammessage.mPosition))
                              .arg(QString::number(errofreammessage.mLength))
                              .arg(QString::number(errofreammessage.mData[0], 16))
                              .arg(QString::number(errofreammessage.mData[1], 16))
                              .arg(QString::number(errofreammessage.mData[2], 16))
                              .arg(QString::number(errofreammessage.mData[3], 16))
                              .arg(QString::number(errofreammessage.mData[4], 16))
                              .arg(QString::number(errofreammessage.mData[5], 16))
                              .arg(QString::number(errofreammessage.mData[6], 16))
                              .arg(QString::number(errofreammessage.mData[7], 16)) +
                          "\n";
            filestream << str;
          }
          BLFreeObject(hFile, &errofreammessage.mHeader.mBase);
          break;
        case BL_OBJ_TYPE_CAN_DRIVER_ERROR:
          erromessage.mHeader.mBase = base;
          bSuccess                  = BLReadObjectSecure(hFile, &erromessage.mHeader.mBase, sizeof(erromessage));
          if (bSuccess)
          {
            QString str = QString("%1 %2 %3 %4%5")
                              .arg(QString::number(erromessage.mHeader.mObjectTimeStamp / 1000000000.0, 'f', 6))
                              .arg("CAN")
                              .arg(QString::number(erromessage.mChannel))
                              .arg("Status:")
                              .arg(DriverErrCode[erromessage.mErrorCode]) +
                          "\n";
            filestream << str;
          }
          BLFreeObject(hFile, &erromessage.mHeader.mBase);
          break;
        default:
          /* skip all other objects */
          bSuccess = BLSkipObject(hFile, &base);
          break;
      }
      if (bSuccess)
      {
        *pRead += 1;
      }
      ++sta;
      if (sta % 100 == 0 || (sta / 100) >= n)
      {
        emit sigStatus(++mn);
      }

      if (!RunState)
      {
        filestream.flush();
        file.close();
        return;
      }
    }
    filestream.flush();
    file.close();
    emit sigStatus(n);
    emit sigInit(-99);
    /* close file */
    if (!BLCloseHandle(hFile))
    {
      return;
    }

    // return bSuccess ? 0 : -1;
  }
}

void blf2asc::on_pushButton_selAsc_clicked()
{
  // m_sAscName = QFileDialog::getSaveFileName(this, "保存asc文件", QCoreApplication::applicationDirPath(), "ASC文件(*asc *asc)");
  if (m_sAscName.isEmpty())
    return;
}

void blf2asc::on_pushButton_converter_clicked()
{
  if (true)
  {
    fifter   = QString("123;456").split(";");
    isFilter = true;
  }
  else
  {
    isFilter = false;
  }
  if (m_sBlfName.isEmpty())
  {
    // QMessageBox::warning(this, "BLF文件为空", "BLF文件不能为空");
  }

  /* 执行转换 */
  QtConcurrent::run(this, &blf2asc::runConveter);
}

void blf2asc::on_recConveterState(int statue)
{

}

void blf2asc::on_recInitState(int max)
{
  if (max == -99)
  {
    QString str = "完成 ok! 转换完成";
  }
  else if (max == -88)
  {
    QString str = "未完成 failed! 转换失败";
  }
}

void blf2asc::closeEvent()
{
  RunState = false;
  Sleep(300);
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

/******************************** End of file *********************************/
