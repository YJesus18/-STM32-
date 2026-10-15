#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/* 循迹速度参数。 */
#define TRACK_BASE_SPEED       26 // 小车基础运行速度
#define TRACK_CROSS_SPEED      25 // 小车交叉运行速度
#define TRACK_SEARCH_SPEED     37 // 小车搜索运行速度
#define TRACK_LOST_STOP_COUNT  5000U // 小车丢失信号停止计数
#define TRACK_PWM_MULTIPLIER   1 // 小车PWM倍数

/* 隧道检测与编码器长度标定参数。 */
#define TUNNEL_DARK_DO_LEVEL       1U
#define TUNNEL_SAMPLE_PERIOD_MS    10U
#define TUNNEL_DEBOUNCE_SAMPLES   10U
#define ENCODER_COUNTS_PER_100CM   5093UL // 编码器每100厘米脉冲数

#endif
