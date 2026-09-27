# EOL_CAN_Tool Linux 构建说明

> 分支：`feat/linux-build`（基于 `fix/p0-crash-20260927`）
> 构建方案：沿用 qmake，`.pro`/`.pri` 中用 `win32` / `unix:!macx` 作用域区分平台；
> Windows 构建逻辑未做任何改动（仅把原有行包进 `win32` 作用域）。

## 1. 平台支持现状

| 模块 | Linux 状态 | 说明 |
|---|---|---|
| 主程序 / 全部业务窗口 | ✅ 可用 | 纯 Qt，无平台相关代码 |
| SocketCAN 驱动（新增） | ✅ 可用 | 基于 Qt6 SerialBus 自带 socketcan 插件，Linux 下默认 CAN 通路 |
| 周立功驱动（Linux 新增） | ✅ 部分可用 | 仅 x86_64；支持 USBCAN-4E-U / USBCAN-8E-U / USBCANFD-800U，官方 `.so` 经 `dlopen` 动态加载（见下） |
| 同星 / Kvaser / 广成驱动 | ❌ 不可用 | 仓库内只有 Windows DLL/.lib，无 Linux 版 SDK；Linux 下编译排除，UI 不显示 |
| libhv（网络库） | ⚠️ 需自编译 | 仓库只有 Windows 二进制；Linux 下需从源码编译安装 `libhv.so` |
| binlog / BLF | ❌ 已排除 | 仓库无调用方（死代码），Linux 下不编译 |
| 自动更新（QSimpleUpdater） | ⚠️ 部分可用 | 检查/下载跨平台；Linux 下"下载后自动安装"被禁用（原逻辑是生成 `.bat` 调 `cmd.exe`），会提示手动安装 |
| 串口 / 网络调试 | ✅ 可用 | 纯 `QSerialPort` / QtNetwork，无 Win32 API |

已知限制：
- Qt 的 socketcan 插件**不能下发波特率**，CAN 接口必须事先用 `ip link` 配置并 up（见下）。
- Kvaser 官方有 Linux SDK（`libcanlib.so`），但 API 与仓库内 Windows 版有差异，作为第二阶段工作，当前版本暂不支持。

## 2. Ubuntu 构建步骤

### 2.1 安装依赖

Ubuntu 24.04：

```bash
sudo apt update
sudo apt install -y build-essential cmake \
  qt6-base-dev qt6-tools-dev \
  qt6-serialport-dev qt6-serialbus-dev \
  qt6-charts-dev qt6-svg-dev qt6-multimedia-dev qt6-5compat-dev \
  libgl1-mesa-dev
```

Ubuntu 22.04：包名略有不同（`libqt6serialport6-dev`、`libqt6serialbus6-dev` 等），其余同上。

### 2.2 编译安装 libhv（网络功能需要）

```bash
git clone https://github.com/ithewei/libhv.git
cd libhv && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
sudo ldconfig
```

### 2.3 配置 CAN 接口（SocketCAN）

```bash
# 实体 CAN 口（示例 can0，500K 波特率）
sudo ip link set can0 up type can bitrate 500000

# 无硬件时可用虚拟 CAN 口测试
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set vcan0 up
```

### 2.4 编译本项目

```bash
cd EOL_CAN_Tool
qmake6 EOL_CAN_Tool.pro
make -j$(nproc)
```

产物在 `../../bin/`（`unix:!macx: DESTDIR` 配置），可执行文件名为 `EOL_CAN_Tool`
（`TARGET` 未显式指定时默认为工程名）。

## 3. 使用说明（Linux）

1. 启动程序，设备"品牌"下拉框显示 **SocketCAN**（默认，index 0）和 **ZLG**（仅 x86_64）。
   选择 ZLG 后，"设备型号"下拉框显示 `ZCAN_USBCAN_4E_U` / `ZCAN_USBCAN_8E_U` / `ZCAN_USBCANFD_800U`。
2. SocketCAN："设备型号"下拉框显示系统中的 socketcan 接口（如 `can0`、`vcan0`）。
   波特率/工作模式等选项会被禁用——这是有意的：波特率必须通过 `ip link` 预先设置，
   若接口未 up，打开设备时会给出中文指引（含具体命令）。
3. 周立功（Linux）：见下节。
4. 其余功能（EOL 标定、固件升级、RTS 控制、网络调试、曲线绘制等）与 Windows 版一致。

## 4. 周立功 Linux 驱动（x86_64）

### 4.1 支持的设备

| 设备型号 | 通道数 | 类型 | 对应驱动库 |
|---|---|---|---|
| `ZCAN_USBCAN_4E_U` | 4 | CAN | `libusbcan-4e.so` |
| `ZCAN_USBCAN_8E_U` | 8 | CAN | `libusbcan-8e.so` |
| `ZCAN_USBCANFD_800U` | 8 | CAN FD | `libusbcanfd800u.so` |

- 驱动库来自周立功官网 Linux 驱动合集（`zlg_can_lib/zlgcan_linux_x86_64/`，含头文件与 README），
  为周立功专有二进制，随源码分发仅用于驱动加载。
- 三个库导出**同名** `ZCAN_*` 符号，不能静态链接；程序按设备类型 `dlopen(RTLD_LOCAL) + dlsym`
  动态加载。构建时 `qmake` 会把三个 `.so` 复制到可执行文件目录（`$ORIGIN` rpath），
  运行时也可放在 `LD_LIBRARY_PATH` 或 `/usr/local/lib`、`/usr/lib` 下。
- aarch64 暂不支持：官方 aarch64 合集里没有这三款设备的新 ZCAN 库（只有旧 VCI 库），
  对应文件收纳在 `zlg_can_lib/zlgcan_linux_aarch64/` 备用，aarch64 构建不编译周立功驱动。

### 4.2 系统依赖与权限

```bash
sudo apt install -y libusb-1.0-0   # 驱动库依赖 libusb-1.0.so.0
```

访问 USB 设备需要权限，建议添加 udev 规则（示例，`0525` 为周立功常见 VID，请按实际设备调整）：

```bash
# /etc/udev/rules.d/99-zlgcan.rules
SUBSYSTEM=="usb", ATTR{idVendor}=="0525", MODE="0666"
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### 4.3 功能与限制

- 标准波特率：4E/8E 经 `IProperty::SetValue("info/channel/channel_N/baud_rate")` 配置（已核实路径）；
  800U 走 CANFD 初始化流程（`ZCAN_CHANNEL_INIT_CONFIG.canfd.*` + 仲裁/数据波特率属性）。
- 暂不支持：自定义波特率、定时发送（自动发送）、发送队列/队列延迟模式、终端电阻配置、
  `ZCAN_GetValue` 诊断类查询（Linux 侧返回字符串，与 Windows 语义不同，仅透传）。
  不支持的功能在界面上会被禁用，或在调用时返回失败并给出中文提示。
- 真机收发尚未验证：当前版本仅验证到驱动库加载（`dlopen` + 全部符号绑定成功）与程序无崩溃；
  有硬件后需实测打开设备、收发、波特率切换。

## 5. 同星 Linux 驱动（x86_64）

### 5.1 支持的设备

| 设备型号 | 通道数 | 类型 | 对应驱动库 |
|---|---|---|---|
| `TS_USBCANFD_1014` | 4 | CAN FD | `libTSCANApiOnLinux.so` |

- 驱动库来自同星官方 GitHub（`TOSUN-Shanghai/libtscandemos` 的 `lib/linux/` 与 `include/linux/`），
  收纳在 `ts_can_lib/tscan_linux_x86_64/`（含头文件与 README），为同星专有二进制。
- 与周立功不同：单个 `.so` 即可，`TSCANLINApi.cpp` 本来就用 `QLibrary` 动态加载
  （Windows 下加载 `ts_can_x64/libTSCAN.dll`），Linux 下改为加载与可执行文件同目录的
  `libTSCANApiOnLinux.so` / `libTSH.so`。构建时 `qmake` 会把两个 `.so` 复制到可执行文件目录
  （`$ORIGIN` rpath），运行时也可放在 `LD_LIBRARY_PATH` 下。
- 官方 Linux 头（`TSCANDef.hpp`）已跨平台，但缺少 `TSCANLINApi.cpp` 所需的
  `tscan_*_t` / `tsdiag_*_t` 函数指针类型，且 `TS_APP_CHANNEL` 改名为 `APP_CHANNEL`、
  `TLibCAN(FD).FProperties/FData` 从 union 改为普通字段；兼容层
  `tscan_linux_x86_64/include/tscan_linux_types.h` 补齐了这些差异，
  `can_driver_ts.cpp` 的 Linux 分支据此做了适配（Windows 代码零改动）。
- 仅 x86_64，官方无 aarch64 版本；aarch64 构建不编译同星驱动。
- 新一代 TC 系列（如 TC114）在 Linux 下是**免驱 SocketCAN 设备**，
  走项目自带的 SocketCAN 驱动即可，无需本库。

### 5.2 系统依赖

```bash
ldd libTSCANApiOnLinux.so   # 仅依赖 libc/libdl/libpthread/libm/libgcc，无需 libusb
```

### 5.3 功能与限制

- 真机收发尚未验证：当前版本仅验证到驱动库加载（`dlopen` + 全部符号绑定成功）与程序无崩溃；
  有硬件后需实测打开设备、收发、波特率切换。

## 6. 打包

Windows 用 NSIS（`scripts/package_nsis.bat`，仅 Windows；CI 见 `scripts/package_ci.sh`）。

Linux 用 `scripts/package_linux.sh` 打 **AppImage** 单文件安装包
（CI 在 `.github/workflows/build.yml` 的 `build-linux` 任务中自动执行）：

```bash
# 先按第 2 节构建,产物在 $GITHUB_WORKSPACE/bin(或本地 ~/bin)
bash scripts/package_linux.sh 1.5.0 ~/bin
# 产物: dist/EOL_CAN_Tool-1.5.0-x86_64.AppImage
```

打包要点（脚本内已处理）：

- 用 `linuxdeploy` + `linuxdeploy-plugin-qt` 把 Qt 6 依赖打进 AppImage，
  图标/桌面项见 `scripts/EOL_CAN_Tool.desktop`；
- 厂商驱动 `.so`（周立功 `libusbcan-*.so` x3、同星
  `libTSCANApiOnLinux.so`/`libTSH.so`）必须与主程序**同目录**
  （主程序 `RUNPATH=$ORIGIN`），脚本把它们复制到 `AppDir/usr/bin/`，
  而不是 `usr/lib/`；
- GitHub Release 同时发布 Windows(`EOL_CAN_Tool_Setup_v*.exe`)与
  Linux(`EOL_CAN_Tool-*-x86_64.AppImage`)两个安装包，`packge_release/updates.json`
  的 `windows`/`linux` 节分别记录各自版本、下载链接与 changelog，
  打 tag 前两节都要更新到与 tag 一致（CI 会校验）。

## 7. 给开发者的说明

- 新增文件：`can_driver/can_driver_socketcan.h`、`.cpp`（`unix:!macx` 作用域编译）。
- `utilities/utility.h` 内对非 Windows 平台提供内联 `memcpy_s` 兼容实现
  （glibc 无此函数；语义与 MSVC 一致：超限返回 `EINVAL` 且目标清零）。
- `mainwindow.cpp` 中 4 处驱动工厂 switch 用 `#ifdef Q_OS_WIN` 包裹；
  Linux 下按品牌下拉框 index 映射：`0 → can_driver_socketcan`，`1 → can_driver_zlg`
  （仅 x86_64，`ZLG_CAN_LINUX_SUPPORT` 宏控制），`2 → can_driver_ts`
  （仅 x86_64，`TSCAN_CAN_LINUX_SUPPORT` 宏控制）。品牌下拉框在 Linux 下 `blockSignals`
  后重填为 `["SocketCAN", "ZLG", "TOSUN"]`（x86_64）或 `["SocketCAN"]`（其他架构），
  避免 `clear()` 信号级联导致启动崩溃；
  `read_cfg()` 中 Linux 默认保持 index 0（SocketCAN），并对旧配置的品牌索引做越界回退。
- 周立功 Linux 驱动（`can_driver_zlg.cpp` 的 `#else` 分支）：
  - 设备表 `kDeviceType` 在 Linux 下仅保留 3 款 dlopen 支持的设备；
  - `ZCAN_*` 调用经宏重定向到 `dlopen` 绑定后的函数指针（`g_zlgApi`），
    宏定义必须位于 `#include "can_driver_zlg.h"` 之后；
  - Linux 库不导出 `ZCAN_SetValue/GetValue`，改经 `IProperty::SetValue/GetValue` 实现，
    Windows 路径 `"N/属性"` 翻译为 `"info/channel/channel_N/属性"`；
    `ZCAN_TransmitFD/ReceiveFD` 在 4E/8E 库中不存在，按可选符号绑定 + 调用前判空；
  - Linux 头文件缺失的宏（`MAKE_CAN_ID`/`GET_ID`/`TX_DELAY_SEND_FLAG`/`ZCAN_USBCANFD_800U`）
    在兼容层中按 Windows 头文件取值补齐；
  - `ZCAN_USBCANFD_800U` 在 Linux 下归类为 CAN FD 设备（`usbcanfd`），走 CANFD 初始化流程；
  - `ZCLOUD_*` 调用、`is_net_*` 系列、网络/云/PCIE 相关分支在 Linux 下禁用或恒为 false。
- 如需验证 Windows 构建未受影响：在 Windows 上用原有流程 `qmake` + MSVC 构建即可，
  本次改动对 `win32` 分支是纯包裹、零语义改动。
