# 周立功 Linux 驱动库 (x86_64)

本目录收纳周立功官方提供的 Linux x86_64 驱动二进制与头文件。

## 文件说明

| 文件 | 说明 |
|---|---|
| `libusbcan-4e.so` | USBCAN-4E-U（4 通道）ZCAN API 驱动 |
| `libusbcan-8e.so` | USBCAN-8E-U（8 通道）ZCAN API 驱动 |
| `libusbcanfd800u.so` | USBCANFD-800U（8 通道 CAN FD）ZCAN API 驱动 |
| `libusbcan.so` / `libusbcanfd.so` | 旧 VCI API 驱动（USBCAN1/2、USBCANFD-100U/200U/MINI 用），当前版本暂未接入，仅收纳备用 |
| `include/` | 官方头文件（`zlgcan.h` 为 `zlgcan_usbcan-4e.h` 的拷贝，便于与 Windows 版统一 `#include "zlgcan.h"`） |

## 来源与许可

- 来源：周立功官网 Linux 驱动合集（2026-09-27 从 http://www.zlg.cn/index.php/can/down 下载，合集内标注 x86_64 目录）。
- 专有二进制：这些 `.so` 为周立功专有驱动，未随开源协议发布；仅随本项目源码一同分发用于驱动加载，不做修改。

## 加载方式（重要）

三个 ZCAN 库导出**同名** `ZCAN_*` 符号，**不能同时静态链接**。
本项目在 `can_driver_zlg.cpp` 中按设备类型 `dlopen(RTLD_LOCAL) + dlsym` 动态加载：

- `ZCAN_USBCAN_4E_U` → `libusbcan-4e.so`
- `ZCAN_USBCAN_8E_U` → `libusbcan-8e.so`
- `ZCAN_USBCANFD_800U` → `libusbcanfd800u.so`

搜索顺序：程序所在目录 → 程序目录的 `../lib` → `LD_LIBRARY_PATH`/系统默认路径。
构建时 `qmake` 会把这三个 `.so` 复制到可执行文件输出目录，并设置 `$ORIGIN` rpath。

## 依赖

- 系统 `libusb-1.0-0`（`libusb-1.0.so.0`），Ubuntu：`sudo apt install libusb-1.0-0`
- 运行时访问 USB 设备需要权限，建议添加 udev 规则或以有权限的用户运行。

## 限制

- 仅 x86_64。aarch64 合集里没有这三款设备的新 ZCAN 库（见 `../zlgcan_linux_aarch64/README.md`）。
- 本目录 `libusb-1.0.so` 若存在于官方合集中，可能是架构标注错误的文件，**不要使用**，一律用系统 libusb。
