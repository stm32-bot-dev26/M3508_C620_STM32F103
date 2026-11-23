//
// Created by 刘桂庆 on 2026/11/20.
//

#include "m3508.h"
#include "can.h"
/* 电机反馈数据 */
volatile M3508_t motor1 = {0};

/* CAN发送和接收结构体 */
static CAN_TxHeaderTypeDef txHeader;
static CAN_RxHeaderTypeDef rxHeader;

/* CAN数据缓存 */
static uint8_t txData[8];
static uint8_t rxData[8];

/* CAN发送邮箱 */
static uint32_t txMailbox;


/* =================================================
   1. CAN滤波器配置
   接收的数据进入FIFO0
   ================================================= */

static void CAN_Filter_Config(void)
{
    CAN_FilterTypeDef filter = {0};

    filter.FilterBank = 0;

    filter.FilterMode = CAN_FILTERMODE_IDMASK;

    filter.FilterScale = CAN_FILTERSCALE_32BIT;

    /* 接收全部标准帧 */
    filter.FilterIdHigh = 0x0000;
    filter.FilterIdLow = 0x0000;

    filter.FilterMaskIdHigh = 0x0000;
    filter.FilterMaskIdLow = 0x0000;

    /* 关键：指定接收FIFO0 */
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;

    filter.FilterActivation = ENABLE;

    if (HAL_CAN_ConfigFilter(&hcan, &filter) != HAL_OK)
    {
        Error_Handler();
    }
}


/* =================================================
   2. CAN初始化
   ================================================= */

void M3508_CAN_Init(void)
{
    /* 配置CAN滤波器 */
    CAN_Filter_Config();

    /* 启动CAN */
    if (HAL_CAN_Start(&hcan) != HAL_OK)
    {
        Error_Handler();
    }
}


/* =================================================
   3. 发送M3508电流控制指令
   电机ID=1
   控制帧ID=0x200
   ================================================= */

void M3508_Send_Current(int16_t current)
{
    /* 电流指令限幅 */
    if (current > 16384)
        current = 16384;

    if (current < -16384)
        current = -16384;

    txHeader.StdId = 0x200;

    txHeader.ExtId = 0;

    txHeader.IDE = CAN_ID_STD;

    txHeader.RTR = CAN_RTR_DATA;

    txHeader.DLC = 8;

    txHeader.TransmitGlobalTime = DISABLE;

    /* 清空数据 */
    for (uint8_t i = 0; i < 8; i++)
    {
        txData[i] = 0;
    }

    /* 电机ID1对应第一个数据位置 */
    txData[0] = (uint8_t)(current >> 8);

    txData[1] = (uint8_t)(current & 0xFF);

    /* 检查发送邮箱 */
    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) > 0)
    {
        if (HAL_CAN_AddTxMessage(
                &hcan,
                &txHeader,
                txData,
                &txMailbox) != HAL_OK)
        {
            /* 发送失败 */
        }
    }
}


/* =================================================
   4. CAN接收数据
   从FIFO0读取
   ================================================= */

void M3508_CAN_Process(void)
{
    while (HAL_CAN_GetRxFifoFillLevel(
               &hcan,
               CAN_RX_FIFO0) > 0)
    {
        if (HAL_CAN_GetRxMessage(
                &hcan,
                CAN_RX_FIFO0,
                &rxHeader,
                rxData) != HAL_OK)
        {
            return;
        }

        /* 检查标准数据帧和反馈ID */
        if (rxHeader.IDE == CAN_ID_STD &&
            rxHeader.RTR == CAN_RTR_DATA &&
            rxHeader.StdId == 0x201 &&
            rxHeader.DLC == 8)
        {
            motor1.angle =
                ((uint16_t)rxData[0] << 8) |
                rxData[1];

            motor1.speed_rpm =
                (int16_t)(((uint16_t)rxData[2] << 8) |
                rxData[3]);

            motor1.current =
                (int16_t)(((uint16_t)rxData[4] << 8) |
                rxData[5]);

            motor1.temperature = rxData[6];

            motor1.online = 1;
        }
    }
}
