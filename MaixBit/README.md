# MaixBit 颜色识别模块

本目录存放智能小车的 MaixBit 视觉程序。当前程序使用摄像头完成蓝色障碍物计数和红色停车识别，并通过 UART 将识别结果发送给 STM32。

## 文件说明

- `color_detection.py`：MaixBit 主程序，包含摄像头配置、蓝色计数、红色停车和串口发送。

程序不依赖 MaixBit LCD，LCD 损坏或未连接时仍可运行。调试信息会输出到 MaixPy IDE 串口终端。

## 功能

### 蓝色障碍物计数

蓝色物体连续满足检测条件后，计数加一并发送累计数量。同一个物体持续停留在画面中只计数一次，物体消失若干帧后才允许识别下一个。

当前参数：

```python
BLUE_THRESHOLD = (5, 85, -30, 40, -80, -5)
DETECT_ROI = (10, 10, 300, 220)
BLUE_PIXELS_THRESHOLD = 120
BLUE_AREA_THRESHOLD = 200
BLUE_MIN_FRAMES = 2
BLUE_RELEASE_FRAMES = 4
```

另外要求蓝色区域的宽度和高度都不小于 8 像素。

### 红色停车

红色检测使用画面中央的独立小 ROI，判断条件比蓝色严格。连续检测到红色后向 STM32 发送停车命令。

当前参数：

```python
RED_THRESHOLD = (15, 75, 35, 80, 20, 70)
RED_STOP_ROI = (130, 90, 60, 60)
RED_PIXELS_THRESHOLD = 500
RED_AREA_THRESHOLD = 800
RED_MIN_FRAMES = 3
RED_RELEASE_FRAMES = 8
```

另外要求红色区域的宽度和高度都不小于 20 像素。红色必须位于专用 ROI 内，ROI 外的红色不会触发停车。

STM32 收到停车命令后会锁存停车，OLED 从 `WALK` 切换为 `STOP`，需要复位或重新上电才能恢复行驶。

## 串口接线

| MaixBit | STM32 | 说明 |
|---|---|---|
| IO24 / UART1_TX | PA10 / USART1_RX | MaixBit 向 STM32 发送识别结果 |
| GND | GND | 两块控制板必须共地 |
| IO25 / UART1_RX | 不连接 | 当前 STM32 不向 MaixBit 回传数据 |

串口参数：

```text
波特率：115200
数据位：8
校验位：无
停止位：1
```

MaixBit 和 STM32 使用 3.3V TTL 串口电平。不要连接成 TX 对 TX。

## 通信协议

每个数据帧固定为 4 字节：

```text
帧头  命令  数据  帧尾
0xAA  CMD   DATA  0x55
```

| 功能 | 完整帧 | 发送策略 |
|---|---|---|
| 蓝色累计数量 | `AA 01 COUNT 55` | 每次计数连续发送 3 遍 |
| 红色停车 | `AA 02 01 55` | 触发时连续发送 5 遍 |

蓝色发送的是绝对累计数量，不是单次加一命令，因此 STM32 可以忽略重复帧。`COUNT` 为一个字节，发送值超过 255 后会回绕。

对应的 STM32 接收和控制代码：

- `../Hardware/Serial.c`
- `../Hardware/VisionControl.c`
- `../Hardware/TrackControl.c`

## 摄像头设置

当前程序针对室内和偏暗环境使用固定曝光、固定增益和固定白平衡：

```python
sensor.set_auto_exposure(False, exposure_us=35000)
sensor.set_auto_gain(False, 18)
sensor.set_auto_whitebal(False)
sensor.set_brightness(2)
sensor.set_contrast(1)
sensor.set_saturation(1)
```

固定参数可以减少 LAB 颜色漂移。如果画面仍然太暗，可以适当提高曝光或增加补光；曝光过高会造成小车运动时画面拖影。

## 运行方法

1. 使用 MaixPy IDE 打开 `color_detection.py`。
2. 连接 MaixBit，确认开发板和摄像头工作正常。
3. 运行脚本并观察 IDE 串口终端。
4. 识别蓝色时会输出 `BLUE count: n`。
5. 识别红色时会输出 `RED stop`。
6. 确认调试正常后，可将脚本保存为开发板启动脚本。

首次测试红色停车时，建议先将车轮架空，避免参数或接线错误导致小车意外运动。

## 参数调试

### 蓝色不灵敏

- 减小 `BLUE_PIXELS_THRESHOLD` 和 `BLUE_AREA_THRESHOLD`。
- 放宽 `BLUE_THRESHOLD`。
- 检查物体是否位于 `DETECT_ROI` 内。
- 增加现场照明，避免只依赖提高曝光。

### 蓝色重复计数

- 增大 `BLUE_MIN_FRAMES`。
- 增大 `BLUE_RELEASE_FRAMES`。
- 缩小蓝色阈值范围，减少背景误识别。

### 红色无法停车

- 确认红色物体进入中央的 `RED_STOP_ROI`。
- 检查 MaixPy IDE 是否输出 `RED stop`。
- 如果没有输出，适当降低红色像素和面积门槛，或重新标定 `RED_THRESHOLD`。
- 如果 IDE 已输出但小车未停车，检查 IO24 到 PA10 的串口线以及两块板的公共地线。

### 红色误触发

- 缩小 `RED_STOP_ROI`。
- 提高 `RED_PIXELS_THRESHOLD` 和 `RED_AREA_THRESHOLD`。
- 增大 `RED_MIN_FRAMES`。
- 收紧 `RED_THRESHOLD`，特别是 LAB 中的 A、B 范围。

## 注意事项

- 调参应以实际赛道光照、摄像头角度和物体材质为准。
- 修改 MaixBit 脚本后，不需要重新编译 STM32；修改通信协议时，两端代码必须同时更新。
- 不要提交编辑器生成的 `.autosave` 文件。
- 仓库中的脚本是唯一维护版本，避免同时修改仓库外的旧副本。
