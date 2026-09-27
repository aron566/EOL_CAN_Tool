# 同星 (TOSUN) Linux 驱动库 (x86_64)

## 来源

- 官方 GitHub：`TOSUN-Shanghai/libtscandemos`
  （"libTSCAN APIs for TOSUN Hardware, suitable in windows and linux platform"）
- 本目录文件取自该仓库 `lib/linux/` 与 `include/linux/`
- **专有二进制**：`libTSCANApiOnLinux.so` / `libTSH.so` 为同星官方闭源库，
  仅随本项目用于驱动其硬件，请遵守同星许可。

## 内容

| 文件 | 说明 |
|---|---|
| `lib/libTSCANApiOnLinux.so` | TSCAN API 主库 (x86_64, 约 7.2MB) |
| `lib/libTSH.so` | 配套硬件抽象库 |
| `include/TSCANDef.hpp` | 官方新版跨平台头文件 (类型/结构体/枚举/回调) |
| `include/tscan_linux_types.h` | 兼容头：补上 `TSCANLINApi.cpp` 所需的 `tscan_*_t` / `tsdiag_*_t` 函数指针类型（新官方头里没有，老 Windows 头里有；已去掉 `__stdcall`） |

## 系统依赖

`ldd` 实测仅依赖 libc / libdl / libpthread / libm / libgcc，**不需要 libusb**，
主流发行版开箱即用。

## 加载方式

`TSCANLINApi.cpp` 本来就用 `QLibrary` 动态加载（Windows 下加载 `ts_can_x64/libTSCAN.dll`），
Linux 下改为加载与可执行文件同目录的 `libTSCANApiOnLinux.so` / `libTSH.so`
（构建时由 `tscan_linux.pri` 的 `QMAKE_POST_LINK` 自动复制，`$ORIGIN` rpath）。

## 支持的设备 / 限制

- 仅 **x86_64**，无 aarch64 版本。
- 覆盖 TSCAN API 设备，如 TC1014 系列（项目驱动设备表 `TS_USBCANFD_1014`）。
- 新一代 TC 系列（如 TC114）在 Linux 下是**免驱 SocketCAN 设备**，
  走项目自带的 SocketCAN 驱动即可，无需本库。
- 无硬件实测：仅验证到库加载与符号解析，收发待真机验证。
