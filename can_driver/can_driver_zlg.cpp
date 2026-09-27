/**
 *  @file can_driver_zlg.cpp
 *
 *  @date 2023年10月24日 16:37:54 星期二
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief 周立功can驱动.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.10.24 16:37 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-10-24 <td>v0.0.1  <td>aron566 <td>初始版本
 *  </table>
 */
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include "can_driver_zlg.h"
#include <QDateTime>
#include <QMessageBox>
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define USE_HW_CAN_SEND_64_DATA_EN  1/**< 是否开启非CANFD设备发送大于64Bytes的数据，采用分包发送 */
#define CAN_MSG_NUM_MAX             64U/**< can接收一次消息最大允许帧数 */
/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/

#ifdef Q_OS_WIN
/* 列表数据需要和对话框中设备列表数据一一对应 (Windows 全设备列表) */
static const can_driver_model::DEVICE_INFO_Typedef_t kDeviceType[] = {
  /* 周立功can */
  {"ZCAN_USBCAN1",            can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCAN1,            1},
  {"ZCAN_USBCAN2",            can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCAN2,            2},
  {"ZCAN_PCI9820I",           can_driver_model::ZLG_CAN_BRAND, ZCAN_PCI9820I,           2},
  {"ZCAN_USBCAN_E_U",         can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCAN_E_U,         1},
  {"ZCAN_USBCAN_2E_U",        can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCAN_2E_U,        2},
  {"ZCAN_USBCAN_4E_U",        can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCAN_4E_U,        4},
  {"ZCAN_PCIE_CANFD_100U",    can_driver_model::ZLG_CAN_BRAND, ZCAN_PCIE_CANFD_100U,    1},
  {"ZCAN_PCIE_CANFD_200U",    can_driver_model::ZLG_CAN_BRAND, ZCAN_PCIE_CANFD_200U,    2},
  {"ZCAN_PCIE_CANFD_400U_EX", can_driver_model::ZLG_CAN_BRAND, ZCAN_PCIE_CANFD_400U_EX, 4},
  {"ZCAN_USBCANFD_200U",      can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCANFD_200U,      2},
  {"ZCAN_USBCANFD_100U",      can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCANFD_100U,      1},
  {"ZCAN_USBCANFD_MINI",      can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCANFD_MINI,      1},
  {"ZCAN_CANETTCP",           can_driver_model::ZLG_CAN_BRAND, ZCAN_CANETTCP,           1},
  {"ZCAN_CANETUDP",           can_driver_model::ZLG_CAN_BRAND, ZCAN_CANETUDP,           1},
  {"ZCAN_WIFICAN_TCP",        can_driver_model::ZLG_CAN_BRAND, ZCAN_WIFICAN_TCP,        1},
  {"ZCAN_WIFICAN_UDP",        can_driver_model::ZLG_CAN_BRAND, ZCAN_WIFICAN_UDP,        1},
  {"ZCAN_CLOUD",              can_driver_model::ZLG_CAN_BRAND, ZCAN_CLOUD,              1},
  {"ZCAN_CANFDWIFI_TCP",      can_driver_model::ZLG_CAN_BRAND, ZCAN_CANFDWIFI_TCP,      1},
  {"ZCAN_CANFDWIFI_UDP",      can_driver_model::ZLG_CAN_BRAND, ZCAN_CANFDWIFI_UDP,      1},
  {"ZCAN_CANFDNET_TCP",       can_driver_model::ZLG_CAN_BRAND, ZCAN_CANFDNET_TCP,       2},
  {"ZCAN_CANFDNET_UDP",       can_driver_model::ZLG_CAN_BRAND, ZCAN_CANFDNET_UDP,       2},
  {"ZCAN_CANFDNET_400U_TCP",  can_driver_model::ZLG_CAN_BRAND, ZCAN_CANFDNET_400U_TCP,  4},
  {"ZCAN_CANFDNET_400U_UDP",  can_driver_model::ZLG_CAN_BRAND, ZCAN_CANFDNET_400U_UDP,  4},
};
#else
/* ================= Linux 兼容层 ================= */
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStringList>

/* Linux 头文件缺失的宏 (取值与 Windows 版 zlgcan 头文件一致) */
#ifndef MAKE_CAN_ID
#define MAKE_CAN_ID(id, eff, rtr, err) CAN_ID(id, eff, rtr, err)
#endif
#ifndef GET_ID
#define GET_ID(id) ((id) & CAN_ID_FLAG)
#endif
#ifndef TX_DELAY_SEND_FLAG
#define TX_DELAY_SEND_FLAG 0x80
#endif
/* ZCAN_USBCANFD_800U 在 Linux 版头文件中未定义，取值与 Windows 版头文件一致 */
#ifndef ZCAN_USBCANFD_800U
#define ZCAN_USBCANFD_800U 59
#endif

/* 列表数据需要和对话框中设备列表数据一一对应
   Linux 仅支持可通过 dlopen 动态加载的 3 款 ZCAN 设备 */
static const can_driver_model::DEVICE_INFO_Typedef_t kDeviceType[] = {
  /* 周立功can (Linux) */
  {"ZCAN_USBCAN_4E_U",   can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCAN_4E_U,   4},
  {"ZCAN_USBCAN_8E_U",   can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCAN_8E_U,   8},
  {"ZCAN_USBCANFD_800U", can_driver_model::ZLG_CAN_BRAND, ZCAN_USBCANFD_800U, 8},
};

/* ---------- ZCAN 动态加载器 (Linux) ----------
 * 三个 .so 导出同名 ZCAN_* 符号，不可静态链接；按设备类型 dlopen(RTLD_LOCAL) + dlsym。
 * 4E/8E 库不导出 ZCAN_TransmitFD/ZCAN_ReceiveFD，该符号按可选绑定(仅 800U 使用)。
 */
typedef struct _ZlgLinuxApi {
  void *handle;
  /* 用 decltype 直接取头文件中的函数类型，保证与官方头文件签名一致
     (注意: 此处 ZCAN_* 宏尚未定义，取的是头文件中的真实声明) */
  decltype(&ZCAN_OpenDevice) OpenDevice;
  decltype(&ZCAN_CloseDevice) CloseDevice;
  decltype(&ZCAN_GetDeviceInf) GetDeviceInf;
  decltype(&ZCAN_InitCAN) InitCAN;
  decltype(&ZCAN_StartCAN) StartCAN;
  decltype(&ZCAN_ResetCAN) ResetCAN;
  decltype(&ZCAN_Transmit) Transmit;
  decltype(&ZCAN_GetReceiveNum) GetReceiveNum;
  decltype(&ZCAN_Receive) Receive;
  decltype(&ZCAN_TransmitFD) TransmitFD;
  decltype(&ZCAN_ReceiveFD) ReceiveFD;
  decltype(&GetIProperty) pGetIProperty;
  decltype(&ReleaseIProperty) pReleaseIProperty;
} ZlgLinuxApi;

static ZlgLinuxApi g_zlgApi;      /**< 零初始化，函数指针初始全为 nullptr */
static QString g_zlgLoadedSo;    /**< 已加载的 .so 文件名(不含路径) */

static const char *zlg_so_name_for_device(quint32 device_type)
{
  switch(device_type)
  {
    case ZCAN_USBCAN_4E_U:   return "libusbcan-4e.so";
    case ZCAN_USBCAN_8E_U:   return "libusbcan-8e.so";
    case ZCAN_USBCANFD_800U: return "libusbcanfd800u.so";
    default:                 return nullptr;
  }
}

static void *zlg_dlopen_from_dirs(const QString &soName)
{
  /* 搜索顺序: 程序目录 -> 程序目录/../lib -> 系统默认路径 */
  QStringList dirs;
  dirs << QCoreApplication::applicationDirPath()
       << QCoreApplication::applicationDirPath() + "/../lib"
       << "/usr/local/lib"
       << "/usr/lib/x86_64-linux-gnu"
       << "/usr/lib";
  for(const QString &dir : dirs)
  {
    QString path = QDir(dir).filePath(soName);
    if(QFile::exists(path))
    {
      void *h = dlopen(path.toUtf8().constData(), RTLD_NOW | RTLD_LOCAL);
      if(nullptr != h)
      {
        return h;
      }
      qDebug() << "zlg dlopen failed:" << path << dlerror();
    }
  }
  /* 最后让动态链接器按默认规则搜索 */
  return dlopen(soName.toUtf8().constData(), RTLD_NOW | RTLD_LOCAL);
}

static bool zlg_load_api(quint32 device_type)
{
  const char *soName = zlg_so_name_for_device(device_type);
  if(nullptr == soName)
  {
    qDebug() << "zlg unsupported device type:" << device_type;
    return false;
  }

  /* 同库已加载则直接复用 */
  if(nullptr != g_zlgApi.handle && g_zlgLoadedSo == QString::fromLatin1(soName))
  {
    return true;
  }

  void *h = zlg_dlopen_from_dirs(QString::fromLatin1(soName));
  if(nullptr == h)
  {
    qDebug() << "zlg dlopen failed:" << soName << dlerror();
    return false;
  }

#define ZLG_BIND_REQUIRED(zcan_sym, member) \
  do { \
    g_zlgApi.member = (decltype(g_zlgApi.member))dlsym(h, #zcan_sym); \
    if(nullptr == g_zlgApi.member) { qDebug() << "zlg dlsym failed:" << #zcan_sym; dlclose(h); return false; } \
  } while(0)

#define ZLG_BIND_OPTIONAL(zcan_sym, member) \
  do { \
    g_zlgApi.member = (decltype(g_zlgApi.member))dlsym(h, #zcan_sym); \
  } while(0)

  ZLG_BIND_REQUIRED(ZCAN_OpenDevice, OpenDevice);
  ZLG_BIND_REQUIRED(ZCAN_CloseDevice, CloseDevice);
  ZLG_BIND_REQUIRED(ZCAN_GetDeviceInf, GetDeviceInf);
  ZLG_BIND_REQUIRED(ZCAN_InitCAN, InitCAN);
  ZLG_BIND_REQUIRED(ZCAN_StartCAN, StartCAN);
  ZLG_BIND_REQUIRED(ZCAN_ResetCAN, ResetCAN);
  ZLG_BIND_REQUIRED(ZCAN_Transmit, Transmit);
  ZLG_BIND_REQUIRED(ZCAN_GetReceiveNum, GetReceiveNum);
  ZLG_BIND_REQUIRED(ZCAN_Receive, Receive);
  /* FD 接口仅 800U 的库导出，4E/8E 缺失不报错(驱动逻辑仅对 CANFD 设备调用) */
  ZLG_BIND_OPTIONAL(ZCAN_TransmitFD, TransmitFD);
  ZLG_BIND_OPTIONAL(ZCAN_ReceiveFD, ReceiveFD);
  ZLG_BIND_REQUIRED(GetIProperty, pGetIProperty);
  ZLG_BIND_REQUIRED(ReleaseIProperty, pReleaseIProperty);

#undef ZLG_BIND_REQUIRED
#undef ZLG_BIND_OPTIONAL

  /* 旧句柄不再关闭: 不同 .so 符号同名但 RTLD_LOCAL 隔离，保留避免悬垂；
     进程退出时由系统回收 */
  g_zlgApi.handle = h;
  g_zlgLoadedSo = QString::fromLatin1(soName);
  qDebug() << "zlg linux api loaded:" << soName;
  return true;
}

/* ---------- ZCAN_SetValue/GetValue 兼容层 (Linux) ----------
 * Linux 库不导出全局 ZCAN_SetValue/ZCAN_GetValue，改经由 IProperty::SetValue/GetValue 实现。
 * Windows 属性路径形如 "通道号/属性名"(如 "0/baud_rate")，
 * Linux 为 "info/channel/channel_通道号/属性名"，此处做转换。
 * 经核实的路径: "N/baud_rate" -> "info/channel/channel_N/baud_rate" (4E/8E 标准波特率)。
 * 库不支持的属性返回失败(0)，调用方按原有逻辑处理并提示。
 */
static void zlg_translate_property_path(const char *winPath, char *out, size_t outSize)
{
  unsigned int ch = 0;
  int n = 0;
  if(nullptr != winPath && 1 == sscanf(winPath, "%u/%n", &ch, &n) && '\0' != winPath[n])
  {
    snprintf(out, outSize, "info/channel/channel_%u/%s", ch, winPath + n);
  }
  else if(nullptr != winPath)
  {
    snprintf(out, outSize, "%s", winPath);
  }
  else
  {
    out[0] = '\0';
  }
}

static UINT zlg_SetValue_linux(DEVICE_HANDLE device_handle, const char *path, const void *value)
{
  if(nullptr == g_zlgApi.pGetIProperty || INVALID_DEVICE_HANDLE == device_handle || nullptr == path)
  {
    return 0;
  }
  IProperty *prop = g_zlgApi.pGetIProperty(device_handle);
  if(nullptr == prop || nullptr == prop->SetValue)
  {
    return 0;
  }
  char linuxPath[160];
  zlg_translate_property_path(path, linuxPath, sizeof(linuxPath));
  /* Windows 侧字符串属性传 const char*，与 IProperty::SetValue(const char*, const char*) 一致 */
  int ret = prop->SetValue(linuxPath, (const char *)value);
  if(nullptr != g_zlgApi.pReleaseIProperty)
  {
    g_zlgApi.pReleaseIProperty(prop);
  }
  return (UINT)ret;
}

static const void *zlg_GetValue_linux(DEVICE_HANDLE device_handle, const char *path)
{
  if(nullptr == g_zlgApi.pGetIProperty || INVALID_DEVICE_HANDLE == device_handle || nullptr == path)
  {
    return nullptr;
  }
  IProperty *prop = g_zlgApi.pGetIProperty(device_handle);
  if(nullptr == prop || nullptr == prop->GetValue)
  {
    return nullptr;
  }
  char linuxPath[160];
  zlg_translate_property_path(path, linuxPath, sizeof(linuxPath));
  const char *ret = prop->GetValue(linuxPath);
  /* 诊断类 GetValue 的返回值在 Linux 侧为字符串，与 Windows 侧语义不同，
     仅做透传；此处不释放 IProperty，保证返回指针在本次调用后仍有效(诊断路径低频调用) */
  return (const void *)ret;
}

/* FD 接口在 4E/8E 库中不存在，调用前判空，避免非 FD 设备误选 CANFD 协议时崩溃 */
static INT zlg_TransmitFD_linux(CHANNEL_HANDLE h, ZCAN_TransmitFD_Data *d, UINT n)
{
  if(nullptr == g_zlgApi.TransmitFD)
  {
    return 0;
  }
  return g_zlgApi.TransmitFD(h, d, n);
}
static INT zlg_ReceiveFD_linux(CHANNEL_HANDLE h, ZCAN_ReceiveFD_Data *d, UINT n, INT t)
{
  if(nullptr == g_zlgApi.ReceiveFD)
  {
    return 0;
  }
  return g_zlgApi.ReceiveFD(h, d, n, t);
}

/* 把驱动代码中的 ZCAN_* 调用重定向到动态加载的函数指针 / 兼容层。
 * 注意: 这些宏必须定义在 #include "can_driver_zlg.h"(内含 zlgcan.h 声明) 之后，
 * 否则会破坏头文件中的函数声明。 */
#define ZCAN_OpenDevice    g_zlgApi.OpenDevice
#define ZCAN_CloseDevice   g_zlgApi.CloseDevice
#define ZCAN_GetDeviceInf  g_zlgApi.GetDeviceInf
#define ZCAN_InitCAN       g_zlgApi.InitCAN
#define ZCAN_StartCAN      g_zlgApi.StartCAN
#define ZCAN_ResetCAN      g_zlgApi.ResetCAN
#define ZCAN_Transmit      g_zlgApi.Transmit
#define ZCAN_GetReceiveNum g_zlgApi.GetReceiveNum
#define ZCAN_Receive       g_zlgApi.Receive
#define ZCAN_TransmitFD    zlg_TransmitFD_linux
#define ZCAN_ReceiveFD     zlg_ReceiveFD_linux
#define ZCAN_SetValue      zlg_SetValue_linux
#define ZCAN_GetValue      zlg_GetValue_linux
#endif /* Q_OS_WIN / Linux */

/* USBCANFD */
static const quint32 kAbitTimingUSB[] = {
  1000000U,//1Mbps
  800000U,//800kbps
  500000U,//500kbps
  250000U,//250kbps
  125000U,//125kbps
  100000U,//100kbps
  50000U, //50kbps
};
static const quint32 kDbitTimingUSB[] = {
  5000000U,//5Mbps
  4000000U,//4Mbps
  2000000U,//2Mbps
  1000000U, //1Mbps
};

/* PCIECANFD brp=1 */
//static const quint32 kAbitTimingPCIE[] = {
//  1000000, //1M(80%)
//  800000, //800K(80%)
//  500000, //500K(80%)
//  250000, //250K(80%)
//  125000  //125K(80%)
//};

//static const quint32 kDbitTimingPCIE[] = {
//  8000000, //8Mbps(80%)
//  5000000, //5Mbps(75%)
//  5000000, //5Mbps(87.5%)
//  4000000, //4Mbps(80%)
//  2000000, //2Mbps(80%)
//  1000000  //1Mbps(80%)
//};

/* can卡波特率 */
static const quint32 kBaudrate[] = {
  1000000U,
  800000U,
  500000U,
  250000U,
  125000U,
  100000U,
  50000U,
  20000U,
  10000U,
  5000U,
};

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

can_driver_zlg::can_driver_zlg(QObject *parent)
    : can_driver_model{parent}
{

}

can_driver_model::SET_FUNCTION_CAN_USE_Typedef_t can_driver_zlg::function_can_use_update_for_choose(const QString &device_type_str)
{
  quint16 index = 0;
  SET_FUNCTION_CAN_USE_Typedef_t function_can_use;

  QString device_name;

  for(quint16 i = 0; i < sizeof(kDeviceType) / sizeof(kDeviceType[0]); i++)
  {
    device_name = QString(kDeviceType[i].device_type_str);
    function_can_use.device_list.append(device_name);
  }

  for(quint16 i = 0; i < sizeof(kDeviceType) / sizeof(kDeviceType[0]); i++)
  {
    if(0 == QString::compare(device_type_str, QString(kDeviceType[i].device_type_str)))
    {
      index = i;

      /* 获取当前通道数 */
      function_can_use.channel_num = kDeviceType[i].channel_count;
    }
  }

  quint32 type = kDeviceType[index].device_type;
#ifdef Q_OS_WIN
  const bool cloudDevice = type == ZCAN_CLOUD;
  const bool netcanfd = is_net_can_fd_type(type);
  const bool netcan = is_net_can_type(type);
  const bool netDevice = (netcan || netcanfd);
  const bool tcpDevice = is_net_tcp_type(type);
  const bool udpDevice = is_net_udp_type(type);
  const bool usbcanfd = type == ZCAN_USBCANFD_100U ||
                        type == ZCAN_USBCANFD_200U ||
                        type == ZCAN_USBCANFD_MINI;
  const bool pciecanfd = type == ZCAN_PCIE_CANFD_100U ||
                         type == ZCAN_PCIE_CANFD_200U ||
                         type == ZCAN_PCIE_CANFD_400U ||
                         type == ZCAN_PCIE_CANFD_400U_EX;

  const bool canfdDevice = usbcanfd || pciecanfd || netcanfd;
  const bool accFilter = pciecanfd || type == ZCAN_USBCAN1 || type == ZCAN_USBCAN2;
#else
  /* Linux 仅支持 USB 直连的 3 款设备，无网络/云/PCIE 设备；
     ZCAN_USBCANFD_800U 为 CAN FD 设备 */
  const bool cloudDevice = false;
  const bool netcanfd = false;
  const bool netcan = false;
  const bool netDevice = false;
  const bool tcpDevice = false;
  const bool udpDevice = false;
  const bool usbcanfd = (type == ZCAN_USBCANFD_800U);
  const bool pciecanfd = false;

  const bool canfdDevice = usbcanfd;
  const bool accFilter = false;
#endif

  /* 工作模式是否可选 */
  function_can_use.work_mode_can_use = ((false == cloudDevice && false == netDevice));

  /* 终端电阻使能是否可选 */
  function_can_use.resistance_cs_use = ((true == usbcanfd));

  /* 波特率选择是否可选 */
  function_can_use.bauds_can_use = ((false == canfdDevice && false == netDevice && false == cloudDevice));

  /* 仲裁域，数据域波特率是否可选 */
  function_can_use.arbitration_data_bauds_can_use = ((true == canfdDevice && false == netDevice && false == cloudDevice));

  /* 自定义波特率选择是否可选 */
  function_can_use.diy_bauds_can_use = ((false == cloudDevice && false == netDevice));

  /* 过滤模式是否可选（验收码，屏蔽码） */
  function_can_use.filter_can_use = ((true == accFilter && false == cloudDevice && false == netDevice));

  /* 网络相关可选设置 */
  if(tcpDevice)
  {
    const bool server = (net_mode_index_ == 0);/* 0服务器 1客户端 */
    /* 本地端口是否可选 */
    function_can_use.local_port_can_use = (server);

    /* 远程端口是否可选 */
    function_can_use.remote_port_can_use = ((false == server));

    /* 远程地址是否可选 */
    function_can_use.remote_addr_can_use = ((false == server));
  }else if(udpDevice)
  {
    /* 本地端口是否可选 */
    function_can_use.local_port_can_use = true;

    /* 远程端口是否可选 */
    function_can_use.remote_port_can_use = true;

    /* 远程地址是否可选 */
    function_can_use.remote_addr_can_use = true;
  }
  else
  {
    /* 本地端口是否可选 */
    function_can_use.local_port_can_use = false;

    /* 远程端口是否可选 */
    function_can_use.remote_port_can_use = false;

    /* 远程地址是否可选 */
    function_can_use.remote_addr_can_use = false;
  }

  for(quint16 i = 0; i < sizeof(kAbitTimingUSB) / sizeof(kAbitTimingUSB[0]); i++)
  {
    function_can_use.abitrate_list.append(bps_number2str(kAbitTimingUSB[i]));
  }
  for(quint16 i = 0; i < sizeof(kDbitTimingUSB) / sizeof(kDbitTimingUSB[0]); i++)
  {
    function_can_use.datarate_list.append(bps_number2str(kDbitTimingUSB[i]));
  }
  for(quint16 i = 0; i < sizeof(kBaudrate) / sizeof(kBaudrate[0]); i++)
  {
    function_can_use.baudrate_list.append(bps_number2str(kBaudrate[i]));
  }

  return function_can_use;
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

QStringList can_driver_zlg::set_device_brand(CAN_BRAND_Typedef_t brand)
{
  QStringList device_list;
  QString device_name;

  brand_ = brand;
  for(quint16 i = 0; i < sizeof(kDeviceType) / sizeof(kDeviceType[0]); i++)
  {
    device_name = QString(kDeviceType[i].device_type_str);
    if(brand_ == kDeviceType[i].brand)
    {
      device_list.append(device_name);
    }
  }
  return device_list;
}

quint8 can_driver_zlg::set_device_type(const QString &device_type_str)
{
  for(quint16 i = 0; i < sizeof(kDeviceType) / sizeof(kDeviceType[0]); i++)
  {
    if(0 == QString::compare(device_type_str, QString(kDeviceType[i].device_type_str)))
    {
      device_type_index_ = i;

      /* 更新功能列表 */
      function_can_use_update();
      return kDeviceType[i].channel_count;
    }
  }
  return 0;
}

bool can_driver_zlg::open()
{
  //  if(kDeviceType[device_type_index_].device_type == ZCAN_CLOUD)
  //  {
  //    zcloudDlg dlg;
  //    if(IDCANCEL == dlg.DoModal())
  //    {
  //      return;
  //    }
  //    device_index_ = dlg.GetDeviceIndex();
  //  }
#ifndef Q_OS_WIN
  /* Linux: 按设备类型 dlopen 对应的官方 .so 并绑定 ZCAN_* 符号，失败则直接返回 */
  if(false == zlg_load_api(kDeviceType[device_type_index_].device_type))
  {
    show_message(tr("load zlg linux driver library failed"));
    return false;
  }
#endif
  device_handle_ = ZCAN_OpenDevice(kDeviceType[device_type_index_].device_type, device_index_, 0);
  if(INVALID_DEVICE_HANDLE == device_handle_)
  {
    show_message(tr("open zlg device failed"));
    return false;
  }

  /* 发送can打开状态 */
  device_opened_ = true;
  emit signal_can_is_opened();
  show_message(tr("open device ok"));
  return true;
}

bool can_driver_zlg::read_info()
{
  QString show_info;

  if(nullptr == device_handle_)
  {
    return false;
  }
  ZCAN_DEVICE_INFO info;
  ZCAN_GetDeviceInf(device_handle_, &info);

  show_info += QString("<font size='5' color='green'><div align='legt'>hw_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.hw_Version, 0, 16);
  show_info += QString("<font size='5' color='green'><div align='legt'>fw_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.fw_Version, 0, 16);
  show_info += QString("<font size='5' color='green'><div align='legt'>dr_Version:</div> <div align='right'>v%1</div> </font>\r\n").arg(info.dr_Version, 0, 16);
  show_info += QString("<font size='5' color='green'><div align='legt'>str_Serial_Num:</div> <div align='right'>");
  char str[sizeof(info.str_Serial_Num) + 1] = {0};
  memcpy(str, info.str_Serial_Num, sizeof(info.str_Serial_Num));
  show_info += QString::asprintf("%s", str);
  show_info += QString("</div> </font>\r\n");

  show_info += QString("<font size='5' color='green'><div align='legt'>str_hw_Type:</div> <div align='right'>");
  char hwstr[sizeof(info.str_hw_Type) + 1] = {0};
  memcpy(hwstr, info.str_hw_Type, sizeof(info.str_hw_Type));
  show_info += QString::asprintf("%s", hwstr);
  show_info += QString("</div> </font>\r\n");

  show_info += QString("<font size='5' color='green'><div align='legt'>can_Num:</div> <div align='right'>%1</div> </font>\r\n").arg(info.can_Num);

  QMessageBox message(QMessageBox::Information, tr("Info"), show_info, QMessageBox::Yes, nullptr);
  message.exec();
  return true;
}

bool can_driver_zlg::init(CHANNEL_STATE_Typedef_t &channel_state)
{
  ZCAN_CHANNEL_INIT_CONFIG config;
  memset(&config, 0, sizeof(config));

  quint32 type = kDeviceType[device_type_index_].device_type;
#ifdef Q_OS_WIN
  const bool cloudDevice = type == ZCAN_CLOUD;
  const bool netcanfd = is_net_can_fd_type(type);
  const bool netcan = is_net_can_type(type);
  const bool netDevice = (netcan || netcanfd);
  const bool tcpDevice = is_net_tcp_type(type);
  const bool usbcanfd = type == ZCAN_USBCANFD_100U ||
                        type == ZCAN_USBCANFD_200U ||
                        type == ZCAN_USBCANFD_MINI;
  const bool pciecanfd = type == ZCAN_PCIE_CANFD_100U ||
                         type == ZCAN_PCIE_CANFD_200U ||
                         type == ZCAN_PCIE_CANFD_400U_EX;
  const bool canfdDevice = usbcanfd || pciecanfd;
#else
  /* Linux 仅支持 USB 直连的 3 款设备，无网络/云/PCIE 设备；
     ZCAN_USBCANFD_800U 为 CAN FD 设备 */
  const bool cloudDevice = false;
  const bool netcanfd = false;
  const bool netcan = false;
  const bool netDevice = false;
  const bool tcpDevice = false;
  const bool usbcanfd = (type == ZCAN_USBCANFD_800U);
  const bool pciecanfd = false;
  const bool canfdDevice = usbcanfd;
#endif
  const bool server = net_mode_index_ == 0;

  if(cloudDevice)
  {
    qDebug() << "cloudDevice";
  }
  else if(netDevice)
  {
    qDebug() << "netDevice/";
    char path[50] = {0};
    char value[100] = {0};
    if(tcpDevice)
    {
      qDebug() << "tcpDevice/";
      snprintf(path, sizeof(path), "%d/work_mode", channel_state.channel_num);
      snprintf(value, sizeof(value), "%d", server ? 1 : 0);
      ZCAN_SetValue(device_handle_, path, value);
      if(server)
      {
        qDebug() << "server";
        snprintf(path, sizeof(path), "%d/local_port", channel_state.channel_num);
        ZCAN_SetValue(device_handle_, path, local_port_str.toStdString().data());
      } // server
      else
      {
        qDebug() << "client";
        snprintf(path, sizeof(path), "%d/ip", channel_state.channel_num);
        ZCAN_SetValue(device_handle_, path, service_ip_str.toStdString().data());

        snprintf(path, sizeof(path), "%d/work_port", channel_state.channel_num);
        ZCAN_SetValue(device_handle_, path, service_port_str.toStdString().data());
      }
    } // tcp
    else
    {
      qDebug() << "udpDevice";
      snprintf(path, sizeof(path), "%d/local_port", channel_state.channel_num);
      ZCAN_SetValue(device_handle_, path, local_port_str.toStdString().data());
      snprintf(path, sizeof(path), "%d/ip", channel_state.channel_num);

      ZCAN_SetValue(device_handle_, path, service_ip_str.toStdString().data());
      snprintf(path, sizeof(path), "%d/work_port", channel_state.channel_num);
      ZCAN_SetValue(device_handle_, path, service_port_str.toStdString().data());
    }
  }
  else
  {
    /* 设置波特率 */
    if(custom_baud_enable_)
    {
      qDebug() << "set diy bps";
      if(!custom_baud_rate_config(channel_state))
      {
        show_message(tr("set diy baudrate failed "), channel_state.channel_num);
        return false;
      }
    }
    else
    {
      qDebug() << "set bps";
      if(!canfdDevice && !baud_rate_config(channel_state))
      {
        show_message(tr("set baudrate failed "), channel_state.channel_num);
        return false;
      }
    }

    if(usbcanfd)
    {
      qDebug() << "set usbcanfd channel";
      char path[50] = {0};
      char value[100] = {0};
      snprintf(path, sizeof(path), "%d/canfd_standard", channel_state.channel_num);
      snprintf(value, sizeof(value), "%d", 0);
      ZCAN_SetValue(device_handle_, path, value);
    }
    if(usbcanfd)
    {
      qDebug() << "usbcanfd diy bps";
      if(custom_baud_enable_)
      {
        qDebug() << "usbcanfd diy bps set ...";
        if(!custom_baud_rate_config(channel_state))
        {
          show_message(tr("set diy baudrate failed "), channel_state.channel_num);
          return false;
        }
      }
      else
      {
        qDebug() << "usbcanfd bps set ...";
        if(!cand_fd_bps_config(channel_state))
        {
          show_message(tr("set baudrate failed "), channel_state.channel_num);
          return false;
        }
      }
      config.can_type = TYPE_CANFD;
      config.canfd.mode = work_mode_index_;
      config.canfd.filter = filter_mode_;
      bool ok;
      config.canfd.acc_code = acc_code_.toUInt(&ok, 16);
      config.canfd.acc_mask = acc_mask_.toUInt(&ok, 16);
    }
#ifdef Q_OS_WIN
    else if(pciecanfd)
    {

      char path[50] = { 0 };
      char value[100] = { 0 };
      if(!cand_fd_bps_config(channel_state))
      {
        show_message(tr("set baudrate failed "), channel_state.channel_num);
        return false;
      }

      if(type == ZCAN_PCIE_CANFD_400U_EX )
      {
        snprintf(path, sizeof(path), "0/set_device_recv_merge");
        snprintf(value, sizeof(value), "0");
        ZCAN_SetValue(device_handle_, path, value);
      }

      config.can_type = TYPE_CANFD;
      config.canfd.mode = work_mode_index_;
      config.canfd.filter = filter_mode_;
      bool ok;
      config.canfd.acc_code = acc_code_.toUInt(&ok, 16);
      config.canfd.acc_mask = acc_mask_.toUInt(&ok, 16);

    }
#endif
    else
    {
      config.can_type = TYPE_CAN;
      config.can.mode = work_mode_index_;
      config.can.filter = filter_mode_;
      bool ok;
      config.can.acc_code = acc_code_.toUInt(&ok, 16);
      config.can.acc_mask = acc_mask_.toUInt(&ok, 16);
    }
  }

  channel_state.channel_handle = ZCAN_InitCAN(device_handle_, channel_state.channel_num, &config);
  if(INVALID_CHANNEL_HANDLE == channel_state.channel_handle)
  {
    show_message(tr("zlg can ch %1 init failed").arg(channel_state.channel_num), channel_state.channel_num);
    return false;
  }
  if(usbcanfd)
  {
    if(resistance_enable_ && !resistance_config(channel_state))
    {
      show_message(tr("set resistance failed"), channel_state.channel_num);
      return false;
    }
  }
  show_message(tr("zlg can ch %1 intit ok").arg(channel_state.channel_num), channel_state.channel_num);
  return true;
}

bool can_driver_zlg::init()
{
  if(false == device_opened_)
  {
    show_message(tr("device is not open "));
    return false;
  }

  /* 初始化对应通道号 */
  bool ret = false;
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }
    channel_state = channel_state_list.takeAt(i);

    ret = init(channel_state);

    channel_state_list.insert(i, channel_state);
  }
  return ret;
}

bool can_driver_zlg::start(const CHANNEL_STATE_Typedef_t &channel_state)
{
  if(ZCAN_StartCAN(channel_state.channel_handle) != STATUS_OK)
  {
    show_message(tr("zlg start can ch %1 failed").arg(channel_state.channel_num), channel_state.channel_num);
    return false;
  }
  show_message(tr("zlg start can ch %1 ok").arg(channel_state.channel_num), channel_state.channel_num);
  return true;
}

bool can_driver_zlg::start()
{
  if(false == device_opened_)
  {
    show_message(tr("device is not open "));
    return false;
  }

  /* 启动对应通道号 */
  bool ret = false;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }
    ret = start(channel_state_list.value(i));
  }

  start_ = true;

  return ret;
}

bool can_driver_zlg::reset(const CHANNEL_STATE_Typedef_t &channel_state)
{
  if(ZCAN_ResetCAN(channel_state.channel_handle) != STATUS_OK)
  {
    show_message(tr("zlg reset can ch %1 failed ").arg(channel_state.channel_num), channel_state.channel_num);
    return false;
  }
  show_message(tr("zlg reset can ch %1 ok ").arg(channel_state.channel_num), channel_state.channel_num);
  return true;
}

bool can_driver_zlg::reset()
{
  /* 复位对应通道号 */
  bool ret = false;

  start_ = false;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }
    ret = reset(channel_state_list.value(i));
  }
  return ret;
}

void can_driver_zlg::close_channel(const CHANNEL_STATE_Typedef_t &channel_state)
{
  ZCAN_ResetCAN(channel_state.channel_handle);
  show_message(tr("zlg device can ch %1 closed").arg(channel_state.channel_num), channel_state.channel_num);
}

bool can_driver_zlg::close()
{
  /* 保护 */
  if(false == device_opened_)
  {
    show_message(tr("device is not open "));
    return false;
  }

  start_ = false;

  /* 关闭对应通道号 */
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    if(false == channel_state_list.value(i).channel_en)
    {
      continue;
    }

    close_channel(channel_state_list.value(i));
  }

  /* 关闭设备 */
#ifdef Q_OS_WIN
  /* ZCLOUD 接口仅 Windows 版 zlgcan 库提供 */
  if(ZCLOUD_IsConnected())
  {
    ZCLOUD_DisconnectServer();
  }
#endif
  ZCAN_CloseDevice(device_handle_);

  device_opened_ = false;
  return true;
}

quint32 can_driver_zlg::zlg_can_send(const CHANNEL_STATE_Typedef_t &channel_state, \
                                 const quint8 *data, quint8 size, quint32 id, \
                                 FRAME_TYPE_Typedef_t frame_type, \
                                 PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 需要发送的帧数 */
  quint32 nSendCount = 1;

  /* 实际发送的帧数 */
  quint32 result = 0;

  switch(protocol)
  {
    /* can */
    case CAN_PROTOCOL_TYPE:
      {
        ZCAN_Transmit_Data can_data;
        can_frame_packed(can_data, id, (quint32)frame_type, data, size);

        if(nSendCount > 0)
        {
          ZCAN_Transmit_Data* pData = new ZCAN_Transmit_Data[nSendCount];
          for(quint32 i = 0; i < nSendCount; ++i)
          {
            memcpy_s(&pData[i], sizeof(ZCAN_Transmit_Data), &can_data, sizeof(can_data));
          }

          result = ZCAN_Transmit(channel_state.channel_handle, pData, nSendCount);
          delete [] pData;
        }
        break;
      }

    /* canfd */
    case CANFD_PROTOCOL_TYPE:
      {
        ZCAN_TransmitFD_Data canfd_data;
        can_frame_packed(canfd_data, id, frame_type, data, size);

        if (nSendCount > 0)
        {
          ZCAN_TransmitFD_Data* pData = new ZCAN_TransmitFD_Data[nSendCount];
          for(quint32 i = 0; i < nSendCount; ++i)
          {
            memcpy_s(&pData[i], sizeof(ZCAN_TransmitFD_Data), &canfd_data, sizeof(canfd_data));
          }

          result = ZCAN_TransmitFD(channel_state.channel_handle, pData, nSendCount);
          delete [] pData;
        }
        break;
      }

    default:
      return 0;
  }
  return result;
}

/* 发送固定协议帧长 */
bool can_driver_zlg::send(const CHANNEL_STATE_Typedef_t &channel_state, \
                      const quint8 *data, quint8 size, quint32 id, \
                      FRAME_TYPE_Typedef_t frame_type, \
                      PROTOCOL_TYPE_Typedef_t protocol)
{
  /* 需要发送的帧数 */
  quint32 nSendCount = 1;

  /* 实际发送的帧数 */
  quint32 result = 0;

  bool ret = false;

  result = zlg_can_send(channel_state, data, size, id, frame_type, protocol);

  /* 消息分发到UI显示cq */
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, CAN_TX_DIRECT,
                   protocol,
                   frame_type,
                   DATA_FRAME_TYPE,
                   data, size);

  QString csText;
  csText = QString("send num:%1, sucess num:%2").arg(nSendCount).arg(result);
  QString result_info_str;
  if(result != nSendCount)
  {
    ret = false;
    result_info_str = tr("[%1]send data failed! ").arg(channel_state.channel_num) + csText;
  }
  else
  {
    ret = true;
    result_info_str = tr("[%1]send data sucessful! ").arg(channel_state.channel_num) + csText;
  }
  msg_to_ui_cq_buf(id, (quint8)channel_state.channel_num, UNKNOW_DIRECT,
                   protocol,
                   frame_type,
                   DATA_FRAME_TYPE,
                   (const quint8 *)result_info_str.toUtf8().data(), result_info_str.size());
  return ret;
}

void can_driver_zlg::function_can_use_update()
{
  quint32 type = kDeviceType[device_type_index_].device_type;
#ifdef Q_OS_WIN
  const bool cloudDevice = type == ZCAN_CLOUD;
  const bool netcanfd = is_net_can_fd_type(type);
  const bool netcan = is_net_can_type(type);
  const bool netDevice = (netcan || netcanfd);
  const bool tcpDevice = is_net_tcp_type(type);
  const bool udpDevice = is_net_udp_type(type);
  const bool usbcanfd = type == ZCAN_USBCANFD_100U ||
                        type == ZCAN_USBCANFD_200U ||
                        type == ZCAN_USBCANFD_MINI;
  const bool pciecanfd = type == ZCAN_PCIE_CANFD_100U ||
                         type == ZCAN_PCIE_CANFD_200U ||
                         type == ZCAN_PCIE_CANFD_400U ||
                         type == ZCAN_PCIE_CANFD_400U_EX;

  const bool canfdDevice = usbcanfd || pciecanfd || netcanfd;
  const bool accFilter = pciecanfd || type == ZCAN_USBCAN1 || type == ZCAN_USBCAN2;
#else
  /* Linux 仅支持 USB 直连的 3 款设备，无网络/云/PCIE 设备；
     ZCAN_USBCANFD_800U 为 CAN FD 设备 */
  const bool cloudDevice = false;
  const bool netcanfd = false;
  const bool netcan = false;
  const bool netDevice = false;
  const bool tcpDevice = false;
  const bool udpDevice = false;
  const bool usbcanfd = (type == ZCAN_USBCANFD_800U);
  const bool pciecanfd = false;

  const bool canfdDevice = usbcanfd;
  const bool accFilter = false;
#endif

  /* 更新设备通道列表 */
  channel_state_list.clear();
  CHANNEL_STATE_Typedef_t channel_state;
  for(quint8 i = 0; i < kDeviceType[device_type_index_].channel_count; i++)
  {
    /* 默认通道1开启 */
    if(0 == i)
    {
      channel_state.channel_en = true;
    }
    else
    {
      channel_state.channel_en = false;
    }
    channel_state.channel_num = i;
    channel_state_list.append(channel_state);
  }

  /* 队列发送支持 */
  support_delay_send_ = usbcanfd || pciecanfd || netcanfd;
  support_delay_send_mode_ = usbcanfd || pciecanfd;
  support_get_send_mode_ = usbcanfd || pciecanfd;
  delay_send_can_use_update(support_delay_send_, support_delay_send_mode_, support_get_send_mode_);

  /* 队列发送模式是否启用 */
  send_queue_mode = false;
  emit signal_send_queue_mode_can_use(send_queue_mode);

  /* 定时发送支持 */
  const bool support_autosend_canfd = canfdDevice;    // CANFD 设备
  const bool support_autosend_can = canfdDevice ;     // CANFD 设备和其他CAN设备
  const bool support_autosend_index = (support_autosend_can && !pciecanfd);   // PCIECANFD 不支持使用索引控制定时，PCIECANFD添加一条即立即发送
  const bool support_stop_single_autosend = usbcanfd;
  const bool support_get_autosend_list = netcanfd;
#ifdef Q_OS_WIN
  auto_send_can_use_update(support_autosend_can, support_autosend_canfd, \
                           support_autosend_index, support_stop_single_autosend, \
                           support_get_autosend_list);
#else
  /* Linux: 定时发送功能暂不支持 (Linux 版 ZCAN(AUTO)_TRANSMIT_OBJ 结构与 Windows 不兼容)，禁用 */
  auto_send_can_use_update(false, false, false, false, false);
#endif

  /* 工作模式是否可选 */
  emit signal_work_mode_can_use((false == cloudDevice && false == netDevice));

  /* 终端电阻使能是否可选 */
  emit signal_resistance_cs_use((true == usbcanfd));

  /* 波特率选择是否可选 */
  emit signal_bauds_can_use((false == canfdDevice && false == netDevice && false == cloudDevice));

  /* 仲裁域，数据域波特率是否可选 */
  emit signal_arbitration_data_bauds_can_use((true == canfdDevice && false == netDevice && false == cloudDevice));

  /* 自定义波特率选择是否可选 */
  emit signal_diy_bauds_can_use((false == cloudDevice && false == netDevice));

  /* 过滤模式是否可选（验收码，屏蔽码） */
  emit signal_filter_can_use((true == accFilter && false == cloudDevice && false == netDevice));

  /* 网络相关可选设置 */
  if(tcpDevice)
  {
    const bool server = (net_mode_index_ == 0);
    /* 本地端口是否可选 */
    emit signal_local_port_can_use(server);

    /* 远程端口是否可选 */
    emit signal_remote_port_can_use((false == server));

    /* 远程地址是否可选 */
    emit signal_remote_addr_can_use((false == server));
  }else if(udpDevice)
  {
    /* 本地端口是否可选 */
    emit signal_local_port_can_use(true);

    /* 远程端口是否可选 */
    emit signal_remote_port_can_use(true);

    /* 远程地址是否可选 */
    emit signal_remote_addr_can_use(true);
  }
  else
  {
    /* 本地端口是否可选 */
    emit signal_local_port_can_use(false);

    /* 远程端口是否可选 */
    emit signal_remote_port_can_use(false);

    /* 远程地址是否可选 */
    emit signal_remote_addr_can_use(false);
  }
}

/* 设置自定义波特率, 需要从CANMaster目录下的baudcal生成字符串 */
bool can_driver_zlg::custom_baud_rate_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  char path[50] = {0};
  snprintf(path, sizeof(path), "%d/baud_rate_custom", channel_state.channel_num);
  return 1 == ZCAN_SetValue(device_handle_, path, custom_baudrate_.toUtf8().data());
}

void can_driver_zlg::receive_data(const CHANNEL_STATE_Typedef_t &channel_state)
{
  ZCAN_Receive_Data can_data[CAN_MSG_NUM_MAX];
  ZCAN_ReceiveFD_Data canfd_data[CAN_MSG_NUM_MAX];
  quint32 len;
  /* 获取can数据长度 */
  len = ZCAN_GetReceiveNum(channel_state.channel_handle, TYPE_CAN);
  if(0 < len)
  {
    len = ZCAN_Receive(channel_state.channel_handle, can_data, CAN_MSG_NUM_MAX, 0U);
    show_rec_message(channel_state, can_data, len);
  }

  /* 获取canfd数据长度 */
  len = ZCAN_GetReceiveNum(channel_state.channel_handle, TYPE_CANFD);
  if(0 < len)
  {
    len = ZCAN_ReceiveFD(channel_state.channel_handle, canfd_data, CAN_MSG_NUM_MAX, 0U);
    show_rec_message(channel_state, canfd_data, len);
  }
}

void can_driver_zlg::show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_Receive_Data *data, quint32 len)
{
  quint16 can_id = 0;
  for(quint32 i = 0; i < len; ++i)
  {
    const ZCAN_Receive_Data& can = data[i];
    const canid_t& id = can.frame.can_id;
    const bool is_eff = IS_EFF(id);
    const bool is_rtr = IS_RTR(id);
    can_id = GET_ID(id);

    /* 消息分发到UI显示cq */
    msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_RX_DIRECT,
                     CAN_PROTOCOL_TYPE, is_eff ? EXT_FRAME_TYPE : STD_FRAME_TYPE,
                     is_rtr ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE,
                     can.frame.data, can.frame.can_dlc);

    /* 消息过滤分发 */
    msg_to_cq_buf(can_id, (quint8)channel_state.channel_num, can.frame.data, can.frame.can_dlc);
  }
}

void can_driver_zlg::show_rec_message(const CHANNEL_STATE_Typedef_t &channel_state, const ZCAN_ReceiveFD_Data *data, quint32 len)
{
  quint16 can_id = 0;
  for(quint32 i = 0; i < len; ++i)
  {
    const ZCAN_ReceiveFD_Data& canfd = data[i];
    const canid_t& id = canfd.frame.can_id;
    can_id = GET_ID(id);

    /* 消息分发到UI显示cq */
    msg_to_ui_cq_buf(can_id, (quint8)channel_state.channel_num, CAN_RX_DIRECT,
                     CANFD_PROTOCOL_TYPE, IS_EFF(id) ? EXT_FRAME_TYPE : STD_FRAME_TYPE,
                     IS_RTR(id) ? REMOTE_FRAME_TYPE : DATA_FRAME_TYPE,
                     canfd.frame.data, canfd.frame.len);

    /* 消息过滤分发 */
    msg_to_cq_buf(can_id, (quint8)channel_state.channel_num, canfd.frame.data, canfd.frame.len);
  }
}

bool can_driver_zlg::transmit_type_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  char path[50] = {0};
  char value[100] = {0};
  snprintf(path, sizeof(path), "%d/send_type", channel_state.channel_num);
  snprintf(value, sizeof(value), "%d", send_type_index_);
  return 1 == ZCAN_SetValue(device_handle_, path, value);
}

/* 设置终端电阻使能 */
bool can_driver_zlg::resistance_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  char path[50] = {0};
  snprintf(path, sizeof(path), "%d/initenal_resistance", channel_state.channel_num);
  char value[10] = {0};
  snprintf(value, sizeof(value), "%d", resistance_enable_);
  return 1 == ZCAN_SetValue(device_handle_, path, value);
}

/* 设置CAN卡波特率 */
bool can_driver_zlg::baud_rate_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  qDebug() << "/baud_rate_config";
  char path[50] = {0};
  snprintf(path, sizeof(path), "%d/baud_rate", channel_state.channel_num);
  char value[10] = {0};
  snprintf(value, sizeof(value), "%d", kBaudrate[baud_index_]);
  return 1 == ZCAN_SetValue(device_handle_, path, value);
}

/* 设置USBCANFD卡波特率 */
bool can_driver_zlg::cand_fd_bps_config(const CHANNEL_STATE_Typedef_t &channel_state)
{
  qDebug() << channel_state.channel_num << "/canfd_abit_baud_rate " << kAbitTimingUSB[abit_baud_index_];
  char path[50] = { 0 };
  snprintf(path, sizeof(path), "%d/canfd_abit_baud_rate", channel_state.channel_num);
  char value[10] = { 0 };
  snprintf(value, sizeof(value), "%d", kAbitTimingUSB[abit_baud_index_]);
  int ret_a = ZCAN_SetValue(device_handle_, path, value);

  qDebug() << channel_state.channel_num << "/canfd_dbit_baud_rate " << kDbitTimingUSB[dbit_baud_index_];
  snprintf(path, sizeof(path), "%d/canfd_dbit_baud_rate", channel_state.channel_num);
  snprintf(value, sizeof(value), "%d", kDbitTimingUSB[dbit_baud_index_]);
  int ret_d = ZCAN_SetValue(device_handle_, path, value);
  return 1 == (ret_a && ret_d);
}

void can_driver_zlg::show_tx_queue_available(const CHANNEL_STATE_Typedef_t &channel_state)
{
  // TODO: Add your control notification handler code here

  char path[50] = {0};
  //  char value[100] = {0};
  QString csText;
  snprintf(path, sizeof(path), "%d/get_device_available_tx_count/1", channel_state.channel_num);
  const char* pRet = (const char *)ZCAN_GetValue(device_handle_, path);
  if (pRet)
  {
    quint32 nSpace = *(int *)pRet;
    csText = QString((tr("[%1]queue can use space %2")).arg(channel_state.channel_num).arg(nSpace));
  }
  else
  {
    csText = tr("get queue can use space failed");
  }
  show_message(csText);
}

void can_driver_zlg::show_tx_queue_available()
{
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en)
    {
      show_tx_queue_available(channel_state);
    }
  }
}

void can_driver_zlg::clear_tx_queue(const CHANNEL_STATE_Typedef_t &channel_state)
{
  // TODO: Add your control notification handler code here

  char path[50] = {0};
  char value[100] = {0};
  snprintf(path, sizeof(path), "%d/clear_delay_send_queue", channel_state.channel_num);
  int nRet = ZCAN_SetValue(device_handle_, path, value);
  QString csText;
  csText = QString(tr("[%1]clear tx queue [%2] ").arg(channel_state.channel_num).arg(nRet > 0 ? tr(" ok ") : tr(" failed ")));
  show_message(csText);
}

void can_driver_zlg::clear_tx_queue()
{
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en)
    {
      clear_tx_queue(channel_state);
    }
  }
}

bool can_driver_zlg::set_send_queue_mode(const CHANNEL_STATE_Typedef_t &channel_state, bool en)
{
  // TODO: Add your control notification handler code here

  char path[50] = {0};
  char value[100] = {0};
  int nDelaySendQueueMode = (send_queue_mode == true && en == true);
  snprintf(path, sizeof(path), "%d/set_send_mode", channel_state.channel_num);
  snprintf(value, sizeof(value), "%d", nDelaySendQueueMode);
  int nRet = ZCAN_SetValue(device_handle_, path, value);
  QString csText, csRet;
  csText = (nDelaySendQueueMode ? tr("[%1]open tx queue mode ").arg(channel_state.channel_num) : tr("[%1]close tx queue mode").arg(channel_state.channel_num));
  csRet = QString::asprintf(tr("[%s]").toUtf8().data(), \
            nRet > 0 ? tr(" ok ").toUtf8().data() : tr(" failed ").toUtf8().data());
  show_message(csText + csRet);
  return nDelaySendQueueMode;
}

bool can_driver_zlg::set_send_queue_mode(bool en)
{
  bool ret = false;
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en)
    {
      ret = set_send_queue_mode(channel_state, en);
    }
  }
  return ret;
}

#ifdef Q_OS_WIN
bool can_driver_zlg::is_net_can_type(quint32 type)
{
  return (type == ZCAN_CANETUDP || type == ZCAN_CANETTCP ||
          type == ZCAN_WIFICAN_TCP || type == ZCAN_WIFICAN_UDP ||
          type == ZCAN_CANDTU_NET || type == ZCAN_CANDTU_NET_400);
}

bool can_driver_zlg::is_net_can_fd_type(quint32 type)
{
  return (type == ZCAN_CANFDNET_TCP || type == ZCAN_CANFDNET_UDP ||
          type == ZCAN_CANFDNET_400U_TCP || type == ZCAN_CANFDNET_400U_UDP ||
          type == ZCAN_CANFDWIFI_TCP || type == ZCAN_CANFDWIFI_UDP);
}

bool can_driver_zlg::is_net_tcp_type(quint32 type)
{
  return (type == ZCAN_CANETTCP || type == ZCAN_WIFICAN_TCP ||
          type == ZCAN_CANDTU_NET || type == ZCAN_CANDTU_NET_400 ||
          type == ZCAN_CANFDNET_TCP || type == ZCAN_CANFDNET_400U_TCP ||
          type == ZCAN_CANFDWIFI_TCP );
}

bool can_driver_zlg::is_net_udp_type(quint32 type)
{
  return (type == ZCAN_CANETUDP || type == ZCAN_WIFICAN_UDP ||
          type == ZCAN_CANFDNET_UDP || type == ZCAN_CANFDNET_400U_UDP ||
          type == ZCAN_CANFDWIFI_UDP);
}
#else
/* Linux 仅支持 USB 直连的 3 款设备，无网络/云设备 */
bool can_driver_zlg::is_net_can_type(quint32) { return false; }
bool can_driver_zlg::is_net_can_fd_type(quint32) { return false; }
bool can_driver_zlg::is_net_tcp_type(quint32) { return false; }
bool can_driver_zlg::is_net_udp_type(quint32) { return false; }
#endif

void can_driver_zlg::show_send_mode(const CHANNEL_STATE_Typedef_t &channel_state)
{
  // TODO: Add your control notification handler code here

  QString csText;
  char path[50] = {0};
  snprintf(path, sizeof(path), "%d/get_send_mode/1", channel_state.channel_num);
  const char* pRet = (const char*)ZCAN_GetValue(device_handle_, path);
  if(pRet)
  {
    quint32 nMode = *(int*)pRet;
    csText = QString(tr("[%1] device current mode: [%2] ").arg(channel_state.channel_num).arg(nMode ? tr("tx queue mode") : tr("tx normal mode")));
  }
  else
  {
    csText = tr("[%1]get channel modefailed").arg(channel_state.channel_num);
  }
  show_message(csText);
}

void can_driver_zlg::show_send_mode()
{
  CHANNEL_STATE_Typedef_t channel_state;
  for(qint32 i = 0; i < channel_state_list.size(); i++)
  {
    channel_state = channel_state_list.value(i);
    if(true == channel_state.channel_en)
    {
      show_send_mode(channel_state);
    }
  }
}

void can_driver_zlg::can_frame_packed(ZCAN_Transmit_Data &can_data, quint32 id, quint32 frame_type, const quint8 *data, quint8 size)
{
  bool bDelay = frm_delay_flag_;
  quint32 nDelayTime = frm_delay_time_;

  memset(&can_data, 0, sizeof(can_data));
  if(true && bDelay)
  {
    can_data.frame.__pad |= TX_DELAY_SEND_FLAG;
    can_data.frame.__res0 = (BYTE)(nDelayTime & 0xFF);
    can_data.frame.__res1 = (BYTE)((nDelayTime >> 8) & 0xFF);
  }
  can_data.frame.can_id = MAKE_CAN_ID(id, frame_type, 0, 0);

  quint8 len = size > CAN_MAX_DLEN ? CAN_MAX_DLEN : (BYTE)size;
  memcpy_s(can_data.frame.data, sizeof(can_data.frame.data), data, len);

  can_data.frame.can_dlc = len;
  can_data.transmit_type = send_type_index_;
}

void can_driver_zlg::can_frame_packed(ZCAN_TransmitFD_Data &canfd_data, quint32 id, quint32 frame_type, const quint8 *data, quint8 size)
{
  memset(&canfd_data, 0, sizeof(canfd_data));
  bool bDelay = frm_delay_flag_;
  quint32 nDelayTime = frm_delay_time_;
  if(true && bDelay)
  {
    canfd_data.frame.flags |= TX_DELAY_SEND_FLAG;
    canfd_data.frame.__res0 = (BYTE)(nDelayTime & 0xFF);
    canfd_data.frame.__res1 = (BYTE)((nDelayTime >> 8) & 0xFF);
  }
  canfd_data.frame.can_id = MAKE_CAN_ID(id, frame_type, 0, 0);

  quint8 len = size > CANFD_MAX_DLEN ? CANFD_MAX_DLEN : (BYTE)size;
  for(quint32 i = 0; i < len; i++)
  {
    canfd_data.frame.data[i] = (BYTE)data[i];
  }

  canfd_data.frame.len = len;
  canfd_data.transmit_type = send_type_index_;
  canfd_data.frame.flags |= canfd_exp_index_ ? CANFD_BRS : 0;
}

void can_driver_zlg::can_frame_packed(ZCAN_Transmit_Data &can_data, bool add_delay_flag)
{
  bool ok;
  quint32 id = id_.toUInt(&ok, 16);
  bool bDelay = frm_delay_flag_;
  quint32 nDelayTime = frm_delay_time_;

  memset(&can_data, 0, sizeof(can_data));
  can_data.frame.can_id = MAKE_CAN_ID(id, frame_type_index_, 0, 0);

  QStringList data_list = datas_.split(' ');
  quint8 len = data_list.length() > CAN_MAX_DLEN ? CAN_MAX_DLEN : (BYTE)data_list.length();
  for(quint32 i = 0; i < len; i++)
  {
    can_data.frame.data[i] = (BYTE)data_list[i].toUShort(&ok, 16);
  }

  can_data.frame.can_dlc = len;
  can_data.transmit_type = send_type_index_;
  if(add_delay_flag && bDelay)
  {
    can_data.frame.__pad |= TX_DELAY_SEND_FLAG;
    can_data.frame.__res0 = (BYTE)(nDelayTime & 0xFF);
    can_data.frame.__res1 = (BYTE)((nDelayTime >> 8) & 0xFF);
  }
}

void can_driver_zlg::can_frame_packed(ZCAN_TransmitFD_Data &canfd_data, bool add_delay_flag)
{
  bool ok;
  quint32 id = id_.toUInt(&ok, 16);
  bool bDelay = frm_delay_flag_;
  quint32 nDelayTime = frm_delay_time_;

  memset(&canfd_data, 0, sizeof(canfd_data));
  canfd_data.frame.can_id = MAKE_CAN_ID(id, frame_type_index_, 0, 0);

  QStringList data_list = datas_.split(' ');
  quint8 len = data_list.length() > CANFD_MAX_DLEN ? CANFD_MAX_DLEN : (BYTE)data_list.length();
  for(quint32 i = 0; i < len; i++)
  {
    canfd_data.frame.data[i] = (BYTE)data_list[i].toUShort(&ok, 16);
  }

  canfd_data.frame.len = len;
  canfd_data.transmit_type = send_type_index_;
  canfd_data.frame.flags |= canfd_exp_index_ ? CANFD_BRS : 0;
  if(add_delay_flag && bDelay)
  {
    canfd_data.frame.flags |= TX_DELAY_SEND_FLAG;
    canfd_data.frame.__res0 = (BYTE)(nDelayTime & 0xFF);
    canfd_data.frame.__res1 = (BYTE)((nDelayTime >> 8) & 0xFF);
  }
}


void can_driver_zlg::add_auto_can(quint32 nEnable)
{
  // TODO: Add your control notification handler code here

  ZCAN_AUTO_TRANSMIT_OBJ autoObj;
  memset(&autoObj, 0, sizeof(autoObj));
  autoObj.enable = nEnable;
  autoObj.interval = auto_send_period_;
  autoObj.index = auto_send_index_;
  can_frame_packed(autoObj.obj, false);

  char path[50] = {0};
  snprintf(path, sizeof(path), "%d/auto_send", channel_index_);
  int nRet = ZCAN_SetValue(device_handle_, path, (const char *)&autoObj);
  QString csText;
  csText = QString::asprintf(tr("add CAN timed transmission index:%d enable:%d period:%d ms ID:0x%X [%s] ").toUtf8().data(), \
    autoObj.index, autoObj.enable, autoObj.interval, autoObj.obj.frame.can_id, \
    (nRet ? tr(" ok ").toUtf8().data() : tr(" failed ").toUtf8().data()));
  show_message(csText);
}

void can_driver_zlg::add_auto_can_fd(quint32 nEnable)
{
  // TODO: Add your control notification handler code here

  ZCANFD_AUTO_TRANSMIT_OBJ autoObj;
  memset(&autoObj, 0, sizeof(autoObj));
#ifdef Q_OS_WIN
  autoObj.enable = nEnable;
#endif
  autoObj.interval = auto_send_period_;
#ifdef Q_OS_WIN
  /* Linux 版 ZCANFD_AUTO_TRANSMIT_OBJ 无 index 字段 */
  autoObj.index = auto_send_index_;
#endif
  can_frame_packed(autoObj.obj, false);

  char path[50] = {0};
  snprintf(path, sizeof(path), "%d/auto_send_canfd", channel_index_);
  int nRet =  ZCAN_SetValue(device_handle_, path, (const char*)&autoObj);
  QString csText;
#ifdef Q_OS_WIN
  csText = QString::asprintf(tr("add CANFD timed transmission index:%d enable:%d period:%d ms ID:0x%X [%s] ").toUtf8().data(), \
            autoObj.index, autoObj.enable, autoObj.interval, autoObj.obj.frame.can_id, \
            (nRet ? tr(" ok ").toUtf8().data() : tr(" failed ").toUtf8().data()));
#else
  /* Linux 版 ZCANFD_AUTO_TRANSMIT_OBJ 无 index/enable 字段 */
  csText = QString::asprintf(tr("add CANFD timed transmission period:%d ms ID:0x%X [%s] ").toUtf8().data(), \
            autoObj.interval, autoObj.obj.frame.can_id, \
            (nRet ? tr(" ok ").toUtf8().data() : tr(" failed ").toUtf8().data()));
#endif
  show_message(csText);
}

void can_driver_zlg::auto_send_start()
{
  // TODO: Add your control notification handler code here

  char path[50] = {0};
  snprintf(path, sizeof(path), "%d/apply_auto_send", channel_index_);
  int nRet = ZCAN_SetValue(device_handle_, path,"0");
  QString csText = tr("start timed transmission");
  QString ret_str = nRet ? tr("[ok]") : tr("[failed]");
  csText += ret_str;
  show_message(csText);
}

void can_driver_zlg::auto_send_stop()
{
  // TODO: Add your control notification handler code here

  char path[50] = {0};
  snprintf(path, sizeof(path), "%d/clear_auto_send", channel_index_);
  int nRet = ZCAN_SetValue(device_handle_, path, "0");
  QString csText = tr("stop timed transmission");
  QString ret_str = nRet ? tr("[ok]") : tr("[failed]");
  csText += ret_str;
  show_message(csText);
}

void can_driver_zlg::auto_send_stop_single()
{
  // TODO: Add your control notification handler code here

  if (0 == protocol_index_)
  {
    //CAN
    add_auto_can(0);
  }
  else
  {
    // CANFD
    add_auto_can_fd(0);
  }
}


void can_driver_zlg::add_auto_send()
{
  // TODO: Add your control notification handler code here

  if(0 == protocol_index_)
  {
    //CAN
    add_auto_can(1);
  }
  else
  {
    // CANFD
    add_auto_can_fd(1);
  }
}

void can_driver_zlg::show_dev_auto_send()
{
  // TODO: Add your control notification handler code here

  QString csText;
  quint32 nCount = 0;
  char path[50] = {0};

  //CAN
  snprintf(path, sizeof(path), "%d/get_auto_send_can_count/1", channel_index_);
  const char* pRet = (const char*)ZCAN_GetValue(device_handle_,path);
  if (pRet)
  {
    nCount = *(int *)pRet;
    csText = QString(tr("CAN timed transmission num:%1").arg(nCount));
    show_message(csText);
    if (nCount > 0)
    {
      snprintf(path, sizeof(path), "%d/get_auto_send_can_data/1", channel_index_);
      pRet = (const char*) ZCAN_GetValue(device_handle_,path);
      if (pRet)
      {
        const ZCAN_AUTO_TRANSMIT_OBJ* pData = (ZCAN_AUTO_TRANSMIT_OBJ*)pRet;
        for (quint32 i = 0; i < nCount; i++)
        {
          csText = QString::asprintf(tr("CAN timer index:%d period:%d ms ID:0x%08X").toUtf8().data(), \
                    pData[i].index, pData[i].interval, pData[i].obj.frame.can_id);
          show_message(csText);
        }
      }
      else
      {
        csText = tr("get CAN timed transmission data failed");
        show_message(csText);
        return;
      }
    }
  }
  else
  {
    csText = tr("get CAN timed transmission num failed");
    show_message(csText);
    return;
  }


  //CANFD
  snprintf(path, sizeof(path), "%d/get_auto_send_canfd_count/1", channel_index_);
  pRet = (const char *)ZCAN_GetValue(device_handle_,path);
  if (pRet)
  {
    nCount = *(int*)pRet;
    csText = QString(tr("CANFD timed transmission num:%1").arg(nCount));
    show_message(csText);
    if (nCount > 0)
    {
      snprintf(path, sizeof(path), "%d/get_auto_send_canfd_data/1", channel_index_);
      pRet = (const char *)ZCAN_GetValue(device_handle_, path);
      if (pRet)
      {
        const ZCANFD_AUTO_TRANSMIT_OBJ *pData = (ZCANFD_AUTO_TRANSMIT_OBJ *)pRet;
        for (quint32 i = 0; i < nCount; i++)
        {
#ifdef Q_OS_WIN
          csText = QString::asprintf(tr("CANFD timer index:%d period:%d ms ID:0x%08X").toUtf8().data(),
                    pData[i].index, pData[i].interval, pData[i].obj.frame.can_id);
#else
          /* Linux 版 ZCANFD_AUTO_TRANSMIT_OBJ 无 index 字段，用循环序号代替 */
          csText = QString::asprintf(tr("CANFD timer index:%d period:%d ms ID:0x%08X").toUtf8().data(),
                    i, pData[i].interval, pData[i].obj.frame.can_id);
#endif
          show_message(csText);
        }
      }
      else
      {
        csText = tr("get CANFD timed transmission data failed");
        show_message(csText);
        return;
      }
    }
  }
  else
  {
    csText = tr("get CANFD timed transmission num failed");
    show_message(csText);
    return;
  }
}

/******************************** End of file *********************************/
