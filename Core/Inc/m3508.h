//
// Created by 刘桂庆 on 2026/11/20.
//


#ifndef __M3508_H
#define __M3508_H

#include "main.h"
#include <stdint.h>

/* M3508反馈数据 */
typedef struct
{
    uint16_t angle;        // 转子机械角度
    int16_t speed_rpm;     // 转速 RPM
    int16_t current;       // 实际电流反馈
    uint8_t temperature;   // 温度
    uint8_t online;        // 在线标志

} M3508_t;

/* 电机1数据 */
extern volatile M3508_t motor1;

/* CAN相关函数 */
void M3508_CAN_Init(void);

void M3508_Send_Current(int16_t current);

void M3508_CAN_Process(void);

#endif

