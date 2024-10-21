
#include "freertos_demo.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/OV7725/ov7725.h"
#include "./BSP/EXTI/exti.h"
#include "./BSP/TIMER/btim.h"
#include "./BSP/SPI/spi.h"
#include "./BSP/BEEP/beep.h"
/*FreeRTOS*********************************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/******************************************************************************************************/
/*FreeRTOS配置*/

/* START_TASK 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define START_TASK_PRIO 1                   /* 任务优先级 */
#define START_STK_SIZE  128                 /* 任务堆栈大小 */
TaskHandle_t            StartTask_Handler;  /* 任务句柄 */
void start_task(void *pvParameters);        /* 任务函数 */

/* TASK1 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK1_PRIO      2                   /* 任务优先级 */
#define TASK1_STK_SIZE  128                 /* 任务堆栈大小 */
TaskHandle_t            Task1Task_Handler;  /* 任务句柄 */
void task1(void *pvParameters);             /* 任务函数 */

/* TASK2 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK2_PRIO      3                   /* 任务优先级 */
#define TASK2_STK_SIZE  128                 /* 任务堆栈大小 */
TaskHandle_t            Task2Task_Handler;  /* 任务句柄 */
void task2(void *pvParameters);             /* 任务函数 */

QueueHandle_t           xQueue;             /* 定义队列 */
#define QUEUE_LENGTH    1                  /* 增加队列支持的消息个数 */
#define QUEUE_ITEM_SIZE sizeof(uint8_t)    /* 队列中每条消息的大小 */


/******************************************************************************************************/
void send_color_over_usart(uint16_t color);

uint16_t g_ov7725_wwidth = 320;  /* 默认窗口宽度为320 */
uint16_t g_ov7725_wheight = 240; /* 默认窗口高度为240 */
extern uint8_t g_ov7725_vsta;  /* 在exit.c里 面定义 */		
extern UART_HandleTypeDef g_uart1_handle;   /* UART句柄 */	
extern SPI_HandleTypeDef g_spi2_handler;

/**
 * @brief       FreeRTOS例程入口函数
 * @param       无
 * @retval      无
 */
void freertos_demo(void)
{   
    xTaskCreate((TaskFunction_t )start_task,            /* 任务函数 */
                (const char*    )"start_task",          /* 任务名称 */
                (uint16_t       )START_STK_SIZE,        /* 任务堆栈大小 */
                (void*          )NULL,                  /* 传入给任务函数的参数 */
                (UBaseType_t    )START_TASK_PRIO,       /* 任务优先级 */
                (TaskHandle_t*  )&StartTask_Handler);   /* 任务句柄 */
    vTaskStartScheduler();
}

/**
 * @brief       start_task
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           /* 进入临界区 */
		/* 创建队列 */
    xQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    /* 创建任务1 */
    xTaskCreate((TaskFunction_t )task1,
                (const char*    )"task1",
                (uint16_t       )TASK1_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK1_PRIO,
                (TaskHandle_t*  )&Task1Task_Handler);
    /* 创建任务2 */
    xTaskCreate((TaskFunction_t )task2,
                (const char*    )"task2",
                (uint16_t       )TASK2_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK2_PRIO,
                (TaskHandle_t*  )&Task2Task_Handler);
    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
    taskEXIT_CRITICAL();            /* 退出临界区 */
}


void ov7725_camera_refresh(void)
{
    uint32_t i, j;
    uint16_t color;

    if (g_ov7725_vsta) /* 有帧中断更新 */
    {
        lcd_scan_dir(U2D_L2R); /* 从上到下, 从左到右 */
        lcd_set_window((lcddev.width - g_ov7725_wwidth) / 2, (lcddev.height - g_ov7725_wheight) / 2,
                       g_ov7725_wwidth, g_ov7725_wheight); /* 将显示区域设置到屏幕中央 */

        lcd_write_ram_prepare(); /* 开始写入GRAM */
        printf("begin\r\n");

        OV7725_RRST(0); /* 开始复位读指针 */
        OV7725_RCLK(0);
        OV7725_RCLK(1);
        OV7725_RCLK(0);
        OV7725_RRST(1); /* 复位读指针结束 */
        OV7725_RCLK(1);

        for (i = 0; i < g_ov7725_wheight; i++)
        {
            for (j = 0; j < g_ov7725_wwidth; j++)
            {
                OV7725_RCLK(0);
                color = OV7725_DATA; /* 读数据 */
                OV7725_RCLK(1);
                color <<= 8;
                OV7725_RCLK(0);
                color |= OV7725_DATA; /* 读数据 */
                OV7725_RCLK(1);
                LCD->LCD_RAM = color;
								
								send_color_over_usart(color);

            }
        }
        printf("end\r\n");

        g_ov7725_vsta = 0; /* 清零帧中断标记 */
        lcd_scan_dir(DFT_SCAN_DIR); /* 恢复默认扫描方向 */
    }
}

/**
 * @brief       task1
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void task1(void *pvParameters)
{
		uint8_t key;
    uint8_t lightmode = 0, effect = 0;
    uint8_t saturation = 4, brightness = 4, contrast = 4;
		uint8_t txdata=0xAA;
		uint8_t rxdata;
	
		lcd_show_string(30,  50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30,  70, 200, 16, 16, "OV7725 TEST", RED);
    lcd_show_string(30,  90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 210, 200, 16, 16, "OV7725 Init...", RED);
		while (1) /* 初始化OV7725 */
    {
        if (ov7725_init() == 0)
        {
            lcd_show_string(30, 210, 200, 16, 16, "OV7725 Init OK       ", RED);

            while (1)
            {
                key = key_scan(0);

                if (key == KEY0_PRES)
                {
                    g_ov7725_wwidth = 320;                                  /* 默认窗口宽度为320 */
                    g_ov7725_wheight = 240;                                 /* 默认窗口高度为240 */
                    ov7725_window_set(g_ov7725_wwidth, g_ov7725_wheight, 0);/* QVGA模式输出 */
                    break;
                }
                else if (key == KEY1_PRES)
                {
                    g_ov7725_wwidth = 320;                                  /* 默认窗口宽度为320 */
                    g_ov7725_wheight = 240;                                 /* 默认窗口高度为240 */
                    ov7725_window_set(g_ov7725_wwidth, g_ov7725_wheight, 1);/* VGA模式输出 */
                    break;
                }


                
                lcd_show_string(30, 230, 210, 16, 16, "KEY0:QVGA  KEY1:VGA", RED); /* 闪烁显示提示信息 */

            }

            ov7725_light_mode(lightmode);
            ov7725_color_saturation(saturation);
            ov7725_brightness(brightness);
            ov7725_contrast(contrast);
            ov7725_special_effects(effect);

            OV7725_OE(0); /* 使能OV7725 FIFO数据输出 */

            break;
        }
        else
        {
            lcd_show_string(30, 210, 200, 16, 16, "OV7725 Error!!", RED);
        }
    }

    btim_timx_int_init(10000, 7200 - 1); /* 10Khz计数频率,1秒钟中断 */
    exti_ov7725_vsync_init();            /* 使能OV7725 VSYNC外部中断, 捕获帧中断 */
    lcd_clear(BLACK);

    while (1)
    {
				HAL_SPI_TransmitReceive(&g_spi2_handler, &txdata, &rxdata, 1, 1000);
				xQueueSend(xQueue, &rxdata, portMAX_DELAY);    /* 将值作为消息发送到队列中 */
        ov7725_camera_refresh(); /* 更新显示 */
				
		}
}

/**
 * @brief       task2
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void task2(void *pvParameters)
{
    uint8_t data=0;          
    while(1)
    {
        xQueueReceive(xQueue, &data, portMAX_DELAY);
        if(data==170)
				{
						BEEP(1);
						LED0(0);
				}else{
						BEEP(0);
						LED0(1);
				}
    }
}

void send_color_over_usart(uint16_t color) {
    // 发送高字节
    USART1->DR = (uint8_t)(color >> 8);
    while ((USART1->SR & 0x40) == 0);  // 等待发送完成
    
    // 发送低字节
    USART1->DR = (uint8_t)(color & 0xFF);
    while ((USART1->SR & 0x40) == 0);  // 等待发送完成
}
