
#ifndef __SCCB_H
#define __SCCB_H

#include "./SYSTEM/sys/sys.h"

/******************************************************************************************/
/* 引脚 定义 */

#define SCCB_SCL_GPIO_PORT GPIOD
#define SCCB_SCL_GPIO_PIN GPIO_PIN_3
#define SCCB_SCL_GPIO_CLK_ENABLE()    \
    do                                \
    {                                 \
        __HAL_RCC_GPIOD_CLK_ENABLE(); \
    } while (0) /* PD口时钟使能 */

#define SCCB_SDA_GPIO_PORT GPIOG
#define SCCB_SDA_GPIO_PIN GPIO_PIN_13
#define SCCB_SDA_GPIO_CLK_ENABLE()    \
    do                                \
    {                                 \
        __HAL_RCC_GPIOG_CLK_ENABLE(); \
    } while (0) /* PG口时钟使能 */

#define OV_SCCB_TYPE_NOD 0 /* 如果用不用开漏模式或SCCB上无上拉电阻，我们需要推挽和输入切换的方式 */
#if OV_SCCB_TYPE_NOD
#define SCCB_SDA_IN()             \
    {                             \
        GPIOG->CRH &= 0XFF0FFFFF; \
        GPIOG->CRH |= 0X00800000; \
    }
#define SCCB_SDA_OUT()            \
    {                             \
        GPIOG->CRH &= 0XFF0FFFFF; \
        GPIOG->CRH |= 0X00300000; \
    }
#endif
/******************************************************************************************/

/* IO操作函数 */
#define SCCB_SCL(x)                                                                                                                                            \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        x ? HAL_GPIO_WritePin(SCCB_SCL_GPIO_PORT, SCCB_SCL_GPIO_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(SCCB_SCL_GPIO_PORT, SCCB_SCL_GPIO_PIN, GPIO_PIN_RESET); \
    } while (0) /* SCL */

#define SCCB_SDA(x)                                                                                                                                            \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        x ? HAL_GPIO_WritePin(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN, GPIO_PIN_RESET); \
    } while (0) /* SDA */

#define SCCB_READ_SDA HAL_GPIO_ReadPin(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN) /* 读取SDA */

/* 对外接口函数 */
void sccb_init(void);
void sccb_stop(void);
void sccb_start(void);

void sccb_nack(void);
uint8_t sccb_read_byte(void);
uint8_t sccb_send_byte(uint8_t data);

#endif
