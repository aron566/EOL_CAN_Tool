# EOL数据传输协议
[toc]

## 版本修订

| 日期       | 修订内容                                                     | 版本    | 修订人 |
| ---------- | ------------------------------------------------------------ | ------- | ------ |
| 2023.3.7   | 初版                                                         | v0.0.1  | Aron   |
| 2023.5.7   | 增加CANFD帧支持，部分报文格式修改                            | v0.0.2  | Aron   |
| 2023.5.10  | 权限配置修改                                                 | v0.0.3  | Aron   |
| 2023.5.11  | 目标数使用2个字节描述，以适应更多目标输出                    | v0.0.4  | Aron   |
| 2023.5.13  | 修正表信息传输数据字节数改为4字节                            | v0.0.5  | Aron   |
| 2023.5.16  | 修改调试配置，2DFFT数据内容，数据未准备好时返回0xFF，增加底噪数据获取 | v0.0.6  | Aron   |
| 2023.5.17  | 修改调试配置，2DFFT数据内容增加当前配置下TX发波配置，数据未准备好时返回0xFF，增加底噪数据获取 | v0.0.7  | Aron   |
| 2023.5.21  | 增加通道底噪表数据传输协议                                   | v0.0.8  | Aron   |
| 2023.5.23  | 完善EOL测试流程                                              | v0.0.9  | Aron   |
| 2023.5.24  | 增加寄存器：电压读取、芯片SN读取、PMIC SN读取、客户UDS软硬件版本号（DID），软件编码（DID）信息读取，完善调试参数设置定义 | v0.0.10 | Aron   |
| 2023.5.25  | 增加寄存器：获取RDM数值                                      | v0.0.11 | Aron   |
| 2023.5.30  | 增加看门狗测试寄存器，完善RDM数据获取流程、修改0x02寄存器获取的内容 | v0.0.12 | Aron   |
| 2023.6.9   | 完善RDM数据报文说明                                          | v0.0.13 | Aron   |
| 2023.6.16  | 修正RDM返回最大点数说明，增加shell命令用于支持调试           | v0.0.14 | Aron   |
| 2023.6.19  | 增加GPIO的操作寄存器，增加shell命令寄存器                    | v0.0.15 | Aron   |
| 2023.6.25  | 修正通道底噪表头恢复内容的描述                               | v0.0.16 | Aron   |
| 2023.6.29  | 增加DTC检测及清除操作寄存器，增加SPI、I2C测试寄存器          | v0.0.17 | Aron   |
| 2023.7.10  | 修改shell命令字符数固定最大64字符写入                        | v0.0.18 | Aron   |
| 2023.7.25  | 修正magdB为无符号数                                          | v0.0.19 | Aron   |
| 2023.7.26  | 修正DTC检测列表                                              | v0.0.20 | Aron   |
| 2023.8.30  | 更新DTC检测列表增加PPAR，以及说明更新，增加设备地址定义      | v0.0.21 | Aron   |
| 2023.9.7   | 修正TxOrder说明                                              | v0.0.22 | Aron   |
| 2023.9.11  | 增加方向图表的制作说明                                       | v0.0.23 | Aron   |
| 2023.9.12  | 完善points字段说明，增加表数据结构说明                       | v0.0.24 | Aron   |
| 2023.10.10 | 增加CFL数据类型说明，增加配置波形寄存器用于区分校准还是正常工作测试目标 | v0.0.25 | Aron   |
| 2023.12.27 | 修正字段描述                                                 | v0.0.26 | Aron   |
| 2024.1.15  | 增加描述寄存器读取写入时具体字节                             | v0.0.27 | Aron   |
| 2024.1.17  | 修改切换配置ID字段描述，修改获取2D数据回复内容增加版本，数据类型，放大系数信息 | v0.0.28 | Aron   |
| 2024.1.18  | 增加dll动态库接口说明                                        | v0.0.29 | Aron   |
| 2024.1.29  | 完善EOL生产流程准备要求                                      | v0.0.30 | Aron   |
| 2024.2.20  | 完善EOL生产流程                                              | v0.0.31 | Aron   |
| 2024.2.28  | 更正看门狗测试写入数据说明                                   | v0.0.32 | Aron   |
|            |                                                              |         |        |

## 传输硬件

传输硬件配置选择，CANFD：标准帧 数据帧（64 Bytes），也就是每帧报文最大64字节数据长度向下兼容8字节数据长度，**若报文长度超出将采用分包方式**！

```
        /-------/-------/-------/-------/-------/
      /   FF  /  RTR  /  DLC  /   ID  /  DATA /
    /-------/-------/-------/-------/-------/
  / 1 bit / 1 bit / 4 bit / 11 bit/8 Bytes/
/-------/-------/-------/-------/-------/
```

| 字段名   | 数据长度    | 数值                                              |
| -------- | ----------- | ------------------------------------------------- |
| FF       | 1 bit       | 0：标准帧（固定）                                 |
| RTR      | 1 bit       | 0：数据帧（固定）                                 |
| DLC      | 4 bit       | 8：本帧有8个字节数据（固定）                      |
| ID       | 11 bit      | CAN ID（数值 0x157代表上位机使用EOL数据传输协议） |
| ==DATA== | ==8 Bytes== | ==根据`ID`不同而不同==                            |

| ID    | 含义                          | DATA 数据 |
| ----- | ----------------------------- | --------- |
| 0x157 | 上位机使用EOL数据传输协议传输 | 协议数据  |
| 0x257 | 下位机回复上位机的EOL数据     |           |



## ID类型0x157 EOL数据传输协议

### 上位机写入（上位机->雷达）

```
           /------------/------------/------------/------------/------------/------------/-----------/
         /    帧头    /    功能码   /  寄存器地址 /  数据长度  /  变长数据   /   CRC16_L   /  CRC16_H  /
       /------------/------------/------------/------------/------------/-------------/-----------/
     /   2Bytes   /    bit0    /   bit1-7   /   2Bytes   /     ...    /    1Byte    /    1Byte   /
   /------------/------------/------------/------------/------------/-------------/------------/
 /***********C*************R***************C**********************/
PS：
默认小端模式：低8位在前
帧长最小为：8Bytes
总帧长为：7Bytes + 数据长度
```

### 下位机回复写入（雷达->上位机）

```
           /------------/------------/------------/------------/------------/------------/------------/
         /    帧头    /   功能码    /  寄存器地址 / ACK RESULT /   MESSAGE  /  CRC16_L   /   CRC16_H  /
       /------------/------------/------------/------------/------------/------------/------------/
     /   2Bytes   /     bit0   /   bit1-7   /   1Bytes   /   1Bytes   /   1Byte    /    1Byte   /
   /------------/------------/------------/------------/------------/------------/------------/
 /***********C*************R***************C**********************/
PS：
帧长固定为：7Bytes 
```



### 上位机读取（上位机->雷达）

```
           /------------/------------/------------/------------/------------/
         /    帧头    /    功能码   / 寄存器地址  /  CRC16_L   /   CRC16_H  /
       /------------/------------/-------------/------------/------------/
     /   2Bytes   /    bit0    /    bit1-7   /   1Byte    /    1Byte   /
   /------------/------------/-------------/------------/------------/
 /******C********R********C*************/ 
PS：
默认小端模式：低8位在前
帧长固定为：5Bytes
```



### 下位机回复读取（雷达->上位机）

```
           /------------/------------/------------/------------/------------/------------/------------/
         /    帧头    /    功能码   /  寄存器地址 /   数据长度  /   变长数据  /  CRC16_L   /   CRC16_H  /
       /------------/------------/-------------/------------/------------/------------/------------/
     /   2Bytes   /    bit0    /    bit1-7   /   2Bytes   /     ...    /    1Byte   /    1Byte   /
   /------------/------------/-------------/------------/------------/------------/------------/
 /*******************C******************R*************C************/
PS：
默认小端模式：低8位在前
帧长最短为：8Bytes
总帧长为：7Bytes + 数据长度
```




| 字段名             | 数据长度 | 数值                                                         |
| ------------------ | -------- | ------------------------------------------------------------ |
| 帧头（大端）       | 2Bytes   | 0x7A55（其中0x7A代表上位机设备发出的报文，0x55代表通讯的下位机地址（广播），因为此处是大端所以0x7A先传）<br>0x7555（其中0x75代表下位机设备发出的报文，0x55代表通讯的下位机地址（广播），因为此处是大端所以0x75先传）<br/>**地址**：0x55代表广播数据帧所有设备都进行回复<br/>其它地址比如：0x04、0x05、0x06、0x07，只有符合相应的地址的设备进行响应回复 |
| 功能码（RW读写位） | 1Bit     | 0代表写入<br/>1代表读取                                      |
| 数据长度（小端）   | 2Bytes   | unsigned short类型即16位无符号整数，0x0110代表272个字节数据长度，小端模式（0x10先传） |
| CRC16_L            | 1Byte    | 16位crc结果低字节                                            |
| CRC16_H            | 1Byte    | 16位crc结果高字节                                            |
| ACK RESULT         | 1Byte    | 0代表无错误，1出现错误                                       |
| MESSAGE            | 1Bytes   | 原因状态码                                                   |

### 原因状态码

| 状态码 | 说明                       |
| ------ | -------------------------- |
| 0      | *OK，正常*                 |
| 1      | *CRC错误*                  |
| 2      | *读HeaserFlash错误*        |
| 3      | *读表数据Flash错误*        |
| 4      | *写headerFlash错误*        |
| 5      | *写数据Flash错误*          |
| 6      | *头部CRC数据校验错误*      |
| 7      | *非表传输下，普通读写错误* |
| 8      | *擦除Flash错误*            |
| 9      | 表类型错误                 |
| 10     | 表数据过大错误             |
| 11     | 未知命令/寄存器错误        |
### 寄存器列表

| 寄存器（范围0 - 0x7F，Max 128个）                         | 说明                                                         | 不定长数据内容                                               | 模式要求              | R/W读写权限 |
| --------------------------------------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | --------------------- | ----------- |
| 0x00<br/>==假设读写该寄存器时：==读:0x01<br/>写:0x00<br/> | 安全认证请求，安全码在非生产模式下3s定时刷新                 | **随机安全码：**<br/>DATA[0]：Byte0<br/>DATA[1]：Byte1<br/>DATA[2]：Byte2<br/>DATA[3]：Byte3<br/> | 无                    | R           |
| 0x01<br/>读:0x03<br/>写:0x02<br/>                         | 设置工作模式<br/>                                            | **随机安全码：**<br/>DATA[0]：Byte0<br/>DATA[1]：Byte1<br/>DATA[2]：Byte2<br/>DATA[3]：Byte3<br/>**工作模式：**<br/>DATA[4]：<br/>0，正常运行模式<br/>1，生产普通模式<br/>2，生产调试模式 | 无                    | W           |
| 0x02<br/>读:0x05<br/>写:0x04<br/>                         | 读取雷达运行时间、工作模式、配置文件数量、及接收通道数、TX发波次序<br/>字节数 = 6Bytes + 配置ID数量 * 6Bytes | **当前时间戳**，单位秒，无符号32位整数<br/>DATA[0]：Byte0(最低位)<br/>DATA[1]：Byte1<br/>DATA[2]：Byte2<br/>DATA[3]：Byte3(最高位)<br/>**工作模式**：<br/>DATA[4]<br/>**配置ID数量：**<br/>DATA[5]<br/>**配置ID：**<br/>DATA[6]<br/>**该配置ID的通道数量：**<br/>DATA[7]<br/>**TX发波次序：**<br/>DATA[8]：TX0的发波次序，0代表未启用<br/>DATA[9]：TX1的发波次序<br/>DATA[10]：TX2的发波次序<br/>DATA[11]：TX3的发波次序<br/>==剩余配置ID及通道数、TX发波次序数据...== | 生产普通/生产调试模式 | R           |
| 0x03<br/>读:0x07<br/>写:0x06<br/>                         | 读写软硬件版本：硬件版本、软件版本、校准版本、UDS硬件版本、UDS软件版本、UDS Boot版本 | **硬件版本号：**<br/>DATA[0]：Byte0<br/>DATA[1]：Byte1<br/>DATA[2]：Byte2<br/>DATA[3]：Byte3<br/>**软件版本号：**<br/>DATA[4]：Byte0<br/>DATA[5]：Byte1<br/>DATA[6]：Byte2<br/>DATA[7]：Byte3<br/>**校准版本号：**<br/>DATA[8]：Byte0<br/>DATA[9]：Byte1<br/>**UDS硬件版本号：**<br/>DATA[10]：Byte0<br/>DATA[11]：Byte1<br/>DATA[12]：Byte2<br/>DATA[13]：Byte3<br/>DATA[14]：Byte4<br/>**UDS软件版本号：**<br/>DATA[15]：Byte0<br/>DATA[16]：Byte1<br/>DATA[17]：Byte2<br/>DATA[18]：Byte3<br/>DATA[19]：Byte4<br/>DATA[20]：Byte5<br/>**UDS Boot版本号：**<br/>DATA[21]：Byte0<br/>DATA[22]：Byte1<br/>DATA[23]：Byte2<br/>DATA[24]：Byte3<br/>DATA[25]：Byte4<br/>DATA[26]：Byte5<br/>DATA[27]：Byte6 | 生产普通/生产调试模式 | RW          |
| 0x04<br/>读:0x09<br/>写:0x08<br/>                         | VCAN测试<br/>==对VCAN发送，VCAN应答，则测试通过==            | DATA[0]：固定为1                                             | 生产普通/生产调试模式 | W           |
| 0x05<br/>读:0x0B<br/>写:0x0A<br/>                         | SN读写                                                       | **SN号：**<br/>DATA[0 - 28]                                  | 生产普通/生产调试模式 | RW          |
| 0x06<br/>读:0x0D<br/>写:0x0C<br/>                         | 读取MOUNTID，接地返回1，悬空返回0                            | DATA[0]：<br/>代表MOUNTID1的状态<br/>DATA[1]：<br/>代表MOUNTID2的状态<br/> | 生产普通/生产调试模式 | R           |
| 0x07<br/>读:0x0F<br/>写:0x0E<br/>                         | 读写表数据，帧计数为0时传输表信息，其他为数据帧，参考：《传输数据：子表类型》《传输数据：数据帧格式》<br/>表信息传输数据内容分为：公共表信息 + 私有表信息<br/>私有表信息的 **长度** 以及 **校验和** 在公共表信息中描述<br/>私有表信息通常描述特定表的相关信息，格式参考：《传输数据：私有表信息》 | ==表信息：18Bytes + 私有表信息==<br/>**无符号16位帧计数**<br/>DATA[0]：Byte0(低位)帧计数<br/>DATA[1]：Byte1(高位)帧计数<br/>DATA[2]：**类ID固定为：0x66**<br/>DATA[3]：**私有头部校验和**<br/>DATA[4]：**私有头部字节数**<br/>DATA[5]：表*主版本号 v0 - 255*<br/>DATA[6]：表*副版本号 0 - 255*<br/>DATA[7]：表*修订版本号 0 - 255*<br/>DATA[8]：参考《子表类型》<br/>DATA[9]：数据类型<br/>0：代表*加特兰CFX 28位数据格式复数*<br/>1：代表*浮点类型复数*<br/>3：32位浮点类型<br/>2：代表*16位整型复数*<br/>10：代表32位浮点数二进制格式<br/>11：代表*加特兰CFL 32位数据格式复数*<br/>数据长度无符号32位（字节数）：<br/>DATA[10]：Byte0(最低位)<br/>DATA[11]：Byte1<br/>DATA[12]：Byte2<br/>DATA[13]：Byte3(最高位)<br/>CRC32位数值：小端模式<br/>DATA[14]：Byte0(最低位)<br/>DATA[15]：Byte1<br/>DATA[16]：Byte2<br/>DATA[17]：Byte3(最高位)<br/>私有头部信息内容长度依据（私有头部字节数==最大64Bytes==、内容依据表类型）：<br/>《传输数据：私有表信息》 | 生产普通/生产调试模式 | RW          |
| 0x08<br/>读:0x11<br/>写:0x10<br/>                         | 设置需要读取的表                                             | DATA[0]：参考《子表类型》                                    | 生产普通/生产调试模式 | W           |
| 0x09<br/>读:0x13<br/>写:0x12<br/>                         | 设置需要特定帧的表数据（用于丢帧重传）                       | DATA[0]：Byte0(低位)帧计数<br/>DATA[1]：Byte1(高位)帧计数<br/> | 生产普通/生产调试模式 | W           |
| 0x0A<br/>读:0x15<br/>写:0x14<br/>                         | 设置雷达重启                                                 | DATA[0]：<br/>固定为1，雷达回复上位机后，立即进行重启        | 生产普通/生产调试模式 | W           |
| 0x0B<br/>读:0x17<br/>写:0x16<br/>                         | 设置雷达保存数据                                             | DATA[0]：<br/>固定为1，雷达会立即触发保存设置的参数到Flash   | 生产普通/生产调试模式 | W           |
| 0x0C<br/>读:0x19<br/>写:0x18<br/>                         | 获取目标数据，**获取之前应先设置0x0D读取目标数据条件，不设置雷达默认使用首个校准配置ID上报**，数据长度依据目标数量，每个目标都有：速度、方位角度、距离、MAG、RCS、SNR、俯仰角度信息<br/>字节数 = 3Bytes + 目标数量 * 16Bytes<br/>==当数据未准备好时：==<br/>配置ID为0xFF，目标数量为0xFFFF | DATA[0]：配置ID<br/>目标数量<br/>DATA[1]：低字节<br/>DATA[2]：高字节<br/>==目标1的数据：==<br/>**速度**放大100倍，有符号位，数值范围 -32768 +32767真实数值：-327.68 +327.67：<br/>DATA[3]：低字节<br/>DATA[4]：高字节<br/>**方位角**放大100倍，有符号位，数值范围 -16384 ~  +16383真实数值：-163.84 ~ +163.83：<br/>DATA[5]：低字节<br/>DATA[6]：高字节<br/>**距离**放大100倍，无符号位，数值范围 0 +131071真实数值：0 ~ +1310.71：<br/>DATA[7]：Byte0(最低位)<br/>DATA[8]：Byte1<br/>DATA[9]：Byte2<br/>DATA[10]：Byte3(最高位)<br/>**MAG**放大10倍，无符号位，数值范围 0 ~  +2046真实数值：0 ~ +204.6：<br/>DATA[11]：低字节<br/>DATA[12]：高字节<br/>**RCS**放大10倍int16 -32768 ~ 32767dB，真实数值：-3276.8 ~ 3276.7dB：<br/>DATA[13]：低字节<br/>DATA[14]：高字节<br/>**SNR**放大10倍，有符号位，数值范围 -2048 ~  +2046真实数值：-204.8 ~ +204.6：<br/>DATA[15]：低字节<br/>DATA[16]：高字节<br/>**俯仰角**放大100倍，有符号位，数值范围 -16384 ~  +16383真实数值：-163.84 ~ +163.83：<br/>DATA[17]：低字节<br/>DATA[18]：高字节<br/>==剩余目标数据...== | 生产普通/生产调试模式 | R           |
| 0x0D<br/>读:0x1B<br/>写:0x1A<br/>                         | 读写手动切换的配置 ID，仅用于获取特定配置下的目标数据        | DATA[0]：配置ID                                              | 生产普通/生产调试模式 | RW          |
| 0x0E<br/>读:0x1D<br/>写:0x1C<br/>                         | 设置读取2DFFT数据条件（转台电机信息，设置时转台应已经在此位置，不可在转动过程中设置避免产生误差） | DATA[0]：方向：<br/>0：水平<br/>1：俯仰<br/>角度：-128 ~ 127deg<br/>DATA[1]<br/>距离：0 ~ 255m<br/>DATA[2]<br/>速度：-128 ~ 127m/s<br/>DATA[3] | 生产普通/生产调试模式 | W           |
| 0x0F<br/>读:0x1F<br/>写:0x1E<br/>                         | 获取目标2DFFT数据<br/>字节数 = 5Bytes（数据类型及放大系数信息） + 配置数 * 6Bytes + 通道数 * 8Bytes（数据类型为12时，int32复数）<br/>TX发波次序：TX0 = 0x00代表未使用<br/>TX0 = 0x01代表2b00000001第1个时序发射<br/>TX0 = 0x02代表2b00000010第2个时序发射<br/>==当数据未准备好时：==<br/>配置ID（值为0xFF）与通道数（值为0xFF），TX发波次序全0，无2DFFT数据 | DATA[0]：数据类型<br/>12：代表*int32位数据格式复数（实部int32、虚部int32每个数据占8Bytes）*<br/><br/>**数据放大系数n** int32<br/>DATA[1]：Byte0(最低位)<br/>DATA[2]：Byte1<br/>DATA[3]：Byte2<br/>DATA[4]：Byte3(最高位)<br/><br/>DATA[5]：配置ID<br/>DATA[6]：通道数（1 - 32~max~）<br/>TX发波次序：<br/>DATA[7]：TX0的发波次序，0代表未启用<br/>DATA[8]：TX1的发波次序<br/>DATA[9]：TX2的发波次序<br/>DATA[10]：TX3的发波次序<br/>通道0的2DFFT数据，**数值放大n倍后**，实部虚部顺序排列：<br/>实部：<br/>DATA[11]：Byte0(最低位)<br/>DATA[12]：Byte1<br/>DATA[13]：Byte2<br/>DATA[14]：Byte3(最高位)<br/>虚部：<br/>DATA[15]：Byte0(最低位)<br/>DATA[16]：Byte1<br/>DATA[17]：Byte2<br/>DATA[18]：Byte3(最高位)<br/>==剩余通道2DFFT数据...==<br/>==剩余配置下以及该配置下的2DFFT数据...== | 生产普通/生产调试模式 | R           |
| 0x10<br/>读:0x21<br/>写:0x20<br/>                         | 读写RCS标定值<br/>字节数 = 1Byte + 配置数 * 3Bytes           | DATA[0]：配置数<br/>DATA[1]：配置ID<br/>**RCS补偿值**，有符号位，数值放大10倍后int16 -32768 ~ 32767dB，真实数值：-3276.8 ~ 3276.7dB<br/>DATA[2]：低字节<br/>DATA[3]：高字节 <br/>==其他配置ID + RCS补偿值...== | 生产普通/生产调试模式 | RW          |
| 0x11<br/>读:0x23<br/>写:0x22<br/>                         | 重置所有参数（不断电保存）<br/>- *清除RCS补偿值*<br/>- *清除校准版本号*<br/>- *清除SN号*<br/>- *清除DTC*<br/> | DATA[0]：固定为1<br/>                                        | 生产普通/生产调试模式 | W           |
| 0x12<br/>读:0x25<br/>写:0x24<br/>                         | 读写校准模式<br/>字节数 = 1Byte + 配置数 * 2Bytes            | DATA[0]：配置数<br/>DATA[1]：配置ID<br/>DATA[2]：校准模式：<br/>0：一度一校准<br/>1：曲线拟合<br/>==其他配置ID + 校准模式...== | 生产普通/生产调试模式 | RW          |
| 0x13<br/>读:0x27<br/>写:0x26<br/>                         | 设置调试运行参数，参数在断电重启后失效<br/>TX发波次序：TX0 = 0x00代表未使用<br/>TX0 = 0x01代表2b00000001第1个时序发射<br/>TX0 = 0x02代表2b00000010第2个时序发射<br/> | DATA[0]：配置ID<br/>DATA[1]：配置属性：<br/>0xFF：不使用此配置<br/>0x00：正常模式运行使用此配置<br/>0x01：校准模式运行使用此配置<br/>0x02：正常与校准模式运行使用此配置<br/>起始频率GHz，扩大100倍后数值，如76.3GHz写入7630，数值范围：7400 - 8100，真实数值：74GHz ~ 81GHz：<br/>DATA[2]：低字节<br/>DATA[3]：高字节<br/>带宽设置MHz，数值放大100倍，数值范围：0 - 500000，真实数值范围：0 - 5000MHz：<br/>DATA[4]：Byte0(最低位)<br/>DATA[5]：Byte1<br/>DATA[6]：Byte2<br/>DATA[7]：Byte3(最高位)<br/>chirp上升时间，us，数值范围：> 0<br/>DATA[8]：chirp上升时间<br/>chirp下降时间，us，数值范围：> 0<br/>DATA[9]：chirp下降时间<br/>chirp周期，us，数值范围：>= chirp上升时间 + chirp下降时间<br/>DATA[10]：chirp周期<br/>chirp个数，数值范围 > 0，并且< 4096<br/>DATA[11]：低字节<br/>DATA[12]：高字节<br/>TX发波次序：<br/>DATA[13]：TX0的发波次序，0代表未启用<br/>DATA[14]：TX1的发波次序<br/>DATA[15]：TX2的发波次序<br/>DATA[16]：TX3的发波次序<br/><br/>TX0增益控制，数值范围：±127dB：<br/>DATA[17]<br/>TX1增益控制，数值范围：±127dB：<br/>DATA[18]<br/>TX2增益控制，数值范围：±127dB：<br/>DATA[19]<br/>TX3增益控制，数值范围：±127dB：<br/>DATA[20]<br/> | 生产调试              | W           |
| 0x14<br/>读:0x29<br/>写:0x28<br/>                         | 获取所有通道的底噪数据（**关闭RTS**）<br/>字节数 = 配置数 * 2 + 通道数 * 2<br/>==当数据未准备好时，配置ID与通道数返回0xFF== | DATA[0]：配置ID<br/>DATA[0]：通道数<br/>TX发波次序：<br/>DATA[2]：TX0的发波次序，0代表未启用<br/>DATA[3]：TX1的发波次序<br/>DATA[4]：TX2的发波次序<br/>DATA[5]：TX3的发波次序<br/>通道0的底噪数据**dB**，有符号位，放大10倍数值范围 -2048 ~  +2046真实数值：-204.8 ~ +204.6：<br/>DATA[1]：低字节<br/>DATA[2]：高字节<br/>==剩余通道数据...==<br/> | 生产普通/生产调试模式 | R           |
| 0x15<br/>读:0x2B<br/>写:0x2A<br/>                         | 设置立即更新通道底噪数据（**关闭RTS**）<br/>雷达在收到请求后认为RTS已关闭，立即更新底噪数据 | DATA[0]：固定为1<br/>                                        | 生产普通/生产调试模式 | W           |
| 0x16<br/>读:0x2D<br/>写:0x2C<br/>                         | 电压读取<br/>                                                | 功能码（RW读写位）为写入请求时：<br/>DATA[0]：设定需要获取的电压类型**<未定义>**<br/>功能码（RW读写位）为读取请求时：<br/>电压值，数值放大10倍，无符号16位，数值范围：0 - 1023V，真实数值：0 - 102.3V<br/>DATA[0]：低字节<br/>DATA[1]：高字节<br/> | 生产普通/生产调试模式 | RW          |
| 0x17<br/>读:0x2F<br/>写:0x2E<br/>                         | 芯片SN读取<br/>字节数 = 1Byte + SN字节数                     | DATA[0]：SN字节数<br/>DATA[1]：芯片SN信息Byte0<br/>==剩余SN信息...== | 生产普通/生产调试模式 | R           |
| 0x18<br/>读:0x31<br/>写:0x30<br/>                         | PMIC SN读取<br/>字节数 = 1Byte + SN字节数                    | DATA[0]：SN字节数<br/>DATA[1]：PMIC SN信息Byte0<br/>==剩余SN信息...== | 生产普通/生产调试模式 | R           |
| 0x19<br/>读:0x33<br/>写:0x32<br/>                         | 客户UDS软硬件版本号（DID），软件编码（DID）信息读取<br/>客户版本信息可能存储的是字符或者数值形式，格式不一<br/>字节数 = 1Byte + 版本信息长度（最大64Bytes） | DATA[0]：版本信息长度，范围：0 - 64Bytes<br/>==版本信息...== | 生产普通/生产调试模式 | R           |
| 0x1A<br/>读:0x35<br/>写:0x34<br/>                         | RDM数据（区域能量指标）<br/>R：距离，m<br/>D：速度，m/s<br/>==获取数据时，应先设定条件（获取使能、配置ID、距离、速度、通道数）再进行获取，数据未准备好只返回1个字节0xFF，数据准备好后，雷达停止基带，等待数据被取出==<br/>写入字节数 = 10Bytes<br/>首次读取RDM数据：<br/>**帧计数为0时**，返回《RDM数据信息》<br/>数据排列方式：[ 距离bin维度 ] [ 速度bin维度 ]<br/>假设距离bin 512点，速度bin 64点，总RDM数据个数512 * 64：<br/>**帧计数大于0并且小于0xFFFF**，返回一帧**最大64个**《RDM数据》，数据含义：<br/>0 - 63：距离bin为0，64个速度bin下的RDM数据<br/>64 - 127：距离bin为1，64个速度bin下的RDM数据<br/>持续读取，当帧计数为0xFFFF时，代表RDM数据获取结束，雷达启动基带信号处理自动进行下一个RDM数据更新 | 功能码（RW读写位）为写入请求时：<br/>设定RDM数据获取，0关闭，1使能：<br/>DATA[0]<br/>**为1使能时，以下参数有效**<br/>设定需要的数据，配置ID：<br/>DATA[1]<br/>设定需要的数据，距离起始，单位m，无符号16位，数值范围：>= 0：<br/>DATA[2]：低字节<br/>DATA[3]：高字节<br/>设定需要的数据，距离结束，单位m，无符号16位，放大10倍，数值范围：<= 1228：<br/>DATA[4]：低字节<br/>DATA[5]：高字节<br/>设定需要的数据，速度起始，单位m/s，无符号8位，放大10倍，数值范围：>= 0：<br/>DATA[6]<br/>设定需要的数据，速度结束，单位m/s，无符号8位，放大10倍，数值范围：<= 91：<br/>DATA[7]<br/>设定需要的数据，通道起始，无符号8位，数值范围：>= 0：<br/>DATA[8]<br/>设定需要的数据，通道结束，无符号8位，数值范围：<= 16：<br/>DATA[9]<br/>功能码（RW读写位）为读取请求时：<br/>**无符号16位帧计数**<br/>DATA[0]：Byte0(低位)帧计数<br/>DATA[1]：Byte1(高位)帧计数<br/>==帧计数为0《RDM数据信息》==<br/>DATA[2]：配置ID<br/>距离bin起始，无符号16位<br/>DATA[3]：低字节<br/>DATA[4]：高字节<br/>距离bin结束，无符号16位<br/>DATA[5]：低字节<br/>DATA[6]：高字节<br/>距离bin最大值，无符号16位<br/>DATA[7]：低字节<br/>DATA[8]：高字节<br/>速度bin起始，无符号16位<br/>DATA[9]：低字节<br/>DATA[10]：高字节<br/>速度bin结束，无符号16位<br/>DATA[11]：低字节<br/>DATA[12]：高字节<br/>速度bin最大值，无符号16位<br/>DATA[13]：低字节<br/>DATA[14]：高字节<br/>DATA[15]：通道号起始<br/>DATA[16]：通道号结束<br/>TX发波次序：<br/>DATA[17]：TX0的发波次序，0代表未启用<br/>DATA[18]：TX1的发波次序<br/>DATA[19]：TX2的发波次序<br/>DATA[20]：TX3的发波次序<br/>==0 < 帧计数 < 0xFFFF《RDM数据》==<br/>n个（最大64个）平均dB数值，有符号16位，放大10倍：<br/>DATA[2]：低字节<br/>DATA[3]：高字节<br/>==剩余RDM数据...==<br/>==帧计数0xFFFF《结束帧》== | 生产调试模式          | RW          |
| 0x1B<br/>读:0x37<br/>写:0x36<br/>                         | 操作看门狗                                                   | DATA[0]：开关喂狗，0：关闭看门狗 + 允许喂狗，1：开启看门狗 + 禁止喂狗（雷达将通过看门狗复位重启）<br/> | 生产普通/生产调试模式 | W           |
| 0x1C<br/>读:0x39<br/>写:0x38<br/>                         | 操作GPIO                                                     | 功能码（RW读写位）为写入请求时：<br/>DATA[0]：端口号<br/>DATA[1]：设置端口方向，0输入，1输出<br/>DATA[2]：设置输出端口数值<br/>功能码（RW读写位）为读取请求时：<br/>DATA[0]：端口号<br/>DATA[1]：端口方向，0输入，1输出<br/>DATA[2]：端口数值，0低电平，1高电平 | 生产普通/生产调试模式 | RW          |
| 0x1D<br/>读:0x3B<br/>写:0x3A<br/>                         | shell命令交互<br/>设备内部shell控制台在接收到命令时，自动发送回复报文，**无需手动读取** | 功能码（RW读写位）为写入请求时：<br/>写入命令：最大支持1 - 64字符<br/>DATA[0] - DATA[255]<br/>功能码（RW读写位）为读取时：<br/>最大单帧1024字符，无需手动读取，监听即可 | 生产调试模式          | RW          |
| 0x1E<br/>读:0x3D<br/>写:0x3C<br/>                         | SPI操作                                                      | 操作指令：0写入测试，1读取测试<br/>DATA[0]：操作指令<br/>    | 生产普通/生产调试模式 | W           |
| 0x1F<br/>读:0x3F<br/>写:0x3E<br/>                         | I2C操作                                                      | 操作指令：0写入测试，1读取测试<br/>DATA[0]：操作指令<br/>    | 生产普通/生产调试模式 | W           |
| 0x20<br/>读:0x41<br/>写:0x40<br/>                         | DTC故障诊断<br/>每个字节代表一个故障，每个故障分为当前故障与历史故障：<br/>当前故障：bit0置1<br/>历史故障：bit3置1<br/> | 功能码（RW读写位）为写入请求时：<br/>DATA[0]：固定1，清除DTC<br/>功能码（RW读写位）为读取请求时：<br/>DTC状态数值<br/>DATA[0]：CAN0_BUSOFF<br/>DATA[1]：CAN1_BUSOFF<br/>DATA[2]：RADAR_UDER_VOLTAGE<br/>DATA[3]：RADAR_OVER_VOLTAGE<br/>DATA[4]：MCU_OVER_TEMPERATURE<br/>DATA[5]：MISS_ALIGNMENT<br/>DATA[6]：NOT_CALIBRATED<br/>DATA[7]：EOL_ERROR_PPAR_WRITE<br/>==剩余状态数值信息...== | 生产普通/生产调试模式 | RW          |
| 0x21<br/>读:0x43<br/>写:0x42<br/>                         | 手动配置工作波形<br/>0：校准波形（默认）<br/>1：正常工作波形<br/> | 功能码（RW读写位）为写入请求时：<br/>DATA[0]：波形工作模式（0校准，1正常工作）<br/>功能码（RW读写位）为读取请求时：<br/>DATA[0]：当前波形工作模式 | 生产普通/生产调试模式 | RW          |

### 传输数据

#### 子表类型

| 表总类                        | 子表类型                                                     | 私有表信息数据内容                                           | 表数据内容                                                   |
| ----------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 0：*方位导向矢量表*           | 0：配置0方位导向矢量表@ELE+0deg<br/>1：配置1方位导向矢量表@ELE+0deg<br/>2：配置2方位导向矢量表@ELE+0deg<br/>3：配置3方位导向矢量表@ELE+0deg | 起始角度32位浮点数：<br/>DATA[18]：Byte0(最低位)<br/>DATA[19]：Byte1<br/>DATA[20]：Byte2<br/>DATA[21]：Byte3(最高位)<br/>结束角度32位浮点数：<br/>DATA[22]：Byte0(最低位)<br/>DATA[23]：Byte1<br/>DATA[24]：Byte2<br/>DATA[25]：Byte3(最高位)<br/>点数：<br/>DATA[26]：Byte0(低位)<br/>DATA[27]：Byte3(高位)<br/>通道数：<br/>DATA[28]<br/>俯仰角度32位浮点数：<br/>DATA[29]：Byte0(最低位)<br/>DATA[30]：Byte1<br/>DATA[31]：Byte2<br/>DATA[32]：Byte3(最高位)<br/>TX发波次序：<br/>DATA[33]：TX0的发波次序，0代表未启用<br/>DATA[34]：TX1的发波次序<br/>DATA[35]：TX2的发波次序<br/>DATA[36]：TX3的发波次序<br/>DATA[37]：Perofile ID<br/> | [起始角度 通道0 1 2 3 ....校准数据]<br/>...<br/>...<br/>...<br/>[结束角度 通道0 1 2 3 ....校准数据] |
| 1：*俯仰导向矢量表*           | 4：配置0俯仰导向矢量表@AZI+0deg<br/>5：配置1俯仰导向矢量表@AZI+0deg<br/>6：配置2俯仰导向矢量表@AZI+0deg<br/>7：配置3俯仰导向矢量表@AZI+0deg | 起始角度32位浮点数：<br/>DATA[18]：Byte0(最低位)<br/>DATA[19]：Byte1<br/>DATA[20]：Byte2<br/>DATA[21]：Byte3(最高位)<br/>结束角度32位浮点数：<br/>DATA[22]：Byte0(最低位)<br/>DATA[23]：Byte1<br/>DATA[24]：Byte2<br/>DATA[25]：Byte3(最高位)<br/>点数：<br/>DATA[26]：Byte0(低位)<br/>DATA[27]：Byte3(高位)<br/>通道数：<br/>DATA[28]<br/>方位角度32位浮点数：<br/>DATA[29]：Byte0(最低位)<br/>DATA[30]：Byte1<br/>DATA[31]：Byte2<br/>DATA[32]：Byte3(最高位)<br/>TX发波次序：<br/>DATA[33]：TX0的发波次序，0代表未启用<br/>DATA[34]：TX1的发波次序<br/>DATA[35]：TX2的发波次序<br/>DATA[36]：TX3的发波次序<br/>DATA[37]：Perofile ID<br/> | [起始角度 通道0 1 2 3 ....校准数据]<br/>...<br/>...<br/>...<br/>[结束角度 通道0 1 2 3 ....校准数据] |
| 2：*天线间距坐标与初相信息表* | 8：配置0天线间距坐标与初相信息表<br/>9：配置1天线间距坐标与初相信息表<br/>10：配置2天线间距坐标与初相信息表<br/>11：配置3天线间距坐标与初相信息表 | 点数：固定为3（x,y,phase）<br/>DATA[18]：Byte0(低位)<br/>DATA[19]：Byte3(高位)<br/>通道数：<br/>DATA[20]<br/>TX发波次序：<br/>DATA[21]：TX0的发波次序，0代表未启用<br/>DATA[22]：TX1的发波次序<br/>DATA[23]：TX2的发波次序<br/>DATA[24]：TX3的发波次序<br/>DATA[25]：Perofile ID<br/> | [通道0 x，y数据]<br/>[通道1 x，y数据]<br/>...<br/>...<br/>[通道15 x，y数据]<br/>[通道0 phase数据]<br/>[通道1 phase数据]<br/>...<br/>...<br/>[通道15 phase数据]<br/>...<br/> |
| 3：*方向图表*                 | 12：配置0方向图表<br/>13：配置1方向图表<br/>14：配置2方向图表<br/>15：配置3方向图表 | 起始角度32位浮点数：<br/>DATA[18]：Byte0(最低位)<br/>DATA[19]：Byte1<br/>DATA[20]：Byte2<br/>DATA[21]：Byte3(最高位)<br/>结束角度32位浮点数：<br/>DATA[22]：Byte0(最低位)<br/>DATA[23]：Byte1<br/>DATA[24]：Byte2<br/>DATA[25]：Byte3(最高位)<br/>点数：<br/>DATA[26]：Byte0(低位)<br/>DATA[27]：Byte3(高位)<br/>通道数：<br/>DATA[28]<br/>单位：<br/>DATA[29]：<br/>0：米<br/>1：dB<br/>DATA[30]：TX0的发波次序，0代表未启用<br/>DATA[31]：TX1的发波次序<br/>DATA[32]：TX2的发波次序<br/>DATA[33]：TX3的发波次序<br/>DATA[34]：Perofile ID<br/> | [起始角度 方向图数据]<br/>...<br/>...<br/>...<br/>[结束角度 方向图数据] |
| 4：*俯仰导向矢量表@AZI-45deg* | 16：配置0俯仰导向矢量表@AZI-45deg<br/>17：配置1俯仰导向矢量表@AZI-45deg<br/>18：配置2俯仰导向矢量表@AZI-45deg<br/>19：配置3俯仰导向矢量表@AZI-45deg | 同1：*俯仰导向矢量表*                                        | 同1：*俯仰导向矢量表*                                        |
| 5：*俯仰导向矢量表@AZI+45deg* | 20：配置0俯仰导向矢量表@AZI+45deg<br/>21：配置1俯仰导向矢量表@AZI+45deg<br/>22：配置2俯仰导向矢量表@AZI+45deg<br/>23：配置3俯仰导向矢量表@AZI+45deg | 同1：*俯仰导向矢量表*                                        | 同1：*俯仰导向矢量表*                                        |
| 6：通道底噪表                 | 24：所有配置下通道底噪表                                     | 各配置下通道数量：<br/>DATA[18]：配置0通道数量<br/>DATA[19]：配置1通道数量<br/>DATA[20]：配置2通道数量<br/>DATA[21]：配置3通道数量<br/>单位：<br/>DATA[22]：<br/>0：米<br/>1：dB<br/>配置0的发射次序：<br/>DATA[23]：TX0的发波次序，0代表未启用<br/>DATA[24]：TX1的发波次序<br/>DATA[25]：TX2的发波次序<br/>DATA[26]：TX3的发波次序<br/>配置1的发射次序：<br/>DATA[27]：TX0的发波次序，0代表未启用<br/>DATA[28]：TX1的发波次序<br/>DATA[29]：TX2的发波次序<br/>DATA[30]：TX3的发波次序<br/>配置2的发射次序：<br/>DATA[31]：TX0的发波次序，0代表未启用<br/>DATA[32]：TX1的发波次序<br/>DATA[33]：TX2的发波次序<br/>DATA[34]：TX3的发波次序<br/>配置3的发射次序：<br/>DATA[35]：TX0的发波次序，0代表未启用<br/>DATA[36]：TX1的发波次序<br/>DATA[37]：TX2的发波次序<br/>DATA[38]：TX3的发波次序<br/> | [配置 0 通道0 底噪数据]<br/>[配置 0 通道1 底噪数据]<br/>...<br/>...<br/>[配置 0 通道15 底噪数据]<br/>[配置 1 通道0 底噪数据]<br/>[配置 1 通道1 底噪数据]<br/>...<br/>...<br/>[配置 1 通道2 底噪数据]<br/>...<br/>==其他配置其他通道底噪数据== |




#### 数据帧格式

数据帧帧计数，从1开始代表传输数据

帧计数为：0xFFFF为结束帧

| 数据类型               | 格式说明                                                     | 不定长数据内容                                               | R/W读写权限 |
| ---------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ----------- |
| 加特兰CFX 28位数据格式 | CFX为加特兰特有表达浮点复数变为32位整型的数据格式，以用于减小内存使用（由8Bytes -> 4Bytes）1 / 2的空间下降，==每帧传输64个点数据即 64 * 4（1个点4个字节） = 256字节== | DATA[0]：Byte0(低位)帧计数<br/>DATA[1]：Byte1(高位)帧计数<br/>第0个数据：<br/>DATA[3]：Byte0(最低位)<br/>DATA[4]：Byte1<br/>DATA[5]：Byte2<br/>DATA[6]：Byte3(最高位)<br/>中间第1 - 62个数据<br/>最后第63个数据：<br/>DATA[254]：Byte0(最低位)<br/>DATA[255]：Byte1<br/>DATA[256]：Byte2<br/>DATA[257]：Byte3(最高位)<br/> | RW          |
| 浮点复数数据格式       | 使用2个32位浮点数（8 Bytes）来表示一个复数的实部与虚部，实部虚部传输，Byte0，代表浮点数在内存中的高字节部分，==每帧传输32个点数据即 32 * 2（实部虚部） * 4（每个实部虚部为32位浮点） = 256字节== | 同上                                                         | RW          |
| 16位整型复数数据格式   | 使用2个16位整型表示一个复数的实部与虚部，实部虚部传输，==每帧传输64个点数据即 64 * 2（实部虚部） * 2（每个实部虚部为16位） = 256字节== | 同上                                                         | RW          |
| 32位浮点数             | 4个字节表示一个浮点数，==每帧传输64个点数据即 64 * 4（每个32位浮点） = 256字节== | 同上                                                         | RW          |
| 加特兰CFL 32位数据格式 | CFL为加特兰特有表达浮点复数变为32位整型的数据格式，以用于减小内存使用（由8Bytes -> 4Bytes）1 / 2的空间下降，==每帧传输64个点数据即 64 * 4（1个点4个字节） = 256字节== | 同上                                                         | RW          |
| 32位整型复数数据格式   | 使用2个32位整型表示一个复数的实部与虚部，实部虚部传输，==每帧传输32个点数据即 32 * 2（实部虚部） * 4（每个实部虚部为32位） = 256字节== | 同上                                                         | RW          |





## CRC16计算公式

```c
/**
 * @brief modbus CRC计算
 * 
 * @param Data 
 * @param GenPoly 多项式
 * @param CrcData 
 * @return uint16_t 
 */
static uint16_t modbus_crc_cal(uint16_t Data ,uint16_t GenPoly ,uint16_t CrcData)
{
    uint16_t TmpI;
    Data *= 2;
    for(TmpI = 8; TmpI > 0; TmpI--) 
  {
        Data = Data / 2;
        if ((Data ^ CrcData) & 1)
            CrcData = (CrcData / 2) ^ GenPoly;
        else
            CrcData /= 2;
    }
    return CrcData;
}

/**
 * @brief modbusCRC计算
 * 
 * @param data 带入CRC计算的数据起始
 * @param data_len 带入CRC计算的数据长度
 * @return uint16_t 
 */
uint16_t modbus_crc_return(uint8_t *data, uint16_t data_len)
{
    uint16_t temp;
    uint16_t crc_ret = 0xFFFF;
    for (temp = 0; temp < data_len; temp++)
    {
        crc_ret = modbus_crc_cal(data[temp], 0xA001, crc_ret);
    }
    return crc_ret;
}
```

## CRC32计算公式

```c
#define POLYNOMIAL      0x04C11DB7  // 多项式

#define USE_CRC32_TABLE 1/**< 是否使用CRC32查表计算，节省1KB内存 */

/* CRC32表 */
#if USE_CRC32_TABLE
  static const unsigned int crc32_table[] = {
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
    0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
    0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
    0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
    0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
    0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
    0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
    0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
    0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
    0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
    0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
    0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
    0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
    0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
    0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
    0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
    0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
    0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
    0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
    0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
    0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
    0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
    0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
    0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
    0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
    0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
    0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
    0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
    0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
    0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
    0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4,
  };
#else
  static unsigned int crc32_table[256];
#endif



/*
 * Generate the table of CRC remainders for all possible bytes.
 */
void CRC_genCrc32Table(void)
{
#if USE_CRC32_TABLE == 0
    int i, j;
    unsigned int crc_accum;

    for (i = 0; i < 256; i++)
    {
        crc_accum = ((unsigned int)i << 24);

        for (j = 0; j < 8; j++)
        {
            if (crc_accum & 0x80000000L)
            {
                crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
            }
            else
            {
                crc_accum = (crc_accum << 1);
            }
        }

        crc32_table[i] = crc_accum;
    }
#endif
}

/*
 * Update the CRC on the data block one byte at a time.
 */
unsigned int CRC_updateCrc32(unsigned int crc_accum, char *data_blk_ptr, int data_blk_size)
{
    int i, j;

    for (j = 0; j < data_blk_size; j++)
    {
        i = ((int)(crc_accum >> 24) ^ *data_blk_ptr++) & 0xff;
        crc_accum = (crc_accum << 8) ^ crc32_table[i];
    }

    return crc_accum;
}
```

## CRC32计算公式python

```PY
#
#  @file cal_crc_demo.py
#
#  @date 2023年12月15日 14:38:05 星期五
#
#  @author aron566
#
#  @copyright Copyright (c) 2023 aron566 <aron566@163.com>.
#
#  @brief 计算CRC.
#
#  @details None.
#
#  @version v0.0.1 aron566 2023.12.15 14:38 初始版本.
#
#  @par 修改日志:
#  <table>
#  <tr><th>Date       <th>Version <th>Author  <th>Description
#  <tr><td>2023-12-15 <td>v0.0.1  <td>aron566 <td>初始版本
#  </table>
#
#** Package ------------------------------------------------------------------

#** Private constants --------------------------------------------------------
#** Public variables ---------------------------------------------------------
#** Private variables --------------------------------------------------------
crcTable = list(range(256))

csvData = [2558448320,1899434256]
#** Private function prototypes ----------------------------------------------

def generateCrcTable():
    POLYNOMIAL = 0x04c11db7
    for i in range(256):
        crc_accum = i << 24
        for j in range(8):
            if crc_accum & 0x80000000:
                crc_accum = ((crc_accum << 1) & 0xFFFFFFFF) ^ POLYNOMIAL
            else:
                crc_accum = ((crc_accum << 1) & 0xFFFFFFFF)
        crcTable[i] = crc_accum
    return

def calCRC(data):
    crc_accum = 0
    idx = 0
    for v in data:
        idx += 1
        i = ((crc_accum >> 24) ^ v) & 0xFF
        crc_accum = ((crc_accum << 8) & 0xFFFFFFFF) ^ crcTable[i]
    return crc_accum

def intArrayCalcCrc(initArray):
    dbfFactorDataBytesTrans = []
    dbfFactorDataBytesFlash = []
    for dbfFactor in initArray:
        byte0 = dbfFactor & 0xFF
        byte1 = (dbfFactor >> 8) & 0xFF
        byte2 = (dbfFactor >> 16) & 0xFF
        byte3 = (dbfFactor >> 24) & 0xFF
        # 发送的数据顺序
        dbfFactorDataBytesTrans.append(byte3)
        dbfFactorDataBytesTrans.append(byte2)
        dbfFactorDataBytesTrans.append(byte1)
        dbfFactorDataBytesTrans.append(byte0)

        # 计算crc的数据顺序
        dbfFactorDataBytesFlash.append(byte0)
        dbfFactorDataBytesFlash.append(byte1)
        dbfFactorDataBytesFlash.append(byte2)
        dbfFactorDataBytesFlash.append(byte3)
        # 计算该分区数据的CRC，注意按照Flash内存储顺序计算
    generateCrcTable()
    crc = calCRC(dbfFactorDataBytesFlash)
    print(f'CRC: {crc}')
    return crc

intArrayCalcCrc(csvData)
#******************************** End of file ********************************
```



## 表数据写入协议流程

```sequence
上位机->雷达: 读取安全码
雷达->上位机: 回复当前安全码，并暂停3s安全码刷新
上位机->雷达: 进入生产普通模式（设置）
Note right of 雷达: 雷达进入到生产模式后安全码不进行刷新

Note left of 上位机: 生产模式下，可以读取当前的模式，配置数，通道数信息
Note right of 雷达: 雷达回复通道数配置文件数

Note left of 上位机: 上位机检测csv文件的所属表，数据类型，通道数
上位机->雷达: 发送第0x0000号帧，写入表信息

Note left of 雷达: 雷达检测表信息是否正确(表数据长度是否溢出)
雷达-->上位机: 回复OK进入表数据接收状态，回复其他则代表失败

上位机->雷达: 分包发送表数据流
雷达-->上位机: 回复OK本包数据验证OK
Note right of 雷达: 雷达接收的数据量满足4KB时进行擦除并存储

上位机->雷达: 发送帧号0xFFFF代表传输结束
雷达-->上位机: 存储最后的数据到flash区域，以上正常回复OK

上位机->雷达: 退出生产普通模式进入正常运行模式（设置）
雷达-->上位机: 回复OK
```

## RDM数据获取协议流程

```sequence
上位机->雷达: 读取安全码
雷达->上位机: 回复当前安全码，并暂停3s安全码刷新
上位机->雷达: 进入生产调试模式（设置）
Note right of 雷达: 雷达进入到调试模式后安全码不进行刷新

Note left of 上位机: 调试模式下，可以读取当前的模式，配置数，通道数信息
Note right of 雷达: 雷达回复通道数配置文件数

上位机->雷达: 使能RDM数据获取，设定获取的距离维度、速度维度、通道范围

Note right of 雷达: 雷达检测设置的参数是否正确
雷达-->上位机: 回复OK进入RDM数据更新状态，回复其他则代表失败
Note right of 雷达: 更新成功，停止信号处理等待读取

上位机->雷达: 读取RDM数据
雷达-->上位机: 回复单字节0xFF代表数据未准备好

Note left of 上位机: 上位机继续读取
雷达-->上位机: 帧计数0x0000代表RDM信息数据帧

Note left of 上位机: 上位机解析后继续读取
雷达-->上位机: 0<帧计数<0xFFFF代表RDM数据帧

Note left of 上位机: 上位机解析后继续读取，直至读取到帧计数为0xFFFF
Note right of 雷达: 雷达发送完所有RDM数据后，发送0xFFFF帧计数代表结束

雷达-->上位机: 发送帧号0xFFFF代表传输结束
Note right of 雷达: 雷达重启信号处理，准备下一帧RDM数据

上位机->雷达: 继续读取RDM数据或者关闭RDM更新

上位机->雷达: 退出调试模式进入正常运行模式（设置）
雷达-->上位机: 回复OK
```

## 报文demo

```
# 上位机读取0x00寄存器，安全访问码，【01】bit1 - 7为0代表寄存器，读写位bit0为1代表读 【AE 89】crc数值
7A 55 01 AE 89
# 雷达回复，安全访问码，【75 55/id】雷达回复固定帧头 【01】bit1 - 7为0代表寄存器，读写位bit0为1代表读 【04 00】代表数据长度4个字节（16位无符号整型，小端模式低字节在前）【68 56 0A 00】为安全码 【7E F5】为crc数值
75 55 01 04 00 68 56 0A 00 7E F5
```

**获取RDM数据**

```
# 设置读取范围：[使能RDM数据更新]，[读取profile 0]，距离：[0] - [0x4CC](放大十倍，1228m) 速度：[0] - [0x5b](放大十倍，91m/s)，通道：[0] - [16]
7A 55 34 0A 00 01 00 00 00 CC 04 00 5b 00 10 79 73
# 雷达回复 ok
75 55 34 00 00 F9 C9 
# 读取RDM
7A 55 35 AF 5E
# 雷达回复：75 55 35 01 00 FF 19 3E代表数据未准备好
回复0号帧：
75 55 35 15 00 [帧计数 00 00] [配置ID 00] [起始距离bin 00 00] [距离结束bin1024 00 04] [距离binMax1024个 00 04] [速度bin起始 00 00] [速度bin结束64 40 00] [速度binMax64个 40 00] 【通道起始 00】 【通道结束 10】 【Tx0 - Tx3发波配置 01 02 04 08】 B1 83
# 继续读取：7A 55 35 AF 5E
# 雷达返回 
75 55 35 【字节数 82 00】 【帧计数 01 00】 9B 
02 BC FF 8B FF 81 FF A3 
FF 88 FF 78 FF 7F FF 8B 
FF 71 FF 6D FF 79 FF 83 
FF 84 FF 76 FF 75 FF 9B 
FF 6D FF 73 FF 72 FF 5F 
FF 75 FF 5E FF 68 FF 63 
FF 6E FF 6C FF 6D FF 74 
FF 66 FF 62 FF 65 FF 87 
FF 65 FF 62 FF 66 FF 74 
FF 6D FF 6C FF 6E FF 63 
FF 68 FF 5E FF 75 FF 5F 
FF 72 FF 73 FF 6D FF 9B 
FF 75 FF 76 FF 84 FF 83 
FF 79 FF 6D FF 71 FF 8A 
FF 7F FF 78 FF 88 FF A3 
FF 81 FF 8B FF BC FF 23 
6C 
# 继续读取：7A 55 35 AF 5E
# 雷达返回 ....
# 继续读取：7A 55 35 AF 5E
# 雷达返回 0xFFFF结束帧
75 55 35 02 00 【FF FF】 BF CE
```



## 表数据CSV文件格式

所有csv格式的文件，都以3行描述一个表数据：

- 第一行：表信息标题
- 第二行：表信息数值
- 第三行：表数据

**方位DBF因子/俯仰DBF因子**

```
table class, version, data type, data size, data crc, points, channel num, start angle*10, end angle*10, ele angle*10, tx_order, profile_id, check sum
```

**天线间距及补偿**

```
table class, version, data type, data size, data crc, points, channel num, tx_order, profile_id, check sum
```

**天线方向图**

```
table class, version, data type, data size, data crc, points, channel num, start angle*10, end angle*10, unit, tx_order, profile_id, check sum
```

**通道底噪表**

```
table class, version, data type, data size, data crc, channel num0, channel num1, channel num2, channel num3,
unit, tx_order0, tx_order1, tx_order2, tx_order3, check sum */
```



**csv字段说明**

- 《table class》表总类
- 《version》版本号，32位整型表示：0xFF0201，代表v1.2.255，主版本1，副版本2，修订版本255
- 《data type》数据类型
- 《data size》数据个数，非字节数
- 《data crc》数据crc32结果
- 《points》点数，带这个字段的表通常：点数×通道数等于数据个数
- 《channel num》通道数
- 《start angle*10》起始角度经过放大10倍的数
- 《end angle*10》结束角度经过放大10倍的数
- 《ele angle*10》副角度经过放大10倍的数，在方位表中说明俯仰角度状态，在俯仰表中说明方位角度状态
- 《unit》单位
- 《channel num0》配置0的通道数
- 《channel num1》配置1的通道数
- 《channel num2》配置2的通道数
- 《channel num3》配置3的通道数
- 《tx_order0》配置0的发波
- 《tx_order1》配置1的发波
- 《tx_order2》配置2的发波
- 《tx_order3》配置3的发波
- 《tx_order》TX的发波次序
  - 每个TX占1个字节总4个字节组成一个u32整型，小端模式
  - 最低字节为TX0的发波次序，如0x00 00 00 01，代表该配置下仅仅TX0在使用，并且第一个发波
  - 如0x00 00 01 02，代表该配置下TX0和1在使用，并且TX1第一个发波，TX0第二个发波
- 《profile_id》配置ID
- 《check sum》和校验，除check sum字段外，所有表信息数值求和的值

### 方向图表的制作demo

```c
#include <stdio.h>
#include <stdint.h>

#define POLYNOMIAL      0x04C11DB7  // 多项式
#define USE_CRC32_TABLE 1/**< 是否使用CRC32查表计算，节省1KB内存 */

/* CRC32表 */
#if USE_CRC32_TABLE
  static const unsigned int crc32_table[] = {
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
    0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
    0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
    0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
    0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
    0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
    0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
    0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
    0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
    0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
    0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
    0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
    0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
    0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
    0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
    0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
    0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
    0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
    0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
    0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
    0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
    0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
    0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
    0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
    0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
    0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
    0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
    0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
    0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
    0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
    0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4,
  };
#else
  static unsigned int crc32_table[256];
#endif


/*
 * Generate the table of CRC remainders for all possible bytes.
 */
void CRC_genCrc32Table(void)
{
#if USE_CRC32_TABLE == 0
    int i, j;
    unsigned int crc_accum;

    for (i = 0; i < 256; i++)
    {
        crc_accum = ((unsigned int)i << 24);

        for (j = 0; j < 8; j++)
        {
            if (crc_accum & 0x80000000L)
            {
                crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
            }
            else
            {
                crc_accum = (crc_accum << 1);
            }
        }

        crc32_table[i] = crc_accum;
    }
#endif
}

/*
 * Update the CRC on the data block one byte at a time.
 */
unsigned int CRC_updateCrc32(unsigned int crc_accum, char *data_blk_ptr, int data_blk_size)
{
    int i, j;

    for (j = 0; j < data_blk_size; j++)
    {
        i = ((int)(crc_accum >> 24) ^ *data_blk_ptr++) & 0xff;
        crc_accum = (crc_accum << 8) ^ crc32_table[i];
    }

    return crc_accum;
}

int main()
{
  float sysPattern[161] = {0};//写入浮点数方向图-80 - +80
  for(int i = -80; i < 81; i++)
  {
    sysPattern[i + 80] = i;
    printf("%u,", *((uint32_t *)sysPattern + (i + 80)));//输出的数据写入csv表
  }
  uint32_t crc = CRC_updateCrc32(0, (char *)sysPattern, sizeof(float) * 161);//计算出的crc写入csv表的crc字段
  printf("\r\ncrc = %u\r\n", crc);
  return 0;
}
```

> 复制以上代码到[在线c语言执行](https://www.nhooo.com/tool/c/)

## 动态库接口

EOL协议封装处理解析功能

- 提供对读写报文的封装，直接转为硬件需发送的协议数据
- 提供对接收到的数据解析功能，接口返回雷达返回的数据内容，帧头及crc计算由dll处理

```c

/**
 * @brief 将需发送的数据转为eol协议字节
 *
 * @param cmd 命令 @ref EOL_MASTER_CMD_Typedef_t 0写入 1读取
 * @param reg 寄存器地址
 * @param p_data 需发送的数据
 * @param data_len 数据长度
 * @param reply_buf 转换后的数据存储区，用于can/uart/eth发送
 * @param reply_len 转换后需发送的数据字节数
 * @return true 成功
 * @return false 失败
 */
bool eol_master_protocol_send_data_package(uint8_t cmd, uint8_t reg,
  const uint8_t *p_data, uint16_t data_len,
  uint8_t *reply_buf, uint16_t *reply_len);

/**
 * @brief 读取从机数据解析出数据段
 *
 * @param p_data 完整报文数据
 * @param data_len 报文数据长度
 * @param p_reg 寄存器地址
 * @param p_cmd 命令
 * @param p_buf 数据段存储地址
 * @param p_buf_len 数据段数据字节长度存储区
 * @return int32_t 解析结果 0成功 -1：crc失败 -2：等待，报文不全 -3：参数不对
 */
int32_t eol_master_protocol_receive_data_decode(const uint8_t *p_data,
  uint16_t data_len, uint8_t *reg, uint8_t *cmd, uint8_t *p_buf, uint16_t *p_buf_len);

/**
 * @brief 清空接收区
 *
 */
void eol_master_protocol_receive_data_clear(void);

/**
 * @brief 获取dll版本
 *
 * @return uint32_t
 */
uint32_t eol_master_protocol_get_version(void);

```

2DFFT数据的解析、传输及算法库功能

- 解析生成的2DFFT数据csv文件，导出DBF因子表、天线间距表、最大最小功率信息
- 解析出的DBF因子自动转为待发送的一维数组，及发送信息（每包发送字节数、总包数）写入雷达，**结合库提供的对读写报文的封装功能**可直接生成硬件传输数据

```c
/**
 * @brief 2dfft数据转为DBF因子
 *
 * @param params_file_name .ini参数文件绝对地址
 * @param fftdata_file_name fftcsv数据csv绝对地址
 * @param output_file_name 输出校准csv文件地址
 * @param dll_file_name 算法动态库绝对地址
 * @param out_data 输出所有数据（一维数组）
 * @param pack_num 总发送包数
 * @param bytes 总字节数
 * @param per_pack_bytes 每包发送字节数
 * @param out_antenna_main_lobe_rx_max_dB 最大功率 128（最大包含4配置）
 * @param out_antenna_main_lobe_rx_min_dB 最小功率 128（最大包含4配置）
 * @return int32_t 0：成功 -1：参数错误 -2：动态库加载失败 -3：内存分配失败
 */
int32_t alg_dll_port_2dfft2cali_data(const char *params_file_name,
  const char *fftdata_file_name, const char *output_file_name, const char *dll_file_name,
  uint8_t *out_data, uint16_t *pack_num, uint32_t *bytes, uint16_t *out_per_pack_bytes,
  float *out_antenna_main_lobe_rx_max_dB,  float *out_antenna_main_lobe_rx_min_dB);

```





## EOL生产流程

- 确认测试需求规格书
  - 中心频率、带宽、方位、俯仰角度范围
  - 测试FOV最大探测距离、角度、速度
  - 接插件安装方向，转台旋转方向
- 制定检测标准阈值
- 设计测试序列

> 工厂生产测试时角色定义：
>
> 上位机：暗箱
>
> 下位机：雷达

```flow
sn_read=>start: 扫码枪录入雷达SN二维码
ready_test=>start: 雷达放置于夹具中，待启动测试
electric_test=>start: 电气测试，电压电流

read_device_access_code=>start: 读取雷达验证码
set_device_run_mode=>start: 设置雷达工作在生产普通模式

pcb_version_test=>start: pcb码读取验证
clear_flash_test=>start: 清除校准涉及的写入信息
read_device_mode=>start: 读取工作模式、配置文件数量、及接收通道数

id0_pin_test=>start: 设置mound0接地，读1，悬空读0
id1_pin_test=>start: 设置mound1接地，读1，悬空读0
sf_version_test=>start: 读取软件版本号验证
soc_sn_test=>start: socSN验证
pmic_sn_test=>start: pmicSN验证
spi_test=>start: spi测试
i2c_test=>start: i2c测试
vcan_test=>start: VCAN通讯验证
ant_calibration=>start: 天线校准，方位@0deg，俯仰@0deg，俯仰@-45deg，俯仰@+45deg，获取相应2DFFT数据
call_dll_calc_dbf_coeff=>start: 调用dll计算dbf因子、方向图等，生成相应csv log
call_dll_decode_csv=>start: 调用dll解析校准数据总表csv，获取打包后的传输数据
write_ant_calibration=>start: 写入DBF因子（方位、俯仰）、方向图、天线间距相位补偿
sys_pattern_test=>start: 方向图测试，选取所有通道n个角度点，判断该角度下偏差

clear_save_dtc=>start: 清除DTC，保存DTC

reboot_radar=>start: 看门狗停止喂狗，等待重启雷达应用校准参数

read_device_access_code2=>start: 读取雷达验证码
set_device_run_mode2=>start: 设置雷达工作在生产普通模式
set_device_tx_wave_mode=>start: 设置雷达工作在正常波形

set_obj_info_profile_id=>start: 设置需要的目标信息ID（正常运行使用）
read_obj_info=>start: 读取目标的rcs，mag，snr，计算rcs offset

write_calc_rcs_compensation=>start: 计算rcs补偿值，写入
write_cali_mode=>start: 校准模式：拟合/一度一校准，写入
sn_write=>start: SN，写入

hw_reboot_radar=>start: 硬件断电重启雷达

sn_check=>start: SN验证
check_calibration_rsl=>start: 测试校准后、方位、俯仰、rcs、snr、mag

read_background_noise=>start: 读取底噪（关闭rts），导出csv
write_background_noise=>start: 写入底噪数据到雷达

e=>end: 测试ok

sn_read->ready_test->electric_test->read_device_access_code->set_device_run_mode->pcb_version_test->clear_flash_test->read_device_mode->id0_pin_test->id1_pin_test->sf_version_test->soc_sn_test->pmic_sn_test->spi_test->i2c_test->vcan_test->ant_calibration->call_dll_calc_dbf_coeff->sys_pattern_test->call_dll_decode_csv->write_ant_calibration->sys_pattern_test->clear_save_dtc->reboot_radar->read_device_access_code2->set_device_run_mode2->set_device_tx_wave_mode->set_obj_info_profile_id->read_obj_info->write_calc_rcs_compensation->write_cali_mode->sn_write->hw_reboot_radar->check_calibration_rsl->e

```

