
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/SRAM/sram.h"
#include "./MALLOC/malloc.h"
#include "./BSP/OV7725/ov7725.h"
#include "./BSP/EXTI/exti.h"
#include "./BSP/SPI/spi.h"
#include "./BSP/BEEP/beep.h"
#include "freertos_demo.h"


int main(void)
{ 
    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */
    delay_init(72);                     /* 延时初始化 */
    usart_init(128000);                 /* 串口初始化为115200 */
    led_init();                         /* 初始化LED */
    lcd_init();                         /* 初始化LCD */
    key_init();                         /* 初始化按键 */
    sram_init();                        /* SRAM初始化 */
    my_mem_init(SRAMIN);                /* 初始化内部SRAM内存池 */
    my_mem_init(SRAMEX);                /* 初始化外部SRAM内存池 */
		spi2_init();
		spi2_set_speed(SPI_SPEED_2);        /* SPI2 切换到高速状态 18Mhz */
    beep_init();
		freertos_demo();                    /* 运行FreeRTOS例程 */
}
