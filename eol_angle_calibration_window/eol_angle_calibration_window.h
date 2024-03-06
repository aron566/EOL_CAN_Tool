/**
 *  @file eol_angle_calibration_window.hpp
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
#ifndef EOL_ANGLE_CALIBRATION_WINDOW_H
#define EOL_ANGLE_CALIBRATION_WINDOW_H
/** Includes -----------------------------------------------------------------*/
#include <QWidget>
#include <QList>
#include <QTimer>
#include <QFile>
/** Private includes ---------------------------------------------------------*/
#include "eol_protocol.h"
/** Private defines ----------------------------------------------------------*/
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

namespace Ui {
class eol_angle_calibration_window;
}

class eol_angle_calibration_window : public QWidget
{
  Q_OBJECT

public:
  explicit eol_angle_calibration_window(QString title, QWidget *parent = nullptr);
  ~eol_angle_calibration_window();

  /**
   * @brief 设置eol协议栈对象
   * @param obj 协议栈对象
   */
  void set_eol_protocol_obj(eol_protocol *obj = nullptr);

public slots:
  /**
   * @brief 添加配置文件信息到链表
   * @param info
   */
  void slot_profile_info_update(eol_protocol::CALIBRATION_PROFILE_INFO_Typedef_t &info)
  {
    calibration_profile_info_list.append(info);
  }

  /**
   * @brief 清除配置信息
   */
  void slot_clear_profile_info()
  {
    calibration_profile_info_list.clear();
  }
protected:
  /**
     * @brief closeEvent
     * @param event
     */
  virtual void closeEvent(QCloseEvent *event) override;

private:

  /**
   * @brief 定时器初始化
   */
  void timer_init();

  /**
   * @brief 更新2DFFT数据
   * @param data 数据
   * @param size 数据长度
   */
  bool update_2dfft_result(const quint8 *data, quint16 size);

  /**
   * @brief 导出2dfft csv文件
   */
  void export_2dfft_csv_file();

  /**
   * @brief 获取特定配置下的通道数
   * @param profile_id 配置ID
   * @return 通道数
   */
  quint8 get_profile_channel_num(quint8 profile_id);
signals:
  /**
   * @brief 窗口关闭信号
   */
  void signal_window_closed();

private slots:
  /**
   * @brief 接收数据
   * @param reg_addr 寄存器地址
   * @param data 数据地址
   * @param data_size 数据长度
   */
  void slot_rw_device_ok(quint8 reg_addr, const quint8 *data, quint16 data_size);

  /**
   * @brief 从机读写无反应信号
   * @param reg 读写寄存器地址
   * @param command 读写标识
   */
  void slot_protocol_rw_err(quint8 reg, quint8 command);

  /**
   * @brief 定时器超时
   */
  void slot_timeout();
  void on_add_config_pushButton_clicked();

  void on_clear_config_pushButton_clicked();

  void on_start_pushButton_clicked();

private:
  Ui::eol_angle_calibration_window *ui;

private:
  QTimer *timer_obj = nullptr;
  eol_protocol *eol_protocol_obj = nullptr;

private:
  typedef enum
  {
    AZI_DIRECTION = 0,  /**< 方位方向 */
    ELE_DIRECTION,      /**< 俯仰方向 */
  }DIRECTION_Typedef_t;

  typedef struct
  {
    qint32 real;          /**< 实部*1024 */
    qint32 image;         /**< 虚部*1024 */
  }COMPLEX_INT32_Typedef_t;

  /* 转台信息 */
  typedef struct
  {
    quint8 profile_id[4]; /**< 使用的配置ID列表 */
    quint8 channel_num[4];/**< 配置列表下通道数，配置列表下：代表下标不代表profile id号 */
    quint8 profile_num;   /**< 配置数目 */
    quint8 direction;     /**< @ref FFT_REQUEST_CONDITION_Typedef_t */
    float s_angle;        /**< 起始角度 */
    float e_angle;        /**< 结束角度 */
    float step_angle;     /**< 步进角度 */
    float current_angle;  /**< 当前角度 */
    float azi_ele_angle;  /**< 当前辅助角度 */
    float rts_range;      /**< 当前rts距离设定m */
    float rts_velocity;   /**< 当前rts速度设定m/s */

    /* fft结果 */
    quint32 bit_tx_order[4];                /**< 配置列表下tx发波次序 */
    COMPLEX_INT32_Typedef_t fft_data[4][16];/**< 配置列表下各通道2dfft数据 */
  }FFT_REQUEST_CONDITION_Typedef_t;

  QList<FFT_REQUEST_CONDITION_Typedef_t> fft_request_list;

  /* 版本:低字节byte0主版本号 v0 - 255 byte1：副版本号 0 - 255 byte2：修订版本号 0 - 255 */
  quint32 fft_data_version = 1;

  /* FFT数据类型 @ref DATA_Typedef_t */
  quint8 fft_data_type = 0;

  /* 数据系数 */
  qint32 fft_data_factor = 1024;

  /* rts */
  float rts_range = 0;
  float rts_velocity = 0;
  float rts_rcs = 0;
  float rts_start_frequency = 0;
  float rts_bandwidth = 0;

  /* 转动时间 */
  qint64 time_ms_s = 0;
  qint64 time_ms_e = 0;

  /* 水平转动 */
  float azi_left_angle_start = 0;
  float azi_right_angle_end = 0;
  float azi_direction_ele_angle = 0;
  float azi_step_angle = 0;

  /* 俯仰转动 */
  float ele_up_angle_end = 0;
  float ele_down_angle_start = 0;
  float ele_direction_azi_angle = 0;
  float ele_step_angle = 0;

  /* 转台条件位置记录 */
  qint32 angle_position_index = 0;

  /* 校准配置信息 */
  QList<eol_protocol::CALIBRATION_PROFILE_INFO_Typedef_t>calibration_profile_info_list;

  /* csv文件 */
  QFile fft_csv_file;
  QFile fft_csv_file_origin;
};

#endif // EOL_ANGLE_CALIBRATION_WINDOW_H
/******************************** End of file *********************************/
