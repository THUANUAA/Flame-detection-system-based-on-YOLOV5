#ifndef _ESP8266_H
#define _ESP8266_H

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


/* ���Ŷ��� */
#define ATK_MW8266D_UART_TX_GPIO_PORT           GPIOB
#define ATK_MW8266D_UART_TX_GPIO_PIN            GPIO_PIN_10
#define ATK_MW8266D_UART_TX_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)     /* PB��ʱ��ʹ�� */

#define ATK_MW8266D_UART_RX_GPIO_PORT           GPIOB
#define ATK_MW8266D_UART_RX_GPIO_PIN            GPIO_PIN_11
#define ATK_MW8266D_UART_RX_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)     /* PB��ʱ��ʹ�� */

#define ATK_MW8266D_UART_INTERFACE              USART3
#define ATK_MW8266D_UART_IRQn                   USART3_IRQn
#define ATK_MW8266D_UART_IRQHandler             USART3_IRQHandler
#define ATK_MW8266D_UART_CLK_ENABLE()           do{ __HAL_RCC_USART3_CLK_ENABLE(); }while(0)    /* USART3 ʱ��ʹ�� */

/* UART�շ������С */
#define ATK_MW8266D_UART_RX_BUF_SIZE            128
#define ATK_MW8266D_UART_TX_BUF_SIZE            64


/* ���Ŷ��� */
#define ATK_MW8266D_RST_GPIO_PORT           GPIOA
#define ATK_MW8266D_RST_GPIO_PIN            GPIO_PIN_4
#define ATK_MW8266D_RST_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0) /* PA��ʱ��ʹ�� */

/* IO���� */
#define ATK_MW8266D_RST(x)                  do{ x ?                                                                                     \
                                                HAL_GPIO_WritePin(ATK_MW8266D_RST_GPIO_PORT, ATK_MW8266D_RST_GPIO_PIN, GPIO_PIN_SET) :  \
                                                HAL_GPIO_WritePin(ATK_MW8266D_RST_GPIO_PORT, ATK_MW8266D_RST_GPIO_PIN, GPIO_PIN_RESET); \
                                            }while(0)

/* ������� */
#define ATK_MW8266D_EOK         0   /* û�д��� */
#define ATK_MW8266D_ERROR       1   /* ͨ�ô��� */
#define ATK_MW8266D_ETIMEOUT    2   /* ��ʱ���� */
#define ATK_MW8266D_EINVAL      3   /* �������� */

/* �������� */
void atk_mw8266d_hw_reset(void);                                            /* ATK-MW8266DӲ����λ */
uint8_t atk_mw8266d_send_at_cmd(const char *cmd, char *ack, uint32_t timeout);    /* ATK-MW8266D����ATָ�� */
uint8_t atk_mw8266d_init(uint32_t baudrate);                                /* ATK-MW8266D��ʼ�� */
uint8_t atk_mw8266d_restore(void);                                          /* ATK-MW8266D�ָ��������� */
uint8_t atk_mw8266d_at_test(void);                                          /* ATK-MW8266D ATָ����� */
uint8_t atk_mw8266d_set_mode(uint8_t mode);                                 /* ����ATK-MW8266D����ģʽ */
uint8_t atk_mw8266d_sw_reset(void);                                         /* ATK-MW8266D�����λ */
uint8_t atk_mw8266d_ate_config(uint8_t cfg);                                /* ATK-MW8266D���û���ģʽ */
uint8_t atk_mw8266d_join_ap(char *ssid, char *pwd);                         /* ATK-MW8266D����WIFI */
uint8_t atk_mw8266d_get_ip(char *buf);                                      /* ATK-MW8266D��ȡIP��ַ */
uint8_t atk_mw8266d_connect_tcp_server(char *server_ip, char *server_port); /* ATK-MW8266D����TCP������ */
uint8_t atk_mw8266d_enter_unvarnished(void);                                /* ATK-MW8266D����͸�� */
void atk_mw8266d_exit_unvarnished(void);                                    /* ATK-MW8266D�˳�͸�� */
uint8_t atk_mw8266d_connect_atkcld(char *id, char *pwd);                    /* ATK-MW8266D����ԭ���Ʒ����� */
uint8_t atk_mw8266d_disconnect_atkcld(void);                                /* ATK-MW8266D�Ͽ�ԭ���Ʒ��������� */

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
