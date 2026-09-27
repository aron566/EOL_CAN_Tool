# EOL_CAN_Tool Linux 构建说明

> 分支：`feat/linux-build`（基于 `fix/p0-crash-20260927`）
> 构建方案：沿用 qmake，`.pro`/`.pri` 中用 `win32` / `unix:!macx` 作用域区分平台；
> Windows 构建逻辑未做任何改动（仅把原有行包进 `win32` 作用域）。

## 1. 平台支持现状

| 模块 | Linux 状态 | 说明 |
|---|---|---|
| 主程序 / 全部业务窗口 | ✅ 可用 | 纯 Qt，无平台相关代码 |
| SocketCAN 驱动（新增） | ✅ 可用 | 基于 Qt6 SerialBus 自带 socketcan 插件，Linux 下默认 CAN 通路 |
| 周立功 / 同星 / Kvaser / 广成驱动 | ❌ 不可用 | 仓库内只有 Windows DLL/.lib，无 Linux 版 SDK；Linux 下编译排除，UI 不显示 |
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

1. 启动程序，设备"品牌"下拉框只会显示 **SocketCAN**（厂商驱动在 Linux 下不可用，已隐藏）。
2. "设备型号"下拉框显示系统中的 socketcan 接口（如 `can0`、`vcan0`）。
3. 波特率/工作模式等选项会被禁用——这是有意的：波特率必须通过 `ip link` 预先设置，
   若接口未 up，打开设备时会给出中文指引（含具体命令）。
4. 其余功能（EOL 标定、固件升级、RTS 控制、网络调试、曲线绘制等）与 Windows 版一致。

## 4. 打包

Windows 用 NSIS（`scripts/package_nsis.bat`，仅 Windows）。
Linux 建议用 linuxdeploy 打 AppImage，或 CPack 打 deb——尚未实现，欢迎补充。

## 5. 给开发者的说明

- 新增文件：`can_driver/can_driver_socketcan.h`、`.cpp`（`unix:!macx` 作用域编译）。
- `utilities/utility.h` 内对非 Windows 平台提供内联 `memcpy_s` 兼容实现
  （glibc 无此函数；语义与 MSVC 一致：超限返回 `EINVAL` 且目标清零）。
- `mainwindow.cpp` 中 4 处驱动工厂 switch 用 `#ifdef Q_OS_WIN` 包裹；
  Linux 下统一 `new can_driver_socketcan()`。品牌下拉框在 Linux 下 `blockSignals`
  后重填，避免 `clear()` 信号级联导致启动崩溃。
- 如需验证 Windows 构建未受影响：在 Windows 上用原有流程 `qmake` + MSVC 构建即可，
  本次改动对 `win32` 分支是纯包裹、零语义改动。
