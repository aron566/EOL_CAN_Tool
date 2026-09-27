# 周立功 Linux 驱动库 (aarch64)

本目录收纳周立功官方提供的 Linux aarch64 驱动二进制。

## 文件说明

| 文件 | 说明 |
|---|---|
| `libusbcan.so` | 旧 VCI API 驱动（USBCAN1/2 用，aarch64） |
| `libusbcanfd.so` | 旧 VCI API 驱动（USBCANFD 系列用，aarch64） |

## 当前状态：暂未接入

官方 aarch64 合集里**只有**旧 VCI API 的 `libusbcan.so` / `libusbcanfd.so`，
没有 x86_64 合集中 `USBCAN-4E-U` / `USBCAN-8E-U` / `USBCANFD-800U` 对应的新 ZCAN 库，
也没有可用的 aarch64 头文件。

因此当前版本在 aarch64 上**不编译**周立功驱动（`zlgcan_linux.pri` 仅在 x86_64 生效），
aarch64 的 CAN 通路只有 SocketCAN。这些文件先收纳在此处，待官方提供
aarch64 ZCAN 库或决定接入旧 VCI API 时再启用。

## 来源与许可

- 来源：周立功官网 Linux 驱动合集（2026-09-27 从 http://www.zlg.cn/index.php/can/down 下载）。
- 专有二进制：这些 `.so` 为周立功专有驱动，未随开源协议发布；仅收纳备用。
