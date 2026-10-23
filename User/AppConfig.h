#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/* Differential PWM correction used by the two center tracking sensors. */
#define TRACK_TWO_SENSOR_CORRECTION 8

/* 循迹速度参数。 */
#define TRACK_BASE_SPEED       18 // 小车基础运行速度
#define TRACK_SEARCH_SPEED     15 // 小车搜索运行速度
#define TRACK_LOST_STOP_COUNT  3500U // 小车丢失信号停止计数
#define TRACK_PWM_MULTIPLIER   1 // 小车PWM倍数

/* 隧道检测与编码器长度标定参数。 */
#define TUNNEL_ENTER_DISTANCE_CM      30U
#define TUNNEL_EXIT_DISTANCE_CM       35U
#define TUNNEL_CONFIRM_SAMPLES        3U
#define ENCODER_COUNTS_PER_100CM   5093UL // 65mm轮胎 编码器每100厘米脉冲数
// #define ENCODER_COUNTS_PER_100CM   6897UL // 48mm轮胎 编码器每100厘米脉冲数

#endif
