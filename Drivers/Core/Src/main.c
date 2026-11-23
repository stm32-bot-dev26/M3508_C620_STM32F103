
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    main.c
  * @brief   M3508 + C620 motor control
  * @board   STM32F103C8T6
  * @author  M3508 Control Project
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "can.h"
#include "gpio.h"
#include "m3508.h"

#include <stdint.h>

/* Private function prototypes */
void SystemClock_Config(void);

/* 电机测试电流指令，初始为0 */
volatile int16_t test_current = 0;

/* CAN发送计时 */
static uint32_t can_send_time = 0;

/* 主函数 */
int main(void)
{
    /* 1. HAL库初始化 */
    HAL_Init();

    /* 2. 系统时钟初始化 */
    SystemClock_Config();

    /* 3. GPIO初始化 */
    MX_GPIO_Init();

    /* 4. CAN1初始化 */
    MX_CAN_Init();

    /* 5. M3508 CAN通信初始化 */
    M3508_CAN_Init();

    /* 6. 初始发送零电流 */
    test_current = 0;

    can_send_time = HAL_GetTick();

    /* 7. 主循环 */
    while (1)
    {
        /* 接收C620返回的反馈数据 */
        M3508_CAN_Process();

        /* 每10ms发送一次控制指令 */
        if ((HAL_GetTick() - can_send_time) >= 10)
        {
            can_send_time = HAL_GetTick();

            M3508_Send_Current(test_current);
        }

        /* 短暂延时 */
        HAL_Delay(1);
    }
}

/* =================================================
   系统时钟配置
   HSE = 8MHz
   SYSCLK = 72MHz
   PCLK1 = 36MHz
   ================================================= */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSE;

    RCC_OscInitStruct.HSEState =
        RCC_HSE_ON;

    RCC_OscInitStruct.HSEPredivValue =
        RCC_HSE_PREDIV_DIV1;

    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_ON;

    RCC_OscInitStruct.PLL.PLLSource =
        RCC_PLLSOURCE_HSE;

    RCC_OscInitStruct.PLL.PLLMUL =
        RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_PLLCLK;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV2;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* =================================================
   错误处理
   ================================================= */

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
    /* 用户可以在这里添加错误提示 */
}

#endif
