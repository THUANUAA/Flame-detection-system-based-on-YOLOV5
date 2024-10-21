#ifndef _ESP8266_H
#define _ESP8266_H

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


/* 引脚定义 */
#define ATK_MW8266D_UART_TX_GPIO_PORT           GPIOB
#define ATK_MW8266D_UART_TX_GPIO_PIN            GPIO_PIN_10
#define ATK_MW8266D_UART_TX_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)     /* PB口时钟使能 */

#define ATK_MW8266D_UART_RX_GPIO_PORT           GPIOB
#define ATK_MW8266D_UART_RX_GPIO_PIN            GPIO_PIN_11
#define ATK_MW8266D_UART_RX_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)     /* PB口时钟使能 */

#define ATK_MW8266D_UART_INTERFACE              USART3
#define ATK_MW8266D_UART_IRQn                   USART3_IRQn
#define ATK_MW8266D_UART_IRQHandler             USART3_IRQHandler
#define ATK_MW8266D_UART_CLK_ENABLE()           do{ __HAL_RCC_USART3_CLK_ENABLE(); }while(0)    /* USART3 时钟使能 */

/* UART收发缓冲大小 */
#define ATK_MW8266D_UART_RX_BUF_SIZE            128
#define ATK_MW8266D_UART_TX_BUF_SIZE            64


/* 引脚定义 */
#define ATK_MW8266D_RST_GPIO_PORT           GPIOA
#define ATK_MW8266D_RST_GPIO_PIN            GPIO_PIN_4
#define ATK_MW8266D_RST_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0) /* PA口时钟使能 */

/* IO操作 */
#define ATK_MW8266D_RST(x)                  do{ x ?                                                                                     \
                                                HAL_GPIO_WritePin(ATK_MW8266D_RST_GPIO_PORT, ATK_MW8266D_RST_GPIO_PIN, GPIO_PIN_SET) :  \
                                                HAL_GPIO_WritePin(ATK_MW8266D_RST_GPIO_PORT, ATK_MW8266D_RST_GPIO_PIN, GPIO_PIN_RESET); \
                                            }while(0)

/* 错误代码 */
#define ATK_MW8266D_EOK         0   /* 没有错误 */
#define ATK_MW8266D_ERROR       1   /* 通用错误 */
#define ATK_MW8266D_ETIMEOUT    2   /* 超时错误 */
#define ATK_MW8266D_EINVAL      3   /* 参数错误 */

/* 操作函数 */
void atk_mw8266d_hw_reset(void);                                            /* ATK-MW8266D硬件复位 */
uint8_t atk_mw8266d_send_at_cmd(const char *cmd, char *ack, uint32_t timeout);    /* ATK-MW8266D发送AT指令 */
uint8_t atk_mw8266d_init(uint32_t baudrate);                                /* ATK-MW8266D初始化 */
uint8_t atk_mw8266d_restore(void);                                          /* ATK-MW8266D恢复出厂设置 */
uint8_t atk_mw8266d_at_test(void);                                          /* ATK-MW8266D AT指令测试 */
uint8_t atk_mw8266d_set_mode(uint8_t mode);                                 /* 设置ATK-MW8266D工作模式 */
uint8_t atk_mw8266d_sw_reset(void);                                         /* ATK-MW8266D软件复位 */
uint8_t atk_mw8266d_ate_config(uint8_t cfg);                                /* ATK-MW8266D设置回显模式 */
uint8_t atk_mw8266d_join_ap(char *ssid, char *pwd);                         /* ATK-MW8266D连接WIFI */
uint8_t atk_mw8266d_get_ip(char *buf);                                      /* ATK-MW8266D获取IP地址 */
uint8_t atk_mw8266d_connect_tcp_server(char *server_ip, char *server_port); /* ATK-MW8266D连接TCP服务器 */
uint8_t atk_mw8266d_enter_unvarnished(void);                                /* ATK-MW8266D进入透传 */
void atk_mw8266d_exit_unvarnished(void);                                    /* ATK-MW8266D退出透传 */
uint8_t atk_mw8266d_connect_atkcld(char *id, char *pwd);                    /* ATK-MW8266D连接原子云服务器 */
uint8_t atk_mw8266d_disconnect_atkcld(void);                                /* ATK-MW8266D断开原子云服务器连接 */

uint8_t atk_mw8266d_UPDATE_Humidity(uint8_t val);
uint8_t atk_mw8266d_UPDATE_temperature(uint8_t val);
void atk_mw8266d_UPDATE_VAL(uint8_t Humidity,uint8_t temperature);
uint8_t atk_mw8266d_GET_VAL(void);
void retry_command(uint8_t (*command_func)(void), const char* command_name, uint32_t delay_ms_time);

uint8_t atk_mw8266d_GET_VAL(void);
void atk_mw8266d_UPDATE_VAL(uint8_t humidity, uint8_t temperature);
uint8_t atk_mw8266d_UPDATE(const char* type, uint8_t val);
void esp8266_init(void);
uint8_t send_command_with_retry(const char* cmd, const char* success_msg, const char* error_msg);

#endif 
