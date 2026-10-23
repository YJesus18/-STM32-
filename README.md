# STM32 智能循迹小车

基于 STM32F103C8T6 的课程设计项目，集成两路红外循迹、TB6612 电机驱动、霍尔编码器测距、HC-SR04 隧道检测、OLED 状态显示，以及 MaixBit 视觉识别。

当前循迹使用分段差速 PWM 控制，不使用 PID。MaixBit 负责识别蓝色障碍物和红色停车标志，并通过串口把结果发送给 STM32。

## 主要功能

- 使用八路红外模块的中间两路进行循迹
- TB6612 双路直流电机驱动
- 左右轮霍尔编码器四倍频计数
- HC-SR04 判断进入和离开隧道
- 编码器计算隧道长度和隧道数量
- MaixBit 蓝色障碍物识别、画框和累计计数
- MaixBit 红色停车识别
- 红色停车锁存，复位前不会重新启动
- OLED 显示蓝色数量、运行状态、隧道数量、长度和超声波距离

## OLED 显示

正常运行时显示格式如下：

```text
BLUE:000 WALK
TUN:000 OUT
LEN:0000cm
DIST:---cm
```

- `BLUE`：MaixBit 识别到的蓝色障碍物累计数量
- `WALK/STOP`：小车运行或红色锁存停车状态
- `TUN`：已经完整通过的隧道数量
- `IN/OUT`：当前是否位于隧道内
- `LEN`：最近一次隧道的编码器测量长度
- `DIST`：HC-SR04 当前测得的顶部距离；无有效回波时显示 `---`

## 项目结构

```text
.
├─ Hardware/                 STM32 外设驱动和业务模块
│  ├─ TrackControl.c         循迹控制
│  ├─ robot.c                TB6612 电机控制
│  ├─ Encoder.c              左右轮编码器
│  ├─ Ultrasonic.c           HC-SR04 驱动
│  ├─ Tunnel.c               隧道状态与长度计算
│  ├─ VisionControl.c        MaixBit 串口协议解析
│  ├─ AppDisplay.c           OLED 页面显示
│  └─ ...
├─ User/
│  ├─ main.c                 主程序
│  └─ AppConfig.h            速度、隧道和编码器参数
├─ MaixBit/
│  ├─ color_detection.py     蓝色计数和红色停车识别
│  └─ README.md              MaixBit 详细说明
├─ Library/                  STM32 标准外设库
├─ Start/                    启动文件和 CMSIS
├─ System/                   延时等基础功能
└─ Project.uvprojx           Keil 工程文件
```

MaixBit 的识别阈值、运行方法和通信说明见 [MaixBit/README.md](MaixBit/README.md)。

## 硬件组成

- STM32F103C8T6 最小系统板
- TB6612FNG 双路电机驱动
- 两个带霍尔编码器的 MG310 减速电机
- 65 mm 轮胎
- 八路红外循迹模块
- HC-SR04 超声波模块
- 0.96 英寸 I2C OLED
- MaixBit 视觉模块和摄像头

## 当前引脚分配

### 两路循迹

| 传感器 | STM32 |
|---|---|
| L4 / 中间左侧 | PA4 |
| L5 / 中间右侧 | PA3 |

### TB6612 电机驱动

| TB6612 | STM32 |
|---|---|
| AIN1 | PB0 |
| AIN2 | PA9 |
| PWMA | PB7 / TIM4_CH2 |
| BIN1 | PA11 |
| BIN2 | PA12 |
| PWMB | PB8 / TIM4_CH3 |
| STBY | PB6 |

TB6612 的 `VM` 使用独立电机电源，逻辑电源、STM32、MaixBit 和所有传感器必须共地。

### 霍尔编码器

| 编码器信号 | STM32 |
|---|---|
| 左轮 A 相 | PB13 |
| 左轮 B 相 | PB12 |
| 右轮 A 相 | PB15 |
| 右轮 B 相 | PB14 |

### HC-SR04

| HC-SR04 | STM32 |
|---|---|
| TRIG | PB9 |
| ECHO | PA8 / EXTI8 |
| VCC | 5V |
| GND | GND |

HC-SR04 的 ECHO 为 5V 信号，建议通过 `1kΩ + 2kΩ` 电阻分压后接入 PA8：

```text
HC-SR04 ECHO -- 1kΩ --+-- PA8
                       |
                      2kΩ
                       |
                      GND
```

### OLED

| OLED | STM32 |
|---|---|
| SCL | PB10 / I2C2_SCL |
| SDA | PB11 / I2C2_SDA |
| VCC | 按模块规格接 3.3V 或 5V |
| GND | GND |

### MaixBit 串口

| MaixBit | STM32 |
|---|---|
| IO24 / UART1_TX | PA10 / USART1_RX |
| GND | GND |
| IO25 / UART1_RX | 当前不连接 |

串口使用 `115200 8N1`。PA9 已用于 TB6612 AIN2，因此当前只使用 MaixBit 到 STM32 的单向通信。

## 隧道检测与长度计算

HC-SR04 朝上安装，通过小车与隧道顶棚的距离判断进出状态。当前参数位于 [User/AppConfig.h](User/AppConfig.h)：

```c
#define TUNNEL_ENTER_DISTANCE_CM 30U
#define TUNNEL_EXIT_DISTANCE_CM  35U
#define TUNNEL_CONFIRM_SAMPLES   3U
```

判断逻辑：

```text
距离 ≤ 30cm，连续3次 → 进入隧道，显示 IN
距离 ≥ 35cm或无回波，连续3次 → 离开隧道，计数加一并显示 OUT
30～35cm → 保持当前状态
```

进入隧道时保存左右编码器计数，离开时计算左右轮增量的平均值：

```text
平均计数 = (|左轮增量| + |右轮增量|) / 2
长度(cm) = 平均计数 × 100 / 每100cm计数
```

65 mm 轮胎、车轮一圈约 1040 个四倍频计数时，理论值为：

```c
#define ENCODER_COUNTS_PER_100CM 5093UL
```

实际滚动直径、轮胎压缩和打滑都会产生误差，最终应使用准确的 100 cm 距离进行实车标定。

## 视觉识别与串口协议

数据帧固定为 4 字节：

```text
帧头  命令  数据  帧尾
0xAA  CMD   DATA  0x55
```

| 功能 | 数据帧 |
|---|---|
| 蓝色累计数量 | `AA 01 COUNT 55` |
| 红色停车 | `AA 02 01 55` |

蓝色发送累计数量，STM32 会忽略重复值。收到红色停车帧后，STM32 将电机 PWM 置零并锁存停车，同时把 OLED 的 `WALK` 改为 `STOP`。

## 主要配置参数

循迹和隧道参数集中在 [User/AppConfig.h](User/AppConfig.h)：

```c
#define TRACK_BASE_SPEED       36
#define TRACK_CROSS_SPEED      30
#define TRACK_SEARCH_SPEED     50
#define TRACK_LOST_STOP_COUNT  3000U
#define TRACK_PWM_MULTIPLIER   1

#define TUNNEL_ENTER_DISTANCE_CM 30U
#define TUNNEL_EXIT_DISTANCE_CM  35U
#define TUNNEL_CONFIRM_SAMPLES   3U
#define ENCODER_COUNTS_PER_100CM 5093UL
```

修改参数后需要重新编译并烧录 STM32。

## 编译与烧录

### STM32

1. 使用 Keil μVision 5 打开 `Project.uvprojx`。
2. 选择 `STM32F103C8` 对应目标。
3. 编译工程并确认没有 Error。
4. 使用 ST-Link 将程序烧录到 STM32。

### MaixBit

1. 使用 MaixPy IDE 打开 `MaixBit/color_detection.py`。
2. 连接 MaixBit 并运行脚本。
3. 在 IDE 串口终端确认蓝色计数和红色停车输出。
4. 调试完成后，将脚本保存为开发板启动脚本。

## 调试建议

- 首次测试电机和红色停车时，先将车轮架空。
- HC-SR04 的 ECHO 必须正确降压，并确保所有模块共地。
- 先观察 OLED 的 `DIST`，再根据实际隧道内外距离调整进入和离开阈值。
- 隧道离开阈值必须大于进入阈值，避免距离波动造成反复切换。
- 蓝色或红色识别异常时，应在实际赛道光照下重新调整 LAB 阈值。
- 修改串口协议时，MaixBit 和 STM32 两端必须同步更新。

## 当前控制说明

当前 `TrackControl.c` 使用中间两路状态判断和固定差速修正，不包含 PID。编码器目前用于隧道长度测量，还没有用于左右轮速度闭环。
