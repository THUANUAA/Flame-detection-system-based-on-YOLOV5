
#ifndef __DHT11_H
#define __DHT11_H

#include "./SYSTEM/sys/sys.h"

/******************************************************************************************/
/* DHT11 引脚 定义 */

#define DHT11_DQ_GPIO_PORT GPIOG
#define DHT11_DQ_GPIO_PIN GPIO_PIN_11
#define DHT11_DQ_GPIO_CLK_ENABLE()    \
    do                                \
    {                                 \
        __HAL_RCC_GPIOG_CLK_ENABLE(); \
    } while (0) /* PG口时钟使能 */

/******************************************************************************************/

/* IO操作函数 */
#define DHT11_DQ_OUT(x)                                                                                                                                        \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        x ? HAL_GPIO_WritePin(DHT11_DQ_GPIO_PORT, DHT11_DQ_GPIO_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(DHT11_DQ_GPIO_PORT, DHT11_DQ_GPIO_PIN, GPIO_PIN_RESET); \
    } while (0) /* 数据端口输出 */
#define DHT11_DQ_IN HAL_GPIO_ReadPin(DHT11_DQ_GPIO_PORT, DHT11_DQ_GPIO_PIN) /* 数据端口输入 */

uint8_t dht11_init(void);                              /* 初始化DHT11 */
uint8_t dht11_check(void);                             /* 检测是否存在DHT11 */
uint8_t dht11_read_data(uint8_t *temp, uint8_t *humi); /* 读取温湿度 */

#endif
