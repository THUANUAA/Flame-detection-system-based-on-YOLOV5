
#ifndef _OV7725_H
#define _OV7725_H

#include "./SYSTEM/sys/sys.h"

/******************************************************************************************/
/* OV7725模块的的 VSYNC/WRST/RRST/OE/RCLK/WEN 引脚 定义
 * D0~D7, 由于引脚多, 且连续, 这里就不在这里定义了,直接在ov7725_init里面修改.所以在移植
 * 的时候, 除了改这6个IO口, 还得改ov7725_init里面的D0~D7所在的IO口.
 */

#define OV7725_VSYNC_GPIO_PORT GPIOA
#define OV7725_VSYNC_GPIO_PIN GPIO_PIN_8
#define OV7725_VSYNC_GPIO_CLK_ENABLE() \
    do                                 \
    {                                  \
        __HAL_RCC_GPIOA_CLK_ENABLE();  \
    } while (0) /* OV7725帧同步脚, 所在IO口时钟使能 */

#define OV7725_WRST_GPIO_PORT GPIOD
#define OV7725_WRST_GPIO_PIN GPIO_PIN_6
#define OV7725_WRST_GPIO_CLK_ENABLE() \
    do                                \
    {                                 \
        __HAL_RCC_GPIOD_CLK_ENABLE(); \
    } while (0) /* FIFO写指针复位脚, 所在IO口时钟使能 */

#define OV7725_RRST_GPIO_PORT GPIOG
#define OV7725_RRST_GPIO_PIN GPIO_PIN_14
#define OV7725_RRST_GPIO_CLK_ENABLE() \
    do                                \
    {                                 \
        __HAL_RCC_GPIOG_CLK_ENABLE(); \
    } while (0) /* FIFO读指针复位脚, 所在IO口时钟使能 */

#define OV7725_OE_GPIO_PORT GPIOG
#define OV7725_OE_GPIO_PIN GPIO_PIN_15
#define OV7725_OE_GPIO_CLK_ENABLE()   \
    do                                \
    {                                 \
        __HAL_RCC_GPIOG_CLK_ENABLE(); \
    } while (0) /* FIFO数据输出使能脚, 所在IO口时钟使能 */

#define OV7725_RCLK_GPIO_PORT GPIOB
#define OV7725_RCLK_GPIO_PIN GPIO_PIN_4
#define OV7725_RCLK_GPIO_CLK_ENABLE() \
    do                                \
    {                                 \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0) /* FIFO读数据时钟脚, 所在IO口时钟使能 */

#define OV7725_WEN_GPIO_PORT GPIOB
#define OV7725_WEN_GPIO_PIN GPIO_PIN_3
#define OV7725_WEN_GPIO_CLK_ENABLE()  \
    do                                \
    {                                 \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0) /* FIFO写数据使能脚, 所在IO口时钟使能 */

/******************************************************************************************/

/* OV7725相关端口定义 */
#define OV7725_VSYNC(x)                                                                                                                                                        \
    do                                                                                                                                                                         \
    {                                                                                                                                                                          \
        x ? HAL_GPIO_WritePin(OV7725_VSYNC_GPIO_PORT, OV7725_VSYNC_GPIO_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(OV7725_VSYNC_GPIO_PORT, OV7725_VSYNC_GPIO_PIN, GPIO_PIN_RESET); \
    } while (0) /* VSYNC */

#define OV7725_WRST(x)                                                                                                                                                     \
    do                                                                                                                                                                     \
    {                                                                                                                                                                      \
        x ? HAL_GPIO_WritePin(OV7725_WRST_GPIO_PORT, OV7725_WRST_GPIO_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(OV7725_WRST_GPIO_PORT, OV7725_WRST_GPIO_PIN, GPIO_PIN_RESET); \
    } while (0) /* WRST */

#define OV7725_RRST(x)                                                                                                                                                     \
    do                                                                                                                                                                     \
    {                                                                                                                                                                      \
        x ? HAL_GPIO_WritePin(OV7725_RRST_GPIO_PORT, OV7725_RRST_GPIO_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(OV7725_RRST_GPIO_PORT, OV7725_RRST_GPIO_PIN, GPIO_PIN_RESET); \
    } while (0) /* RRST */

#define OV7725_OE(x)                                                                                                                                               \
    do                                                                                                                                                             \
    {                                                                                                                                                              \
        x ? HAL_GPIO_WritePin(OV7725_OE_GPIO_PORT, OV7725_OE_GPIO_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(OV7725_OE_GPIO_PORT, OV7725_OE_GPIO_PIN, GPIO_PIN_RESET); \
    } while (0) /* OE */

#define OV7725_RCLK(x) x ? (OV7725_RCLK_GPIO_PORT->BSRR = OV7725_RCLK_GPIO_PIN) : (OV7725_RCLK_GPIO_PORT->BRR = OV7725_RCLK_GPIO_PIN) /* RCLK */
// #define OV7725_RCLK(x)      HAL_GPIO_TogglePin(OV7725_RCLK_GPIO_PORT,OV7725_RCLK_GPIO_PIN);

#define OV7725_WEN(x)                                                                                                                                                  \
    do                                                                                                                                                                 \
    {                                                                                                                                                                  \
        x ? HAL_GPIO_WritePin(OV7725_WEN_GPIO_PORT, OV7725_WEN_GPIO_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(OV7725_WEN_GPIO_PORT, OV7725_WEN_GPIO_PIN, GPIO_PIN_RESET); \
    } while (0) /* WEN */

#define OV7725_DATA GPIOC->IDR & 0X00FF /* D0~D7 */

/* OV7725 ID */
#define OV7725_MID 0X7FA2  /* MID , 第一个MID*/
#define OV7725_MID1 0X7FFF /* MID1, 第二个MID */
#define OV7725_PID 0X7721

/* OV7670的SCCB地址 */
#define OV7725_ADDR 0X42

uint8_t ov7725_init(void);                           /* OV7725初始化 */
uint8_t ov7725_read_reg(uint16_t reg);               /* OV7725 读寄存器 */
uint8_t ov7725_write_reg(uint8_t reg, uint8_t data); /* OV7725写寄存器 */

void ov7725_light_mode(uint8_t mode);                                  /* OV7725 灯光模式设置 */
void ov7725_color_saturation(uint8_t sat);                             /* OV7725 色彩饱和度设置 */
void ov7725_brightness(uint8_t bright);                                /* OV7725 亮度设置 */
void ov7725_contrast(uint8_t contrast);                                /* OV7725 对比度设置 */
void ov7725_special_effects(uint8_t eft);                              /* OV7725 特效设置 */
void ov7725_window_set(uint16_t width, uint16_t height, uint8_t mode); /* OV7725 输出窗口设置 */
void ov7725_camera_refresh(void);
#endif
