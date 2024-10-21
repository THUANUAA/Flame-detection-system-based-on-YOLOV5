
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
/*FreeRTOS����*/

/* START_TASK ���� ����
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define START_TASK_PRIO 1                   /* �������ȼ� */
#define START_STK_SIZE  128                 /* �����ջ��С */
TaskHandle_t            StartTask_Handler;  /* ������ */
void start_task(void *pvParameters);        /* ������ */

/* TASK1 ���� ����
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define TASK1_PRIO      2                   /* �������ȼ� */
#define TASK1_STK_SIZE  128                 /* �����ջ��С */
TaskHandle_t            Task1Task_Handler;  /* ������ */
void task1(void *pvParameters);             /* ������ */

/* TASK2 ���� ����
 * ����: ������ �������ȼ� ��ջ��С ��������
 */
#define TASK2_PRIO      3                   /* �������ȼ� */
#define TASK2_STK_SIZE  128                 /* �����ջ��С */
TaskHandle_t            Task2Task_Handler;  /* ������ */
void task2(void *pvParameters);             /* ������ */

QueueHandle_t           xQueue;             /* ������� */
#define QUEUE_LENGTH    1                  /* ���Ӷ���֧�ֵ���Ϣ���� */
#define QUEUE_ITEM_SIZE sizeof(uint8_t)    /* ������ÿ����Ϣ�Ĵ�С */


/******************************************************************************************************/
void send_color_over_usart(uint16_t color);

uint16_t g_ov7725_wwidth = 320;  /* Ĭ�ϴ��ڿ��Ϊ320 */
uint16_t g_ov7725_wheight = 240; /* Ĭ�ϴ��ڸ߶�Ϊ240 */
extern uint8_t g_ov7725_vsta;  /* ��exit.c�� �涨�� */		
extern UART_HandleTypeDef g_uart1_handle;   /* UART��� */	
extern SPI_HandleTypeDef g_spi2_handler;

/**
 * @brief       FreeRTOS������ں���
 * @param       ��
 * @retval      ��
 */
void freertos_demo(void)
{   
    xTaskCreate((TaskFunction_t )start_task,            /* ������ */
                (const char*    )"start_task",          /* �������� */
                (uint16_t       )START_STK_SIZE,        /* �����ջ��С */
                (void*          )NULL,                  /* ������������Ĳ��� */
                (UBaseType_t    )START_TASK_PRIO,       /* �������ȼ� */
                (TaskHandle_t*  )&StartTask_Handler);   /* ������ */
    vTaskStartScheduler();
}

/**
 * @brief       start_task
 * @param       pvParameters : �������(δ�õ�)
 * @retval      ��
 */
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           /* �����ٽ��� */
		/* �������� */
    xQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    /* ��������1 */
    xTaskCreate((TaskFunction_t )task1,
                (const char*    )"task1",
                (uint16_t       )TASK1_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK1_PRIO,
                (TaskHandle_t*  )&Task1Task_Handler);
    /* ��������2 */
    xTaskCreate((TaskFunction_t )task2,
                (const char*    )"task2",
                (uint16_t       )TASK2_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK2_PRIO,
                (TaskHandle_t*  )&Task2Task_Handler);
    vTaskDelete(StartTask_Handler); /* ɾ����ʼ���� */
    taskEXIT_CRITICAL();            /* �˳��ٽ��� */
}


void ov7725_camera_refresh(void)
{
    uint32_t i, j;
    uint16_t color;

    if (g_ov7725_vsta) /* ��֡�жϸ��� */
    {
        lcd_scan_dir(U2D_L2R); /* ���ϵ���, ������ */
        lcd_set_window((lcddev.width - g_ov7725_wwidth) / 2, (lcddev.height - g_ov7725_wheight) / 2,
                       g_ov7725_wwidth, g_ov7725_wheight); /* ����ʾ�������õ���Ļ���� */

        lcd_write_ram_prepare(); /* ��ʼд��GRAM */
        printf("begin\r\n");

        OV7725_RRST(0); /* ��ʼ��λ��ָ�� */
        OV7725_RCLK(0);
        OV7725_RCLK(1);
        OV7725_RCLK(0);
        OV7725_RRST(1); /* ��λ��ָ����� */
        OV7725_RCLK(1);

        for (i = 0; i < g_ov7725_wheight; i++)
        {
            for (j = 0; j < g_ov7725_wwidth; j++)
            {
                OV7725_RCLK(0);
                color = OV7725_DATA; /* ������ */
                OV7725_RCLK(1);
                color <<= 8;
                OV7725_RCLK(0);
                color |= OV7725_DATA; /* ������ */
                OV7725_RCLK(1);
                LCD->LCD_RAM = color;
								
								send_color_over_usart(color);

            }
        }
        printf("end\r\n");

        g_ov7725_vsta = 0; /* ����֡�жϱ�� */
        lcd_scan_dir(DFT_SCAN_DIR); /* �ָ�Ĭ��ɨ�跽�� */
    }
}

/**
 * @brief       task1
 * @param       pvParameters : �������(δ�õ�)
 * @retval      ��
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
		while (1) /* ��ʼ��OV7725 */
    {
        if (ov7725_init() == 0)
        {
            lcd_show_string(30, 210, 200, 16, 16, "OV7725 Init OK       ", RED);

            while (1)
            {
                key = key_scan(0);

                if (key == KEY0_PRES)
                {
                    g_ov7725_wwidth = 320;                                  /* Ĭ�ϴ��ڿ��Ϊ320 */
                    g_ov7725_wheight = 240;                                 /* Ĭ�ϴ��ڸ߶�Ϊ240 */
                    ov7725_window_set(g_ov7725_wwidth, g_ov7725_wheight, 0);/* QVGAģʽ��� */
                    break;
                }
                else if (key == KEY1_PRES)
                {
                    g_ov7725_wwidth = 320;                                  /* Ĭ�ϴ��ڿ��Ϊ320 */
                    g_ov7725_wheight = 240;                                 /* Ĭ�ϴ��ڸ߶�Ϊ240 */
                    ov7725_window_set(g_ov7725_wwidth, g_ov7725_wheight, 1);/* VGAģʽ��� */
                    break;
                }


                
                lcd_show_string(30, 230, 210, 16, 16, "KEY0:QVGA  KEY1:VGA", RED); /* ��˸��ʾ��ʾ��Ϣ */

            }

            ov7725_light_mode(lightmode);
            ov7725_color_saturation(saturation);
            ov7725_brightness(brightness);
            ov7725_contrast(contrast);
            ov7725_special_effects(effect);

            OV7725_OE(0); /* ʹ��OV7725 FIFO������� */

            break;
        }
        else
        {
            lcd_show_string(30, 210, 200, 16, 16, "OV7725 Error!!", RED);
        }
    }

    btim_timx_int_init(10000, 7200 - 1); /* 10Khz����Ƶ��,1�����ж� */
    exti_ov7725_vsync_init();            /* ʹ��OV7725 VSYNC�ⲿ�ж�, ����֡�ж� */
    lcd_clear(BLACK);

    while (1)
    {
				HAL_SPI_TransmitReceive(&g_spi2_handler, &txdata, &rxdata, 1, 1000);
				xQueueSend(xQueue, &rxdata, portMAX_DELAY);    /* ��ֵ��Ϊ��Ϣ���͵������� */
        ov7725_camera_refresh(); /* ������ʾ */
				
		}
}

/**
 * @brief       task2
 * @param       pvParameters : �������(δ�õ�)
 * @retval      ��
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
    // ���͸��ֽ�
    USART1->DR = (uint8_t)(color >> 8);
    while ((USART1->SR & 0x40) == 0);  // �ȴ��������
    
    // ���͵��ֽ�
    USART1->DR = (uint8_t)(color & 0xFF);
    while ((USART1->SR & 0x40) == 0);  // �ȴ��������
}
