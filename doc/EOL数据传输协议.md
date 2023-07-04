# EOL数据传输协议

## 版本修订

| 日期      | 修订内容                                                     | 版本    | 修订人 |
| --------- | ------------------------------------------------------------ | ------- | ------ |
| 2023.3.7  | 初版                                                         | v0.0.1  | Aron   |
| 2023.5.7  | 增加CANFD帧支持，部分报文格式修改                            | v0.0.2  | Aron   |
| 2023.5.10 | 权限配置修改                                                 | v0.0.3  | Aron   |
| 2023.5.11 | 目标数使用2个字节描述，以适应更多目标输出                    | v0.0.4  | Aron   |
| 2023.5.13 | 修正表信息传输数据字节数改为4字节                            | v0.0.5  | Aron   |
| 2023.5.16 | 修改调试配置，2DFFT数据内容，数据未准备好时返回0xFF，增加底噪数据获取 | v0.0.6  | Aron   |
| 2023.5.17 | 修改调试配置，2DFFT数据内容增加当前配置下TX发波配置，数据未准备好时返回0xFF，增加底噪数据获取 | v0.0.7  | Aron   |
| 2023.5.21 | 增加通道底噪表数据传输协议                                   | v0.0.8  | Aron   |
| 2023.5.23 | 完善EOL测试流程                                              | v0.0.9  | Aron   |
| 2023.5.24 | 增加寄存器：电压读取、芯片SN读取、PMIC SN读取、客户UDS软硬件版本号（DID），软件编码（DID）信息读取，完善调试参数设置定义 | v0.0.10 | Aron   |
| 2023.5.25 | 增加寄存器：获取RDM数值                                      | v0.0.11 | Aron   |
| 2023.5.30 | 增加看门狗测试寄存器，完善RDM数据获取流程、修改0x02寄存器获取的内容 | v0.0.12 | Aron   |
| 2023.6.9  | 完善RDM数据报文说明                                          | v0.0.13 | Aron   |
| 2023.6.16 | 修正RDM返回最大点数说明，增加shell命令用于支持调试           | v0.0.14 | Aron   |
| 2023.6.19 | 增加GPIO的操作寄存器，增加shell命令寄存器                    | v0.0.15 | Aron   |
| 2023.6.25 | 修正通道底噪表头恢复内容的描述                               | v0.0.16 | Aron   |
| 2023.6.29 | 增加DTC检测及清除操作寄存器，增加SPI、I2C测试寄存器          | v0.0.17 | Aron   |
|           |                                                              |         |        |

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
| 帧头（大端）       | 2Bytes   | 0x7A55（其中0x7A代表上位机设备发出的报文，0x55代表通讯的下位机地址，因为此处是大端所以0x7A先传）<br>0x7555（其中0x75代表下位机设备发出的报文，0x55代表通讯的下位机地址，因为此处是大端所以0x75先传） |
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

| 寄存器（范围0 - 0x7F，Max 128个） | 说明                                                         | 不定长数据内容                                               | 模式要求              | R/W读写权限 |
| --------------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | --------------------- | ----------- |
| 0x00                              | 安全认证请求，安全码在非生产模式下3s定时刷新                 | **随机安全码：**<br/>DATA[0]：Byte0<br/>DATA[1]：Byte1<br/>DATA[2]：Byte2<br/>DATA[3]：Byte3<br/> | 无                    | R           |
| 0x01                              | 设置工作模式                                                 | **随机安全码：**<br/>DATA[0]：Byte0<br/>DATA[1]：Byte1<br/>DATA[2]：Byte2<br/>DATA[3]：Byte3<br/>**工作模式：**<br/>DATA[4]：<br/>0，正常运行模式<br/>1，生产普通模式<br/>2，生产调试模式 | 无                    | W           |
| 0x02                              | 读取雷达运行时间、工作模式、配置文件数量、及接收通道数、TX发波次序<br/>字节数 = 6Bytes + 配置ID数量 * 6Bytes | **当前时间戳**，单位秒，无符号32位整数<br/>DATA[0]：Byte0(最低位)<br/>DATA[1]：Byte1<br/>DATA[2]：Byte2<br/>DATA[3]：Byte3(最高位)<br/>**工作模式**：<br/>DATA[4]<br/>**配置ID数量：**<br/>DATA[5]<br/>**配置ID：**<br/>DATA[6]<br/>**该配置ID的通道数量：**<br/>DATA[7]<br/>**TX发波次序：**<br/>DATA[8]：TX0的发波次序，0代表未启用<br/>DATA[9]：TX1的发波次序<br/>DATA[10]：TX2的发波次序<br/>DATA[11]：TX3的发波次序<br/>==剩余配置ID及通道数、TX发波次序数据...== | 无                    | R           |
| 0x03                              | 读写软硬件版本：硬件版本、软件版本、校准版本、USD硬件版本、USD软件版本、USD Boot版本 | **硬件版本号：**<br/>DATA[0]：Byte0<br/>DATA[1]：Byte1<br/>DATA[2]：Byte2<br/>DATA[3]：Byte3<br/>**软件版本号：**<br/>DATA[4]：Byte0<br/>DATA[5]：Byte1<br/>DATA[6]：Byte2<br/>DATA[7]：Byte3<br/>**校准版本号：**<br/>DATA[8]：Byte0<br/>DATA[9]：Byte1<br/>**USD硬件版本号：**<br/>DATA[10]：Byte0<br/>DATA[11]：Byte1<br/>DATA[12]：Byte2<br/>DATA[13]：Byte3<br/>DATA[14]：Byte4<br/>**USD软件版本号：**<br/>DATA[15]：Byte0<br/>DATA[16]：Byte1<br/>DATA[17]：Byte2<br/>DATA[18]：Byte3<br/>DATA[19]：Byte4<br/>DATA[20]：Byte5<br/>**USD Boot版本号：**<br/>DATA[21]：Byte0<br/>DATA[22]：Byte1<br/>DATA[23]：Byte2<br/>DATA[24]：Byte3<br/>DATA[25]：Byte4<br/>DATA[26]：Byte5<br/>DATA[27]：Byte6 | 生产普通/生产调试模式 | RW          |
| 0x04                              | VCAN测试<br/>==对VCAN发送，VCAN应答，则测试通过==            | DATA[0]：固定为1                                             | 生产普通/生产调试模式 | W           |
| 0x05                              | SN读写                                                       | **SN号：**<br/>DATA[0 - 28]                                  | 生产普通/生产调试模式 | RW          |
| 0x06                              | 读取MOUNTID，接地返回1，悬空返回0                            | DATA[0]：<br/>代表MOUNTID1的状态<br/>DATA[1]：<br/>代表MOUNTID2的状态<br/> | 生产普通/生产调试模式 | R           |
| 0x07                              | 读写表数据，帧计数为0时传输表信息，其他为数据帧，参考：《传输数据：子表类型》《传输数据：数据帧格式》<br/>表信息传输数据内容分为：公共表信息 + 私有表信息<br/>私有表信息的 **长度** 以及 **校验和** 在公共表信息中描述<br/>私有表信息通常描述特定表的相关信息，格式参考：《传输数据：私有表信息》 | ==表信息：18Bytes + 私有表信息==<br/>**无符号16位帧计数**<br/>DATA[0]：Byte0(低位)帧计数<br/>DATA[1]：Byte1(高位)帧计数<br/>DATA[2]：**类ID固定为：0x66**<br/>DATA[3]：**私有头部校验和**<br/>DATA[4]：**私有头部字节数**<br/>DATA[5]：表*主版本号 v0 - 255*<br/>DATA[6]：表*副版本号 0 - 255*<br/>DATA[7]：表*修订版本号 0 - 255*<br/>DATA[8]：参考《子表类型》<br/>DATA[9]：数据类型<br/>0：代表*加特兰CFX 28位数据格式复数*<br/>1：代表*浮点类型复数*<br/>3：32位浮点类型<br/>2：代表*16位整型复数*<br/>10：代表32位浮点数二进制格式<br/>数据长度无符号32位（字节数）：<br/>DATA[10]：Byte0(最低位)<br/>DATA[11]：Byte1<br/>DATA[12]：Byte2<br/>DATA[13]：Byte3(最高位)<br/>CRC32位数值：小端模式<br/>DATA[14]：Byte0(最低位)<br/>DATA[15]：Byte1<br/>DATA[16]：Byte2<br/>DATA[17]：Byte3(最高位)<br/>私有头部信息内容长度依据（私有头部字节数==最大64Bytes==、内容依据表类型）：<br/>《传输数据：私有表信息》 | 生产普通/生产调试模式 | RW          |
| 0x08                              | 设置需要读取的表                                             | DATA[0]：参考《子表类型》                                    | 生产普通/生产调试模式 | W           |
| 0x09                              | 设置需要特定帧的表数据（用于丢帧重传）                       | DATA[0]：Byte0(低位)帧计数<br/>DATA[1]：Byte1(高位)帧计数<br/> | 生产普通/生产调试模式 | W           |
| 0x0A                              | 设置雷达重启                                                 | DATA[0]：<br/>固定为1，雷达回复上位机后，立即进行重启        | 生产普通/生产调试模式 | W           |
| 0x0B                              | 设置雷达保存数据                                             | DATA[0]：<br/>固定为1，雷达会立即触发保存参数到flash         | 生产普通/生产调试模式 | W           |
| 0x0C                              | 获取目标数据，**获取之前应先设置0x0D读取目标数据条件，不设置雷达默认使用首个校准配置ID上报**，数据长度依据目标数量，每个目标都有：速度、方位角度、距离、MAG、RCS、SNR、俯仰角度信息<br/>字节数 = 3Bytes + 目标数量 * 16Bytes<br/>==当数据未准备好时：==<br/>配置ID为0xFF，目标数量为0xFFFF | DATA[0]：配置ID<br/>目标数量<br/>DATA[1]：低字节<br/>DATA[2]：高字节<br/>==目标1的数据：==<br/>**速度**放大100倍，有符号位，数值范围 -32768 +32767真实数值：-327.68 +327.67：<br/>DATA[3]：低字节<br/>DATA[4]：高字节<br/>**方位角**放大100倍，有符号位，数值范围 -16384 ~  +16383真实数值：-163.84 ~ +163.83：<br/>DATA[5]：低字节<br/>DATA[6]：高字节<br/>**距离**放大100倍，无符号位，数值范围 0 +131071真实数值：0 ~ +1310.71：<br/>DATA[7]：Byte0(最低位)<br/>DATA[8]：Byte1<br/>DATA[9]：Byte2<br/>DATA[10]：Byte3(最高位)<br/>**MAG**放大10倍，有符号位，数值范围 -2048 ~  +2046真实数值：-204.8 ~ +204.6：<br/>DATA[11]：低字节<br/>DATA[12]：高字节<br/>**RCS**放大10倍int16 -32768 ~ 32767dB，真实数值：-3276.8 ~ 3276.7dB：<br/>DATA[13]：低字节<br/>DATA[14]：高字节<br/>**SNR**放大10倍，有符号位，数值范围 -2048 ~  +2046真实数值：-204.8 ~ +204.6：<br/>DATA[15]：低字节<br/>DATA[16]：高字节<br/>**俯仰角**放大100倍，有符号位，数值范围 -16384 ~  +16383真实数值：-163.84 ~ +163.83：<br/>DATA[17]：低字节<br/>DATA[18]：高字节<br/>==剩余目标数据...== | 生产普通/生产调试模式 | R           |
| 0x0D                              | 读写手动切换的Profile ID，仅用于获取特定配置下的目标数据     | DATA[0]：配置ID                                              | 生产普通/生产调试模式 | RW          |
| 0x0E                              | 设置读取2DFFT数据条件（转台电机信息，设置时转台应已经在此位置） | DATA[0]：方向：<br/>0：水平<br/>1：俯仰<br/>角度：-128 ~ 127deg<br/>DATA[1]<br/>距离：0 ~ 255m<br/>DATA[2]<br/>速度：-128 ~ 127m/s<br/>DATA[3] | 生产普通/生产调试模式 | W           |
| 0x0F                              | 获取目标2DFFT数据<br/>字节数 = 配置数 * 6Bytes + 通道数 * 8Bytes<br/>TX发波次序：TX0 = 0x00代表未使用<br/>TX0 = 0x01代表2b00000001第1个时序发射<br/>TX0 = 0x02代表2b00000010第2个时序发射<br/>==当数据未准备好时：==<br/>配置ID（值为0xFF）与通道数（值为0xFF），TX发波次序全0，无2DFFT数据 | DATA[0]：配置ID<br/>DATA[1]：通道数（1 - 32~max~）<br/>TX发波次序：<br/>DATA[2]：TX0的发波次序，0代表未启用<br/>DATA[3]：TX1的发波次序<br/>DATA[4]：TX2的发波次序<br/>DATA[5]：TX3的发波次序<br/>通道0的2DFFT数据，数值放大1024倍后int32，实部虚部顺序排列：<br/>实部：<br/>DATA[6]：Byte0(最低位)<br/>DATA[7]：Byte1<br/>DATA[8]：Byte2<br/>DATA[9]：Byte3(最高位)<br/>虚部：<br/>DATA[10]：Byte0(最低位)<br/>DATA[11]：Byte1<br/>DATA[12]：Byte2<br/>DATA[13]：Byte3(最高位)<br/>==剩余通道2DFFT数据...==<br/>==剩余配置下以及该配置下的2DFFT数据...== | 生产普通/生产调试模式 | R           |
| 0x10                              | 读写RCS标定值<br/>字节数 = 1Byte + 配置数 * 3Bytes           | DATA[0]：配置数<br/>DATA[1]：配置ID<br/>**RCS补偿值**，有符号位，数值放大10倍后int16 -32768 ~ 32767dB，真实数值：-3276.8 ~ 3276.7dB<br/>DATA[2]：低字节<br/>DATA[3]：高字节 <br/>==其他配置ID + RCS补偿值...== | 生产普通/生产调试模式 | RW          |
| 0x11                              | 重置所有参数                                                 | DATA[0]：固定为1<br/>                                        | 生产普通/生产调试模式 | W           |
| 0x12                              | 读写校准模式<br/>字节数 = 1Byte + 配置数 * 2Bytes            | DATA[0]：配置数<br/>DATA[1]：配置ID<br/>DATA[2]：校准模式：<br/>0：一度一校准<br/>1：曲线拟合<br/>==其他配置ID + 校准模式...== | 生产普通/生产调试模式 | RW          |
| 0x13                              | 设置调试运行参数，参数在断电重启后失效<br/>TX发波次序：TX0 = 0x00代表未使用<br/>TX0 = 0x01代表2b00000001第1个时序发射<br/>TX0 = 0x02代表2b00000010第2个时序发射<br/> | DATA[0]：配置ID<br/>DATA[1]：配置属性：<br/>0xFF：不使用此配置<br/>0x00：正常模式运行使用此配置<br/>0x01：校准模式运行使用此配置<br/>0x02：正常与校准模式运行使用此配置<br/>起始频率GHz，扩大100倍后数值，如76.3GHz写入7630，数值范围：7400 - 8100，真实数值：74GHz ~ 81GHz：<br/>DATA[2]：低字节<br/>DATA[3]：高字节<br/>带宽设置MHz，数值放大100倍，数值范围：0 - 500000，真实数值范围：0 - 5000MHz：<br/>DATA[4]：Byte0(最低位)<br/>DATA[5]：Byte1<br/>DATA[6]：Byte2<br/>DATA[7]：Byte3(最高位)<br/>chirp上升时间，us，数值范围：> 0<br/>DATA[8]：chirp上升时间<br/>chirp下降时间，us，数值范围：> 0<br/>DATA[9]：chirp下降时间<br/>chirp周期，us，数值范围：>= chirp上升时间 + chirp下降时间<br/>DATA[10]：chirp周期<br/>chirp个数，数值范围 > 0，并且< 4096<br/>DATA[11]：低字节<br/>DATA[12]：高字节<br/>TX发波次序：<br/>DATA[13]：TX0的发波次序，0代表未启用<br/>DATA[14]：TX1的发波次序<br/>DATA[15]：TX2的发波次序<br/>DATA[16]：TX3的发波次序<br/><br/>TX0增益控制，数值范围：±127dB：<br/>DATA[17]<br/>TX1增益控制，数值范围：±127dB：<br/>DATA[18]<br/>TX2增益控制，数值范围：±127dB：<br/>DATA[19]<br/>TX3增益控制，数值范围：±127dB：<br/>DATA[20]<br/> | 生产调试              | W           |
| 0x14                              | 获取所有通道的底噪数据（**关闭RTS**）<br/>字节数 = 配置数 * 2 + 通道数 * 2<br/>==当数据未准备好时，配置ID与通道数返回0xFF== | DATA[0]：配置ID<br/>DATA[0]：通道数<br/>TX发波次序：<br/>DATA[2]：TX0的发波次序，0代表未启用<br/>DATA[3]：TX1的发波次序<br/>DATA[4]：TX2的发波次序<br/>DATA[5]：TX3的发波次序<br/>通道0的底噪数据**dB**，有符号位，放大10倍数值范围 -2048 ~  +2046真实数值：-204.8 ~ +204.6：<br/>DATA[1]：低字节<br/>DATA[2]：高字节<br/>==剩余通道数据...==<br/> | 生产普通/生产调试模式 | R           |
| 0x15                              | 设置立即更新通道底噪数据（**关闭RTS**）<br/>雷达在收到请求后认为RTS已关闭，立即更新底噪数据 | DATA[0]：固定为1<br/>                                        | 生产普通/生产调试模式 | W           |
| 0x16                              | 电压读取<br/>                                                | 功能码（RW读写位）为写入请求时：<br/>DATA[0]：设定需要获取的电压类型**<未定义>**<br/>功能码（RW读写位）为读取请求时：<br/>电压值，数值放大10倍，无符号16位，数值范围：0 - 1023V，真实数值：0 - 102.3V<br/>DATA[0]：低字节<br/>DATA[1]：高字节<br/> | 生产普通/生产调试模式 | RW          |
| 0x17                              | 芯片SN读取<br/>字节数 = 1Byte + SN字节数                     | DATA[0]：SN字节数<br/>DATA[1]：芯片SN信息Byte0<br/>==剩余SN信息...== | 生产普通/生产调试模式 | R           |
| 0x18                              | PMIC SN读取<br/>字节数 = 1Byte + SN字节数                    | DATA[0]：SN字节数<br/>DATA[1]：PMIC SN信息Byte0<br/>==剩余SN信息...== | 生产普通/生产调试模式 | R           |
| 0x19                              | 客户UDS软硬件版本号（DID），软件编码（DID）信息读取<br/>客户版本信息可能存储的是字符或者数值形式，格式不一<br/>字节数 = 1Byte + 版本信息长度（最大64Bytes） | DATA[0]：版本信息长度，范围：0 - 64Bytes<br/>==版本信息...== | 生产普通/生产调试模式 | R           |
| 0x1A                              | RDM数据（区域能量指标）<br/>R：距离，m<br/>D：速度，m/s<br/>==获取数据时，应先设定条件（获取使能、配置ID、距离、速度、通道数）再进行获取，数据未准备好只返回1个字节0xFF，数据准备好后，雷达停止基带，等待数据被取出==<br/>写入字节数 = 10Bytes<br/>首次读取RDM数据：<br/>**帧计数为0时**，返回《RDM数据信息》<br/>数据排列方式：[ 距离bin维度 ] [ 速度bin维度 ]<br/>假设距离bin 512点，速度bin 64点，总RDM数据个数512 * 64：<br/>**帧计数大于0并且小于0xFFFF**，返回一帧**最大64个**《RDM数据》，数据含义：<br/>0 - 63：距离bin为0，64个速度bin下的RDM数据<br/>64 - 127：距离bin为1，64个速度bin下的RDM数据<br/>持续读取，当帧计数为0xFFFF时，代表RDM数据获取结束，雷达启动基带信号处理自动进行下一个RDM数据更新 | 功能码（RW读写位）为写入请求时：<br/>设定RDM数据获取，0关闭，1使能：<br/>DATA[0]<br/>**为1使能时，以下参数有效**<br/>设定需要的数据，配置ID：<br/>DATA[1]<br/>设定需要的数据，距离起始，单位m，无符号16位，数值范围：>= 0：<br/>DATA[2]：低字节<br/>DATA[3]：高字节<br/>设定需要的数据，距离结束，单位m，无符号16位，放大10倍，数值范围：<= 1228：<br/>DATA[4]：低字节<br/>DATA[5]：高字节<br/>设定需要的数据，速度起始，单位m/s，无符号8位，放大10倍，数值范围：>= 0：<br/>DATA[6]<br/>设定需要的数据，速度结束，单位m/s，无符号8位，放大10倍，数值范围：<= 91：<br/>DATA[7]<br/>设定需要的数据，通道起始，无符号8位，数值范围：>= 0：<br/>DATA[8]<br/>设定需要的数据，通道结束，无符号8位，数值范围：<= 16：<br/>DATA[9]<br/>功能码（RW读写位）为读取请求时：<br/>**无符号16位帧计数**<br/>DATA[0]：Byte0(低位)帧计数<br/>DATA[1]：Byte1(高位)帧计数<br/>==帧计数为0《RDM数据信息》==<br/>DATA[2]：配置ID<br/>距离bin起始，无符号16位<br/>DATA[3]：低字节<br/>DATA[4]：高字节<br/>距离bin结束，无符号16位<br/>DATA[5]：低字节<br/>DATA[6]：高字节<br/>距离bin最大值，无符号16位<br/>DATA[7]：低字节<br/>DATA[8]：高字节<br/>速度bin起始，无符号16位<br/>DATA[9]：低字节<br/>DATA[10]：高字节<br/>速度bin结束，无符号16位<br/>DATA[11]：低字节<br/>DATA[12]：高字节<br/>速度bin最大值，无符号16位<br/>DATA[13]：低字节<br/>DATA[14]：高字节<br/>DATA[15]：通道号起始<br/>DATA[16]：通道号结束<br/>TX发波次序：<br/>DATA[17]：TX0的发波次序，0代表未启用<br/>DATA[18]：TX1的发波次序<br/>DATA[19]：TX2的发波次序<br/>DATA[20]：TX3的发波次序<br/>==0 < 帧计数 < 0xFFFF《RDM数据》==<br/>n个（最大64个）平均dB数值，有符号16位，放大10倍：<br/>DATA[2]：低字节<br/>DATA[3]：高字节<br/>==剩余RDM数据...==<br/>==帧计数0xFFFF《结束帧》== | 生产调试模式          | RW          |
| 0x1B                              | 操作看门狗                                                   | DATA[0]：开关喂狗，0关闭喂狗，1允许喂狗<br/>                 | 生产普通/生产调试模式 | W           |
| 0x1C                              | 操作GPIO                                                     | 功能码（RW读写位）为写入请求时：<br/>DATA[0]：端口号<br/>DATA[1]：设置端口方向，0输入，1输出<br/>DATA[2]：设置输出端口数值<br/>功能码（RW读写位）为读取请求时：<br/>DATA[0]：端口号<br/>DATA[1]：端口方向，0输入，1输出<br/>DATA[2]：端口数值，0低电平，1高电平 | 生产普通/生产调试模式 | RW          |
| 0x1D                              | shell命令交互<br/>设备内部shell控制台在接收到命令时，自动发送回复报文，**无需手动读取** | 功能码（RW读写位）为写入请求时：<br/>写入命令：最大支持1 - 256字符<br/>DATA[0] - DATA[255]<br/>功能码（RW读写位）为读取时：<br/>最大单帧1024字符，无需手动读取，监听即可 | 生产调试模式          | RW          |
| 0x1E                              | SPI操作                                                      | 操作指令：0写入测试，1读取测试<br/>DATA[0]：操作指令<br/>    | 生产普通/生产调试模式 | W           |
| 0x1F                              | I2C操作                                                      | 操作指令：0写入测试，1读取测试<br/>DATA[0]：操作指令<br/>    | 生产普通/生产调试模式 | W           |
| 0x20                              | DTC故障诊断                                                  | 功能码（RW读写位）为写入请求时：<br/>DATA[0]：固定1，清除DTC<br/>功能码（RW读写位）为读取请求时：<br/>DTC状态数值<br/>DATA[0]：CAN0_BUSOFF<br/>DATA[1]：CAN1_BUSOFF<br/>DATA[2]：RADAR_UDER_VOLTAGE<br/>DATA[3]：RADAR_OVER_VOLTAGE<br/>DATA[4]：MCU_Over_Temperature<br/>DATA[5]：MISS_ALIGNMENT<br/>DATA[6]：MCU_Over_Temperature<br/>DATA[7]：NOT_CALIBRATED<br/>==剩余状态数值信息...== | 生产普通/生产调试模式 | RW          |

### 传输数据

#### 子表类型

| 表总类                        | 子表类型                                                     | 私有表信息数据内容                                           |
| ----------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 0：*方位导向矢量表*           | 0：配置0方位导向矢量表@ELE+0deg<br/>1：配置1方位导向矢量表@ELE+0deg<br/>2：配置2方位导向矢量表@ELE+0deg<br/>3：配置3方位导向矢量表@ELE+0deg | 起始角度32位浮点数：<br/>DATA[18]：Byte0(最低位)<br/>DATA[19]：Byte1<br/>DATA[20]：Byte2<br/>DATA[21]：Byte3(最高位)<br/>结束角度32位浮点数：<br/>DATA[22]：Byte0(最低位)<br/>DATA[23]：Byte1<br/>DATA[24]：Byte2<br/>DATA[25]：Byte3(最高位)<br/>点数：<br/>DATA[26]：Byte0(低位)<br/>DATA[27]：Byte3(高位)<br/>通道数：<br/>DATA[28]<br/>俯仰角度32位浮点数：<br/>DATA[29]：Byte0(最低位)<br/>DATA[30]：Byte1<br/>DATA[31]：Byte2<br/>DATA[32]：Byte3(最高位)<br/>TX发波次序：<br/>DATA[33]：TX0的发波次序，0代表未启用<br/>DATA[34]：TX1的发波次序<br/>DATA[35]：TX2的发波次序<br/>DATA[36]：TX3的发波次序<br/>DATA[37]：Perofile ID<br/> |
| 1：*俯仰导向矢量表*           | 4：配置0俯仰导向矢量表@AZI+0deg<br/>5：配置1俯仰导向矢量表@AZI+0deg<br/>6：配置2俯仰导向矢量表@AZI+0deg<br/>7：配置3俯仰导向矢量表@AZI+0deg | 起始角度32位浮点数：<br/>DATA[18]：Byte0(最低位)<br/>DATA[19]：Byte1<br/>DATA[20]：Byte2<br/>DATA[21]：Byte3(最高位)<br/>结束角度32位浮点数：<br/>DATA[22]：Byte0(最低位)<br/>DATA[23]：Byte1<br/>DATA[24]：Byte2<br/>DATA[25]：Byte3(最高位)<br/>点数：<br/>DATA[26]：Byte0(低位)<br/>DATA[27]：Byte3(高位)<br/>通道数：<br/>DATA[28]<br/>方位角度32位浮点数：<br/>DATA[29]：Byte0(最低位)<br/>DATA[30]：Byte1<br/>DATA[31]：Byte2<br/>DATA[32]：Byte3(最高位)<br/>TX发波次序：<br/>DATA[33]：TX0的发波次序，0代表未启用<br/>DATA[34]：TX1的发波次序<br/>DATA[35]：TX2的发波次序<br/>DATA[36]：TX3的发波次序<br/>DATA[37]：Perofile ID<br/> |
| 2：*天线间距坐标与初相信息表* | 8：配置0天线间距坐标与初相信息表<br/>9：配置1天线间距坐标与初相信息表<br/>10：配置2天线间距坐标与初相信息表<br/>11：配置3天线间距坐标与初相信息表 | 点数：<br/>DATA[18]：Byte0(低位)<br/>DATA[19]：Byte3(高位)<br/>通道数：<br/>DATA[20]<br/>TX发波次序：<br/>DATA[21]：TX0的发波次序，0代表未启用<br/>DATA[22]：TX1的发波次序<br/>DATA[23]：TX2的发波次序<br/>DATA[24]：TX3的发波次序<br/>DATA[25]：Perofile ID<br/> |
| 3：*方向图表*                 | 12：配置0方向图表<br/>13：配置1方向图表<br/>14：配置2方向图表<br/>15：配置3方向图表 | 起始角度32位浮点数：<br/>DATA[18]：Byte0(最低位)<br/>DATA[19]：Byte1<br/>DATA[20]：Byte2<br/>DATA[21]：Byte3(最高位)<br/>结束角度32位浮点数：<br/>DATA[22]：Byte0(最低位)<br/>DATA[23]：Byte1<br/>DATA[24]：Byte2<br/>DATA[25]：Byte3(最高位)<br/>点数：<br/>DATA[26]：Byte0(低位)<br/>DATA[27]：Byte3(高位)<br/>通道数：<br/>DATA[28]<br/>单位：<br/>DATA[29]：<br/>0：米<br/>1：dB<br/>DATA[30]：TX0的发波次序，0代表未启用<br/>DATA[31]：TX1的发波次序<br/>DATA[32]：TX2的发波次序<br/>DATA[33]：TX3的发波次序<br/>DATA[34]：Perofile ID<br/> |
| 4：*俯仰导向矢量表@AZI-45deg* | 16：配置0俯仰导向矢量表@AZI-45deg<br/>17：配置1俯仰导向矢量表@AZI-45deg<br/>18：配置2俯仰导向矢量表@AZI-45deg<br/>19：配置3俯仰导向矢量表@AZI-45deg | 同1：*俯仰导向矢量表*                                        |
| 5：*俯仰导向矢量表@AZI+45deg* | 20：配置0俯仰导向矢量表@AZI+45deg<br/>21：配置1俯仰导向矢量表@AZI+45deg<br/>22：配置2俯仰导向矢量表@AZI+45deg<br/>23：配置3俯仰导向矢量表@AZI+45deg | 同1：*俯仰导向矢量表*                                        |
| 6：通道底噪表                 | 24：所有配置下通道底噪表                                     | 各配置下通道数量：<br/>DATA[18]：配置0通道数量<br/>DATA[19]：配置1通道数量<br/>DATA[20]：配置2通道数量<br/>DATA[21]：配置3通道数量<br/>单位：<br/>DATA[22]：<br/>0：米<br/>1：dB<br/>配置0的发射次序：<br/>DATA[23]：TX0的发波次序，0代表未启用<br/>DATA[24]：TX1的发波次序<br/>DATA[25]：TX2的发波次序<br/>DATA[26]：TX3的发波次序<br/>配置1的发射次序：<br/>DATA[27]：TX0的发波次序，0代表未启用<br/>DATA[28]：TX1的发波次序<br/>DATA[29]：TX2的发波次序<br/>DATA[30]：TX3的发波次序<br/>配置2的发射次序：<br/>DATA[31]：TX0的发波次序，0代表未启用<br/>DATA[32]：TX1的发波次序<br/>DATA[33]：TX2的发波次序<br/>DATA[34]：TX3的发波次序<br/>配置3的发射次序：<br/>DATA[35]：TX0的发波次序，0代表未启用<br/>DATA[36]：TX1的发波次序<br/>DATA[37]：TX2的发波次序<br/>DATA[38]：TX3的发波次序<br/> |




#### 数据帧格式

数据帧帧计数，从1开始代表传输数据

帧计数为：0xFFFF为结束帧

| 数据类型               | 格式说明                                                     | 不定长数据内容                                               | R/W读写权限 |
| ---------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ----------- |
| 加特兰CFX 28位数据格式 | CFX为加特兰特有表达浮点复数变为32位整型的数据格式，以用于减小内存使用（由8Bytes -> 4Bytes）1 / 2的空间下降，==每帧传输64个点数据即 64 * 4（1个点4个字节） = 256字节== | DATA[0]：Byte0(低位)帧计数<br/>DATA[1]：Byte1(高位)帧计数<br/>第0个数据：<br/>DATA[3]：Byte0(最低位)<br/>DATA[4]：Byte1<br/>DATA[5]：Byte2<br/>DATA[6]：Byte3(最高位)<br/>中间第1 - 62个数据<br/>最后第63个数据：<br/>DATA[254]：Byte0(最低位)<br/>DATA[255]：Byte1<br/>DATA[256]：Byte2<br/>DATA[257]：Byte3(最高位)<br/> | RW          |
| 浮点复数数据格式       | 使用2个32位浮点数（8 Bytes）来表示一个复数的实部与虚部，实部虚部传输，Byte0，代表浮点数在内存中的高字节部分，==每帧传输32个点数据即 32 * 2（实部虚部） * 4（每个实部虚部为32位浮点） = 256字节== | 同上                                                         | RW          |
| 16位整型复数数据格式   | 使用2个16位整型表示一个复数的实部与虚部，实部虚部传输，==每帧传输64个点数据即 64 * 2（实部虚部） * 2（每个实部虚部为16位） = 256字节== | 同上                                                         | RW          |
| 32位浮点数             | 4个字节表示一个浮点数，==每帧传输64个点数据即 64 * 4（每个32位浮点） = 256字节== | 同上                                                         | RW          |





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
# 雷达回复，安全访问码，【75 55】雷达回复固定帧头 【01】bit1 - 7为0代表寄存器，读写位bit0为1代表读 【04 00】代表数据长度4个字节（16位无符号整型，小端模式低字节在前）【68 56 0A 00】为安全码 【7E F5】为crc数值
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
- 《points》点数
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
  - 最低字节为TX0的发波次序，如0xFF FF FF 00，代表该配置下仅仅TX0在使用，并且第一个发波
  - 如0xFF FF 00 01，代表该配置下TX0和1在使用，并且TX1第一个发波，TX0第二个发波
- 《profile_id》配置ID
- 《check sum》和校验，除check sum字段外，所有表信息数值求和的值

## EOL生产流程

```flow
sn_read=>start: 扫码枪录入雷达SN二维码
ready_test=>start: 雷达放置于夹具中，待启动测试
electric_test=>start: 电气测试，电压电流

read_device_access_code=>start: 读取雷达验证码
set_device_run_mode=>start: 设置雷达工作在生产普通模式
pcb_version_test=>start: pcb码读取验证？
clear_flash_test=>start: 清除校准涉及的写入信息？
read_device_mode=>start: 读取工作模式、配置文件数量、及接收通道数
sn_write=>start: SN写入再读取验证？
id0_pin_test=>start: 设置mound0接地，读1，悬空读0
id1_pin_test=>start: 设置mound1接地，读1，悬空读0
sf_version_test=>start: 读取软件版本号验证？
vcan_test=>start: VCAN通讯验证？
ant_calibration=>start: 天线校准，方位@0deg，俯仰@0deg，俯仰@-45deg，俯仰@+45deg，获取相应2DFFT数据
call_dll_calc_dbf_coeff=>start: 调用dll计算dbf因子、方向图等，生成相应csv log
sys_pattern_test=>start: 方向图测试，选取所有通道n个角度点，判断该角度下偏差
call_dll_decode_csv=>start: 调用dll解析校准数据总表csv，获取打包后的传输数据
write_ant_calibration=>start: 写入DBF因子（方位、俯仰）、方向图、天线间距相位补偿

reboot_radar=>start: 重启雷达应用校准参数

read_device_access_code2=>start: 读取雷达验证码
set_device_run_mode2=>start: 设置雷达工作在生产普通模式

set_obj_info_profile_id=>start: 设置需要的目标信息ID（正常运行使用）
read_obj_info=>start: 读取目标的rcs，mag，snr，计算rcs
write_calc_rcs_compensation=>start: 计算rcs补偿值，写入

check_calibration_rsl=>start: 测试校准后、方位、俯仰、rcs、snr、mag

read_background_noise=>start: 读取底噪（关闭rts），导出csv
write_background_noise=>start: 写入底噪数据到雷达

e=>end: 测试ok

sn_read->ready_test->electric_test->read_device_access_code->set_device_run_mode->pcb_version_test->clear_flash_test->read_device_mode->sn_write->id0_pin_test->id1_pin_test->sf_version_test->vcan_test->ant_calibration->call_dll_calc_dbf_coeff->sys_pattern_test->call_dll_decode_csv->write_ant_calibration->reboot_radar->read_device_access_code2->set_device_run_mode2->set_obj_info_profile_id->read_obj_info->write_calc_rcs_compensation->check_calibration_rsl->e

```

