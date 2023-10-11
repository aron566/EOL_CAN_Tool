/**
 *  @file man.cpp
 *
 *  @date 2023年05月07日 14:36:17 星期天
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
 *
 *  @brief 主窗口.
 *
 *  @details None.
 *
 *  @version v0.0.1 aron566 2023.05.07 14:36 初始版本.
 *
 *  @par 修改日志:
 *  <table>
 *  <tr><th>Date       <th>Version <th>Author  <th>Description
 *  <tr><td>2023-05-07 <td>v0.0.1  <td>aron566 <td>初始版本
 *  <tr><td>2023-05-07 <td>v0.0.2  <td>aron566 <td>eol协议缓冲区增大至4K
 *  <tr><td>2023-05-15 <td>v0.0.3  <td>aron566 <td>完善rcs页面，消息刷新机制更新支持滚轮查询
 *  <tr><td>2023-05-16 <td>v0.0.4  <td>aron566 <td>优化部分代码
 *  <tr><td>2023-05-17 <td>v0.0.5  <td>aron566 <td>优化部分代码
 *  <tr><td>2023-05-21 <td>v0.0.6  <td>aron566 <td>增加天线校准功能与导出2dfft数据功能
 *  <tr><td>2023-05-21 <td>v0.0.7  <td>aron566 <td>适配新EOL协议v0.0.8增加底噪表传输功能
 *  <tr><td>2023-05-22 <td>v0.0.8  <td>aron566 <td>修复上翻显示色彩异常问题
 *  <tr><td>2023-06-01 <td>v0.0.9  <td>aron566 <td>增加帧诊断窗口，优化eol协议栈发送机制，允许发送多种硬件端口
 *  <tr><td>2023-06-05 <td>v0.0.10 <td>aron566 <td>修复析构会导致的问题
 *  <tr><td>2023-06-06 <td>v0.0.11 <td>aron566 <td>增加chip pmic sn看门狗测试接口，报文导出txt接口，增加读取rcs补偿值接口
 *  <tr><td>2023-06-08 <td>v0.0.12 <td>aron566 <td>修复导出csv checksum可能不对问题，增加设置校准模式按钮
 *  <tr><td>2023-06-09 <td>v0.0.13 <td>aron566 <td>增加手动输入crc计算功能
 *  <tr><td>2023-06-12 <td>v0.0.14 <td>aron566 <td>增加数据曲线功能，打印格式：【$%d %d;】
 *  <tr><td>2023-06-13 <td>v0.0.15 <td>aron566 <td>修复信息读写的时对版本写的bug
 *  <tr><td>2023-06-14 <td>v0.0.16 <td>aron566 <td>修复csv表传输，某些字段未传输问题
 *  <tr><td>2023-06-19 <td>v0.0.17 <td>aron566 <td>增加命令终端功能
 *  <tr><td>2023-06-20 <td>v0.0.18 <td>aron566 <td>适配EOL协议v0.0.15版本，增加gpio寄存器，shell寄存器
 *  <tr><td>2023-06-25 <td>v0.0.19 <td>aron566 <td>适配GCcan驱动
 *  <tr><td>2023-06-26 <td>v0.0.20 <td>aron566 <td>手动发送将自动依据发送的字节数分包发送，增加回环工作模式，为通讯配置增加配置文件
 *  <tr><td>2023-06-27 <td>v0.0.21 <td>aron566 <td>适配GCcanfd驱动
 *  <tr><td>2023-06-28 <td>v0.0.22 <td>aron566 <td>修复GCcanfd驱动，初始化波特率设置不对问题，提示信息不对问题
 *  <tr><td>2023-06-29 <td>v0.0.23 <td>aron566 <td>修复GCcanfd驱动，关闭问题避免二次关闭异常
 *  <tr><td>2023-06-29 <td>v0.0.24 <td>aron566 <td>优化帧诊断，屏蔽black信号刷新避免卡顿，修复gc发送帧协议类型不对问题
 *  <tr><td>2023-07-03 <td>v0.0.25 <td>aron566 <td>增加more页面数据保存，统一一个配置文件管理
 *  <tr><td>2023-07-05 <td>v0.0.26 <td>aron566 <td>配置版本号改为下划线
 *  <tr><td>2023-07-10 <td>v0.0.27 <td>aron566 <td>调试窗口应用参数后立即生效
 *  <tr><td>2023-07-14 <td>v0.0.28 <td>aron566 <td>EOL显示校准配置信息显示优化
 *  <tr><td>2023-07-15 <td>v0.0.29 <td>aron566 <td>窗口按钮大小优化
 *  <tr><td>2023-07-21 <td>v0.0.30 <td>aron566 <td>弹窗字体大小修改
 *  <tr><td>2023-07-25 <td>v0.0.31 <td>aron566 <td>mag类型修改为无符号
 *  <tr><td>2023-07-31 <td>v1.0.0  <td>aron566 <td>增加远程更新功能
 *  <tr><td>2023-08-01 <td>v1.0.1  <td>aron566 <td>增加版本检测逻辑
 *  <tr><td>2023-08-02 <td>v1.0.2  <td>aron566 <td>增加启动检查版本，中文路径支持
 *  <tr><td>2023-08-02 <td>v1.0.3  <td>aron566 <td>更新保存文件名逻辑修改
 *  <tr><td>2023-08-03 <td>v1.0.4  <td>aron566 <td>VCAN打印优化，显示在一行，修改配置文件字段统一小写加下划线
 *  <tr><td>2023-08-07 <td>v1.0.5  <td>aron566 <td>更新ZLG 20230728驱动库文件
 *  <tr><td>2023-08-14 <td>v1.0.6  <td>aron566 <td>增加DTC检测清除页面，优化手动发送页面布局
 *  <tr><td>2023-08-18 <td>v1.0.7  <td>aron566 <td>更多页面增加针对can打印的can id限制
 *  <tr><td>2023-08-23 <td>v1.0.8  <td>aron566 <td>eol页面增加通讯通道选择，修复字符显示通道设置异常，提高eol线程空闲时间
 *  <tr><td>2023-08-24 <td>v1.0.9  <td>aron566 <td>修复shell窗口发送按键导致崩溃问题，增加原子操作避免线程多发
 *  <tr><td>2023-08-24 <td>v1.0.10 <td>aron566 <td>增加VCAN测试通道选择，目标列表显示优化
 *  <tr><td>2023-08-30 <td>v1.0.11 <td>aron566 <td>增加DTC故障检测，增加设备地址选择
 *  <tr><td>2023-09-06 <td>v1.0.12 <td>aron566 <td>取消接收can消息的延时100us，增大can缓冲区防止丢失数据，优化卡顿，以及频繁的发送信号
 *  <tr><td>2023-09-07 <td>v1.1.0  <td>aron566 <td>重构can数据接收显示机制，显示流畅优化，避免数据量大导致的界面卡顿
 *  <tr><td>2023-09-08 <td>v1.1.1  <td>aron566 <td>进入shell调试页面自动关闭中文输入法，避免输入异常，增加rcs80测试阈值，格式优化cq代码，优化shell界面
 *  <tr><td>2023-09-08 <td>v1.1.2  <td>aron566 <td>eol协议默认64字节发送，can不支持的设备默认分包发送
 *  <tr><td>2023-09-13 <td>v1.1.3  <td>aron566 <td>增加tscan盒支持
 *  <tr><td>2023-09-15 <td>v1.1.4  <td>aron566 <td>修复gccanfd发送数据长度未对齐导致发送失败问题
 *  <tr><td>2023-09-18 <td>v1.1.5  <td>aron566 <td>显示控件更换为QPlainTextEdit提高性能，删除无效代码
 *  <tr><td>2023-09-19 <td>v1.1.6  <td>aron566 <td>打开设备时关闭品牌选择下拉框
 *  <tr><td>2023-09-20 <td>v1.1.7  <td>aron566 <td>2dfft增加时间计算
 *  <tr><td>2023-09-22 <td>v1.1.8  <td>aron566 <td>发送消息改为异步方式，发送消息增加事件处理否则会异常
 *  <tr><td>2023-09-25 <td>v1.1.9  <td>aron566 <td>修复诊断报文统计unknow direction消息
 *  <tr><td>2023-09-26 <td>v1.1.10 <td>aron566 <td>修复发送表数据会导致信号死锁问题，接收数据过多导致卡死问题
 *  <tr><td>2023-09-27 <td>v1.1.11 <td>aron566 <td>减少内存占用，消息数限制
 *  <tr><td>2023-10-10 <td>v1.1.12 <td>aron566 <td>支持CFL数据类型csv格式
 *  <tr><td>2023-10-11 <td>v1.1.13 <td>aron566 <td>支持v0.0.25协议，增加波形设置用与目标测试
 *  </table>
 */
#include "mainwindow.h"

#include <QApplication>
#include <QTextCodec>

#define PC_SOFTWARE_VERSION       "1.1.13"

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QApplication a(argc, argv);
    a.setApplicationName("EOL CAN Tool");
    a.setApplicationVersion(PC_SOFTWARE_VERSION);
    MainWindow w;
    w.show();
    return a.exec();
}
