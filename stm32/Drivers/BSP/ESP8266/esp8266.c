
#include "./BSP/ESP8266/esp8266.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef g_uart_handle; /* ATK-MW8266D UART */
static struct
{
    uint8_t buf[ATK_MW8266D_UART_RX_BUF_SIZE]; /* ֡���ջ��� */
    struct
    {
        uint16_t len : 15;                                  /* ֡���ճ��ȣ�sta[14:0] */
        uint16_t finsh : 1;                                 /* ֡������ɱ�־��sta[15] */
    } sta;                                                  /* ֡״̬��Ϣ */
} g_uart_rx_frame = {0};                                    /* ATK-MW8266D UART����֡������Ϣ�ṹ�� */
static uint8_t g_uart_tx_buf[ATK_MW8266D_UART_TX_BUF_SIZE]; /* ATK-MW8266D UART���ͻ��� */

/**
 * @brief       ATK-MW8266D UART printf
 * @param       fmt: ����ӡ������
 * @retval      ��
 */
void atk_mw8266d_uart_printf(char *fmt, ...)
{
    va_list ap;
    uint16_t len;

    va_start(ap, fmt);
    vsprintf((char *)g_uart_tx_buf, fmt, ap);
    va_end(ap);
    len = strlen((const char *)g_uart_tx_buf);
    HAL_UART_Transmit(&g_uart_handle, g_uart_tx_buf, len, HAL_MAX_DELAY);
}

/**
 * @brief       ATK-MW8266D UART���¿�ʼ��������
 * @param       ��
 * @retval      ��
 */
void atk_mw8266d_uart_rx_restart(void)
{
    g_uart_rx_frame.sta.len = 0;
    g_uart_rx_frame.sta.finsh = 0;
}

/**
 * @brief       ��ȡATK-MW8266D UART���յ���һ֡����
 * @param       ��
 * @retval      NULL: δ���յ�һ֡����
 *              ����: ���յ���һ֡����
 */
uint8_t *atk_mw8266d_uart_rx_get_frame(void)
{
    if (g_uart_rx_frame.sta.finsh == 1)
    {
        g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = '\0';
        return g_uart_rx_frame.buf;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief       ��ȡATK-MW8266D UART���յ���һ֡���ݵĳ���
 * @param       ��
 * @retval      0   : δ���յ�һ֡����
 *              ����: ���յ���һ֡���ݵĳ���
 */
uint16_t atk_mw8266d_uart_rx_get_frame_len(void)
{
    if (g_uart_rx_frame.sta.finsh == 1)
    {
        return g_uart_rx_frame.sta.len;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief       ATK-MW8266D UART��ʼ��
 * @param       baudrate: UARTͨѶ������
 * @retval      ��
 */
void atk_mw8266d_uart_init(uint32_t baudrate)
{
    g_uart_handle.Instance = ATK_MW8266D_UART_INTERFACE;    /* ATK-MW8266D UART */
    g_uart_handle.Init.BaudRate = baudrate;                 /* ������ */
    g_uart_handle.Init.WordLength = UART_WORDLENGTH_8B;     /* ����λ */
    g_uart_handle.Init.StopBits = UART_STOPBITS_1;          /* ֹͣλ */
    g_uart_handle.Init.Parity = UART_PARITY_NONE;           /* У��λ */
    g_uart_handle.Init.Mode = UART_MODE_TX_RX;              /* �շ�ģʽ */
    g_uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;     /* ��Ӳ������ */
    g_uart_handle.Init.OverSampling = UART_OVERSAMPLING_16; /* ������ */
    HAL_UART_Init(&g_uart_handle);                          /* ʹ��ATK-MW8266D UART
                                                             * HAL_UART_Init()����ú���HAL_UART_MspInit()
                                                             * �ú����������ļ�usart.c��
                                                             */
}

/**
 * @brief       ATK-MW8266D UART�жϻص�����
 * @param       ��
 * @retval      ��
 */
void ATK_MW8266D_UART_IRQHandler(void)
{
    uint8_t tmp;

    if (__HAL_UART_GET_FLAG(&g_uart_handle, UART_FLAG_ORE) != RESET) /* UART���չ��ش����ж� */
    {
        __HAL_UART_CLEAR_OREFLAG(&g_uart_handle); /* ������չ��ش����жϱ�־ */
        (void)g_uart_handle.Instance->SR;         /* �ȶ�SR�Ĵ������ٶ�DR�Ĵ��� */
        (void)g_uart_handle.Instance->DR;
    }

    if (__HAL_UART_GET_FLAG(&g_uart_handle, UART_FLAG_RXNE) != RESET) /* UART�����ж� */
    {
        HAL_UART_Receive(&g_uart_handle, &tmp, 1, HAL_MAX_DELAY); /* UART�������� */

        if (g_uart_rx_frame.sta.len < (ATK_MW8266D_UART_RX_BUF_SIZE - 1)) /* �ж�UART���ջ����Ƿ����
                                                                           * ����һλ��������'\0'
                                                                           */
        {
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp; /* �����յ�������д�뻺�� */
            g_uart_rx_frame.sta.len++;                          /* ���½��յ������ݳ��� */
        }
        else /* UART���ջ������ */
        {
            g_uart_rx_frame.sta.len = 0;                        /* ����֮ǰ�յ������� */
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp; /* �����յ�������д�뻺�� */
            g_uart_rx_frame.sta.len++;                          /* ���½��յ������ݳ��� */
        }
    }

    if (__HAL_UART_GET_FLAG(&g_uart_handle, UART_FLAG_IDLE) != RESET) /* UART���߿����ж� */
    {
        g_uart_rx_frame.sta.finsh = 1; /* ���֡������� */

        __HAL_UART_CLEAR_IDLEFLAG(&g_uart_handle); /* ���UART���߿����ж� */
    }
}

/**
 * @brief       ATK-MW8266DӲ����ʼ��
 * @param       ��
 * @retval      ��
 */
static void atk_mw8266d_hw_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    ATK_MW8266D_RST_GPIO_CLK_ENABLE();

    gpio_init_struct.Pin = ATK_MW8266D_RST_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MW8266D_RST_GPIO_PORT, &gpio_init_struct);
}

/**
 * @brief       ATK-MW8266DӲ����λ
 * @param       ��
 * @retval      ��
 */
void atk_mw8266d_hw_reset(void)
{
    ATK_MW8266D_RST(0);
    delay_ms(100);
    ATK_MW8266D_RST(1);
    delay_ms(500);
}

/**
 * @brief       ATK-MW8266D����ATָ��
 * @param       cmd    : �����͵�ATָ��
 *              ack    : �ȴ�����Ӧ
 *              timeout: �ȴ���ʱʱ��
 * @retval      ATK_MW8266D_EOK     : ����ִ�гɹ�
 *              ATK_MW8266D_ETIMEOUT: �ȴ�����Ӧ��ʱ������ִ��ʧ��
 */
uint8_t atk_mw8266d_send_at_cmd(const char *cmd, char *ack, uint32_t timeout)
{
    uint8_t *ret = NULL;

    atk_mw8266d_uart_rx_restart();

    atk_mw8266d_uart_printf("%s\r\n", cmd);

    if ((ack == NULL) || (timeout == 0))
    {
        return ATK_MW8266D_EOK;
    }
    else
    {
        while (timeout > 0)
        {
            ret = atk_mw8266d_uart_rx_get_frame();
            if (ret != NULL)
            {
                if (strstr((const char *)ret, ack) != NULL)
                {
                    return ATK_MW8266D_EOK;
                }
                else
                {
                    atk_mw8266d_uart_rx_restart();
                }
            }
            timeout--;
            delay_ms(1);
        }

        return ATK_MW8266D_ETIMEOUT;
    }
}

/**
 * @brief       ATK-MW8266D��ʼ��
 * @param       baudrate: ATK-MW8266D UARTͨѶ������
 * @retval      ATK_MW8266D_EOK  : ATK-MW8266D��ʼ���ɹ�������ִ�гɹ�
 *              ATK_MW8266D_ERROR: ATK-MW8266D��ʼ��ʧ�ܣ�����ִ��ʧ��
 */
uint8_t atk_mw8266d_init(uint32_t baudrate)
{

    atk_mw8266d_hw_init(); /* ATK-MW8266DӲ����ʼ�� */

    atk_mw8266d_hw_reset(); /* ATK-MW8266DӲ����λ */

    atk_mw8266d_uart_init(baudrate);              /* ATK-MW8266D UART��ʼ�� */
    if (atk_mw8266d_at_test() != ATK_MW8266D_EOK) /* ATK-MW8266D ATָ����� */
    {
        return ATK_MW8266D_ERROR;
    }

    return ATK_MW8266D_EOK;
}

/**
 * @brief       ATK-MW8266D�ָ���������
 * @param       ��
 * @retval      ATK_MW8266D_EOK  : �ָ��������óɹ�
 *              ATK_MW8266D_ERROR: �ָ���������ʧ��
 */
uint8_t atk_mw8266d_restore(void)
{
    uint8_t ret;

    ret = atk_mw8266d_send_at_cmd("AT+RESTORE", "ready", 3000);
    if (ret == ATK_MW8266D_EOK)
    {
        return ATK_MW8266D_EOK;
    }
    else
    {
        return ATK_MW8266D_ERROR;
    }
}

/**
 * @brief       ATK-MW8266D ATָ�����
 * @param       ��
 * @retval      ATK_MW8266D_EOK  : ATָ����Գɹ�
 *              ATK_MW8266D_ERROR: ATָ�����ʧ��
 */
uint8_t atk_mw8266d_at_test(void)
{
    uint8_t ret;
    uint8_t i;

    for (i = 0; i < 10; i++)
    {
        ret = atk_mw8266d_send_at_cmd("AT", "OK", 500);
        if (ret == ATK_MW8266D_EOK)
        {
            return ATK_MW8266D_EOK;
        }
    }
    return ATK_MW8266D_ERROR;
}

/**
 * @brief       ����ATK-MW8266D����ģʽ
 * @param       mode: 1��Stationģʽ
 *                    2��APģʽ
 *                    3��AP+Stationģʽ
 * @retval      ATK_MW8266D_EOK   : ����ģʽ���óɹ�
 *              ATK_MW8266D_ERROR : ����ģʽ����ʧ��
 *              ATK_MW8266D_EINVAL: mode�������󣬹���ģʽ����ʧ��
 */
uint8_t atk_mw8266d_set_mode(uint8_t mode)
{
    uint8_t ret;

    switch (mode)
    {
    case 1:
    {
        ret = atk_mw8266d_send_at_cmd("AT+CWMODE=1", "OK", 500); /* Stationģʽ */
        break;
    }
    case 2:
    {
        ret = atk_mw8266d_send_at_cmd("AT+CWMODE=2", "OK", 500); /* APģʽ */
        break;
    }
    case 3:
    {
        ret = atk_mw8266d_send_at_cmd("AT+CWMODE=3", "OK", 500); /* AP+Stationģʽ */
        break;
    }
    default:
    {
        return ATK_MW8266D_EINVAL;
    }
    }

    if (ret == ATK_MW8266D_EOK)
    {
        return ATK_MW8266D_EOK;
    }
    else
    {
        return ATK_MW8266D_ERROR;
    }
}

/**
 * @brief       ATK-MW8266D�����λ
 * @param       ��
 * @retval      ATK_MW8266D_EOK  : �����λ�ɹ�
 *              ATK_MW8266D_ERROR: �����λʧ��
 */
uint8_t atk_mw8266d_sw_reset(void)
{
    uint8_t ret;

    ret = atk_mw8266d_send_at_cmd("AT+RST", "OK", 500);
    if (ret == ATK_MW8266D_EOK)
    {
        delay_ms(1000);
        return ATK_MW8266D_EOK;
    }
    else
    {
        return ATK_MW8266D_ERROR;
    }
}

/**
 * @brief       ATK-MW8266D���û���ģʽ
 * @param       cfg: 0���رջ���
 *                   1���򿪻���
 * @retval      ATK_MW8266D_EOK  : ���û���ģʽ�ɹ�
 *              ATK_MW8266D_ERROR: ���û���ģʽʧ��
 */
uint8_t atk_mw8266d_ate_config(uint8_t cfg)
{
    uint8_t ret;

    switch (cfg)
    {
    case 0:
    {
        ret = atk_mw8266d_send_at_cmd("ATE0", "OK", 500); /* �رջ��� */
        break;
    }
    case 1:
    {
        ret = atk_mw8266d_send_at_cmd("ATE1", "OK", 500); /* �򿪻��� */
        break;
    }
    default:
    {
        return ATK_MW8266D_EINVAL;
    }
    }

    if (ret == ATK_MW8266D_EOK)
    {
        return ATK_MW8266D_EOK;
    }
    else
    {
        return ATK_MW8266D_ERROR;
    }
}

/**
 * @brief       ATK-MW8266D����WIFI
 * @param       ssid: WIFI����
 *              pwd : WIFI����
 * @retval      ATK_MW8266D_EOK  : WIFI���ӳɹ�
 *              ATK_MW8266D_ERROR: WIFI����ʧ��
 */
uint8_t atk_mw8266d_join_ap(char *ssid, char *pwd)
{
    uint8_t ret;
    char cmd[64];

    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
    ret = atk_mw8266d_send_at_cmd(cmd, "WIFI GOT IP", 10000);
    if (ret == ATK_MW8266D_EOK)
    {
        return ATK_MW8266D_EOK;
    }
    else
    {
        return ATK_MW8266D_ERROR;
    }
}

/**
 * @brief       ATK-MW8266D��ȡIP��ַ
 * @param       buf: IP��ַ����Ҫ16�ֽ��ڴ�ռ�
 * @retval      ATK_MW8266D_EOK  : ��ȡIP��ַ�ɹ�
 *              ATK_MW8266D_ERROR: ��ȡIP��ַʧ��
 */
uint8_t atk_mw8266d_get_ip(char *buf)
{
    uint8_t ret;
    char *p_start;
    char *p_end;

    ret = atk_mw8266d_send_at_cmd("AT+CIFSR", "OK", 500);
    if (ret != ATK_MW8266D_EOK)
    {
        return ATK_MW8266D_ERROR;
    }

    p_start = strstr((const char *)atk_mw8266d_uart_rx_get_frame(), "\"");
    p_end = strstr(p_start + 1, "\"");
    *p_end = '\0';
    sprintf(buf, "%s", p_start + 1);

    return ATK_MW8266D_EOK;
}

/**
 * @brief       ATK-MW8266D����TCP������
 * @param       server_ip  : TCP������IP��ַ
 *              server_port: TCP�������˿ں�
 * @retval      ATK_MW8266D_EOK  : ����TCP�������ɹ�
 *              ATK_MW8266D_ERROR: ����TCP������ʧ��
 */
uint8_t atk_mw8266d_connect_tcp_server(char *server_ip, char *server_port)
{
    uint8_t ret;
    char cmd[64];

    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s", server_ip, server_port);
    ret = atk_mw8266d_send_at_cmd(cmd, "CONNECT", 5000);
    if (ret == ATK_MW8266D_EOK)
    {
        return ATK_MW8266D_EOK;
    }
    else
    {
        return ATK_MW8266D_ERROR;
    }
}

/**
 * @brief       ATK-MW8266D����͸��
 * @param       ��
 * @retval      ATK_MW8266D_EOK  : ����͸���ɹ�
 *              ATK_MW8266D_ERROR: ����͸��ʧ��
 */
uint8_t atk_mw8266d_enter_unvarnished(void)
{
    uint8_t ret;

    ret = atk_mw8266d_send_at_cmd("AT+CIPMODE=1", "OK", 500);
    ret += atk_mw8266d_send_at_cmd("AT+CIPSEND", ">", 500);
    if (ret == ATK_MW8266D_EOK)
    {
        return ATK_MW8266D_EOK;
    }
    else
    {
        return ATK_MW8266D_ERROR;
    }
}

/**
 * @brief       ATK-MW8266D�˳�͸��
 * @param       ��
 * @retval      ��
 */
void atk_mw8266d_exit_unvarnished(void)
{
    atk_mw8266d_uart_printf("+++");
}

/**
 * @brief       ATK-MW8266D����ԭ���Ʒ�����
 * @param       id : ԭ�����豸���
 *              pwd: ԭ�����豸����
 * @retval      ATK_MW8266D_EOK  : ����ԭ���Ʒ������ɹ�
 *              ATK_MW8266D_ERROR: ����ԭ���Ʒ�����ʧ��
 */
uint8_t atk_mw8266d_connect_atkcld(char *id, char *pwd)
{
    uint8_t ret;
    char cmd[64];

    sprintf(cmd, "AT+ATKCLDSTA=\"%s\",\"%s\"", id, pwd);
    ret = atk_mw8266d_send_at_cmd(cmd, "CLOUD CONNECTED", 10000);
    if (ret == ATK_MW8266D_EOK)
    {
        return ATK_MW8266D_EOK;
    }
    else
    {
        return ATK_MW8266D_ERROR;
    }
}

/**
 * @brief       ATK-MW8266D�Ͽ�ԭ���Ʒ���������
 * @param       ��
 * @retval      ATK_MW8266D_EOK  : �Ͽ�ԭ���Ʒ��������ӳɹ�
 *              ATK_MW8266D_ERROR: �Ͽ�ԭ���Ʒ���������ʧ��
 */
uint8_t atk_mw8266d_disconnect_atkcld(void)
{
    uint8_t ret;

    ret = atk_mw8266d_send_at_cmd("AT+ATKCLDCLS", "CLOUD DISCONNECT", 500);
    if (ret == ATK_MW8266D_EOK)
    {
        return ATK_MW8266D_EOK;
    }
    else
    {
        return ATK_MW8266D_ERROR;
    }
}

uint8_t send_command_with_retry(const char *cmd, const char *success_msg, const char *error_msg)
{
    uint8_t ret;
    do
    {
        ret = atk_mw8266d_send_at_cmd(cmd, "OK", 500);
        if (ret == ATK_MW8266D_EOK)
        {
            printf("%s Success!\r\n", success_msg);
            return 1;
        }
        else
        {
            printf("%s Error! Retrying...\r\n", error_msg);
        }
    } while (ret == 0);
    return 0;
}

void esp8266_init(void) // esp8266��ʼ��
{
    if (atk_mw8266d_init(115200) != 0)
    {
        printf("ATK-MW8266D init failed!\r\n");
        while (1)
        {
            LED0_TOGGLE();
            delay_ms(200);
        }
    }
    printf("111\r\n");
    // ִ��ÿ��������������߼�
    send_command_with_retry("AT+RST", "RST", "RST");
    delay_ms(3000);
    printf("222\r\n");
    send_command_with_retry("AT+CWMODE=1", "CWMODE", "CWMODE");
    delay_ms(3000);

    send_command_with_retry("AT+CWJAP=\"Eight\",\"chaoxihuanxiaowang\"", "CWJAP", "CWJAP");
    delay_ms(3000);

    send_command_with_retry("AT+CIFSR", "CIFSR", "CIFSR");
    delay_ms(3000);

    send_command_with_retry("AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"", "CIPSNTPCFG", "CIPSNTPCFG");
    delay_ms(3000);

    send_command_with_retry("AT+MQTTUSERCFG=0,1,\"NULL\",\"study&k1svvIgaMEL\",\"8de87fa346bd9b028b647c2f8917c38471ef387a510585b24636e4009cfc985c\",0,0,\"\"", "MQTTUSERCFG", "MQTTUSERCFG");
    delay_ms(3000);

    send_command_with_retry("AT+MQTTCLIENTID=0,\"k1svvIgaMEL.study|securemode=2\\,signmethod=hmacsha256\\,timestamp=1727402704083|\"", "MQTTCLIENTID", "MQTTCLIENTID");
    delay_ms(3000);

    send_command_with_retry("AT+MQTTCONN=0,\"iot-06z00bm65nni323.mqtt.iothub.aliyuncs.com\",1883,1", "MQTTCONN", "MQTTCONN");
    delay_ms(3000);

    send_command_with_retry("AT+MQTTSUB=0,\"/k1svvIgaMEL/study/user/get\",1", "MQTTSUB", "MQTTSUB");
    delay_ms(3000);
}

uint8_t atk_mw8266d_UPDATE(const char *type, uint8_t val)
{
    char cmd[128];
    sprintf(cmd, "AT+MQTTPUB=0,\"/sys/k1svvIgaMEL/study/thing/event/property/post\",\"{params:{\\\"%s\\\":%d}}\",0,0", type, val);
    return send_command_with_retry(cmd, "UPDATE", "UPDATE");
}

void atk_mw8266d_UPDATE_VAL(uint8_t humidity, uint8_t temperature)
{
    atk_mw8266d_UPDATE("temperature", temperature);
    delay_ms(3000);
    atk_mw8266d_UPDATE("Humidity", humidity);
}

uint8_t atk_mw8266d_GET_VAL(void)
{
    uint8_t *val = atk_mw8266d_uart_rx_get_frame();
    if (strstr((const char *)val, "on") != NULL)
    {
        printf("LED ON!\r\n");
        return 1;
    }
    else
    {
        printf("LED OFF!\r\n");
        return 0;
    }
}
