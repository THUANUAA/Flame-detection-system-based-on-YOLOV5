
#include "./BSP/OV7725/sccb.h"
#include "./BSP/OV7725/ov7725.h"
#include "./BSP/OV7725/ov7725cfg.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"


/**
 * @brief       OV7725 读寄存器
 * @param       reg : 寄存器地址
 * @retval      读到的寄存器值
 */
uint8_t ov7725_read_reg(uint16_t reg)
{
    uint8_t data = 0;
    
    sccb_start();                       /* 起始信号 */
    sccb_send_byte(OV7725_ADDR);        /* 写通讯地址 */
    sccb_send_byte(reg);                /* 寄存器地址 */
    sccb_stop();                        /* 停止信号 */
    
    /* 设置寄存器地址后，才是读 */
    sccb_start();                       /* 起始信号 */
    sccb_send_byte(OV7725_ADDR | 0X01); /* 读通讯地址 */
    data = sccb_read_byte();            /* 读取数据 */
    sccb_nack();                        /* 非应答信号 */
    sccb_stop();                        /* 停止信号 */
    
    return data;
}

/**
 * @brief       OV7725 写寄存器
 * @param       reg : 寄存器地址
 * @param       data: 要写入寄存器的值
 * @retval      0, 成功; 1, 失败;
 */
uint8_t ov7725_write_reg(uint8_t reg, uint8_t data)
{
    uint8_t res = 0;
    
    sccb_start();   /* 起始信号 */

    if (sccb_send_byte(OV7725_ADDR))res = 1;    /* 写通讯地址 */

    if (sccb_send_byte(reg))res = 1;            /* 寄存器地址 */

    if (sccb_send_byte(data))res = 1;           /* 写数据 */

    sccb_stop();    /* 停止信号 */
    
    return res;
}

/**
 * @brief       初始化 OV7725
 * @param       无
 * @retval      0, 成功; 1, 失败;
 */
uint8_t ov7725_init(void)
{
    uint16_t i = 0;
    uint16_t reg = 0;
    
    OV7725_VSYNC_GPIO_CLK_ENABLE();     /* VSYNC引脚  GPIO时钟使能 */
    OV7725_WRST_GPIO_CLK_ENABLE();      /* WRST引脚   GPIO时钟使能 */
    OV7725_RRST_GPIO_CLK_ENABLE();      /* RRST引脚   GPIO时钟使能 */
    OV7725_OE_GPIO_CLK_ENABLE();        /* OE引脚     GPIO时钟使能 */
    OV7725_RCLK_GPIO_CLK_ENABLE();      /* RCLK引脚   GPIO时钟使能 */
    OV7725_WEN_GPIO_CLK_ENABLE();       /* WEN引脚    GPIO时钟使能 */
    __HAL_RCC_GPIOC_CLK_ENABLE();       /* C0~C7引脚  GPIO时钟使能 */

    GPIO_InitTypeDef gpio_init_struct;
    gpio_init_struct.Pin = OV7725_VSYNC_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_INPUT;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OV7725_VSYNC_GPIO_PORT, &gpio_init_struct);    /* VSYNC 引脚模式设置 */

    gpio_init_struct.Pin = OV7725_WRST_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OV7725_WRST_GPIO_PORT, &gpio_init_struct);    /* WRST  引脚模式设置 */
    
    gpio_init_struct.Pin = OV7725_RRST_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OV7725_RRST_GPIO_PORT, &gpio_init_struct);    /* RRST  引脚模式设置 */
    
    gpio_init_struct.Pin = OV7725_OE_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OV7725_OE_GPIO_PORT, &gpio_init_struct);      /* OE    引脚模式设置 */
    
    gpio_init_struct.Pin = OV7725_RCLK_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OV7725_RCLK_GPIO_PORT, &gpio_init_struct);    /* RCLK  引脚模式设置 */
    
    gpio_init_struct.Pin = OV7725_WEN_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OV7725_WEN_GPIO_PORT, &gpio_init_struct);     /* WEN   引脚模式设置 */
    
    gpio_init_struct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;                     
    gpio_init_struct.Mode = GPIO_MODE_INPUT;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);                    /* C0~C7 引脚模式设置 */
    
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG(); /* 禁止JTAG, 使能SWD, 释放PB3,PB4两个引脚做普通IO用 */
    
    OV7725_WRST(1);             /* WRST =1 */
    OV7725_RRST(1);             /* RRST =1 */
    OV7725_OE(1);               /* OE =1 */
    OV7725_RCLK(1);             /* RCLK =1 */
    OV7725_WEN(1);              /* WEN =1 */

    sccb_init();                /* 初始化SCCB 的IO口 */

    if (ov7725_write_reg(0x12, 0x80))   /* 复位 */
    {
        return 1;
    }

    delay_ms(50);
    reg = ov7725_read_reg(0X1c);    /* 读取厂家ID 高八位 */
    reg <<= 8;
    reg |= ov7725_read_reg(0X1d);   /* 读取厂家ID 低八位 */

    if ((reg != OV7725_MID) && (reg != OV7725_MID1))    /* MID不正确 ? */
    {
        printf("MID:%d\r\n", reg);
        return 1;
    }

    reg = ov7725_read_reg(0X0a);    /* 读取厂家ID 高八位 */
    reg <<= 8;
    reg |= ov7725_read_reg(0X0b);   /* 读取厂家ID 低八位 */

    if (reg != OV7725_PID)          /* PID不正确 ? */
    {
        printf("HID:%d\r\n", reg);
        return 2;
    }

    /* 初始化 OV7725,采用QVGA分辨率(320*240) */
    for (i = 0; i < sizeof(ov7725_init_reg_tb1) / sizeof(ov7725_init_reg_tb1[0]); i++)
    {
        ov7725_write_reg(ov7725_init_reg_tb1[i][0], ov7725_init_reg_tb1[i][1]);
    }

    return 0;   /* ok */
}

/**
 * @brief       OV7725 灯光模式设置
 * @param       mode : 0~5, 灯光模式
 *   @arg       0: 自动
 *   @arg       1: 日光sunny
 *   @arg       2: 阴天cloudy
 *   @arg       3: 办公室office
 *   @arg       4: 家里home
 *   @arg       5: 夜晚night
 * @retval      无
 */
void ov7725_light_mode(uint8_t mode)
{
    switch (mode)
    {
        case 0: /* auto, 自动模式 */
            ov7725_write_reg(0x13, 0xff); /* AWB on */
            ov7725_write_reg(0x0e, 0x65);
            ov7725_write_reg(0x2d, 0x00);
            ov7725_write_reg(0x2e, 0x00);
            break;

        case 1: /* sunny, 晴天 */
            ov7725_write_reg(0x13, 0xfd); /* AWB off */
            ov7725_write_reg(0x01, 0x5a);
            ov7725_write_reg(0x02, 0x5c);
            ov7725_write_reg(0x0e, 0x65);
            ov7725_write_reg(0x2d, 0x00);
            ov7725_write_reg(0x2e, 0x00);
            break;

        case 2: /* cloudy, 阴天 */
            ov7725_write_reg(0x13, 0xfd); /* AWB off */
            ov7725_write_reg(0x01, 0x58);
            ov7725_write_reg(0x02, 0x60);
            ov7725_write_reg(0x0e, 0x65);
            ov7725_write_reg(0x2d, 0x00);
            ov7725_write_reg(0x2e, 0x00);
            break;

        case 3: /* office, 办公室 */
            ov7725_write_reg(0x13, 0xfd); /* AWB off */
            ov7725_write_reg(0x01, 0x84);
            ov7725_write_reg(0x02, 0x4c);
            ov7725_write_reg(0x0e, 0x65);
            ov7725_write_reg(0x2d, 0x00);
            ov7725_write_reg(0x2e, 0x00);
            break;

        case 4: /* home, 家里 */
            ov7725_write_reg(0x13, 0xfd); /* AWB off */
            ov7725_write_reg(0x01, 0x96);
            ov7725_write_reg(0x02, 0x40);
            ov7725_write_reg(0x0e, 0x65);
            ov7725_write_reg(0x2d, 0x00);
            ov7725_write_reg(0x2e, 0x00);
            break;

        case 5: /* night, 夜晚 */
            ov7725_write_reg(0x13, 0xff); /* AWB on */
            ov7725_write_reg(0x0e, 0xe5);
            break;
    }
}

/**
 * @brief       OV7725 色彩饱和度设置
 * @param       set : 0~8, 代表色彩饱和度 -4 ~ 4.
 * @retval      无
 */
void ov7725_color_saturation(uint8_t sat)
{
    ov7725_write_reg(USAT, sat << 4);
    ov7725_write_reg(VSAT, sat << 4);
}

/**
 * @brief       OV7725 亮度设置
 * @param       bright : 0~8, 代表亮度 -4 ~ 4.
 * @retval      无
 */
void ov7725_brightness(uint8_t bright)
{
    uint8_t bright_value, sign;

    switch (bright)
    {
        case 8:
            bright_value = 0x48;
            sign = 0x06;
            break;

        case 7:
            bright_value = 0x38;
            sign = 0x06;
            break;

        case 6:
            bright_value = 0x28;
            sign = 0x06;
            break;

        case 5:
            bright_value = 0x18;
            sign = 0x06;
            break;

        case 4:
            bright_value = 0x08;
            sign = 0x06;
            break;

        case 3:
            bright_value = 0x08;
            sign = 0x0e;
            break;

        case 2:
            bright_value = 0x18;
            sign = 0x0e;
            break;

        case 1:
            bright_value = 0x28;
            sign = 0x0e;
            break;

        case 0:
            bright_value = 0x38;
            sign = 0x0e;
            break;
    }

    ov7725_write_reg(BRIGHT, bright_value);
    ov7725_write_reg(SIGN, sign);
}

/**
 * @brief       OV7725 对比度设置
 * @param       contrast : 0~8, 代表对比度 -4 ~ 4.
 * @retval      无
 */
void ov7725_contrast(uint8_t contrast)
{
    ov7725_write_reg(CNST, (0x30 - (8 - contrast) * 4));
}

/**
 * @brief       OV7725 特效设置
 * @param       eft : 0~6, 特效效果
 *   @arg       0: 正常
 *   @arg       1: 负片
 *   @arg       2: 黑白
 *   @arg       3: 偏红
 *   @arg       4: 偏绿
 *   @arg       5: 偏蓝
 *   @arg       6: 复古
 * @retval      无
 */
void ov7725_special_effects(uint8_t eft)
{
    switch (eft)
    {
        case 0: /* 正常 */
            ov7725_write_reg(0xa6, 0x06);   /* TSLB设置 */
            ov7725_write_reg(0x60, 0x80);   /* MANV,手动V值 */
            ov7725_write_reg(0x61, 0x80);   /* MANU,手动U值 */
            break;

        case 1: /* 负片 */
            ov7725_write_reg(0xa6, 0x46);
            break;

        case 2: /* 黑白 */
            ov7725_write_reg(0xa6, 0x26);
            ov7725_write_reg(0x60, 0x80);
            ov7725_write_reg(0x61, 0x80);
            break;

        case 3: /* 偏红 */
            ov7725_write_reg(0xa6, 0x1e);
            ov7725_write_reg(0x60, 0x80);
            ov7725_write_reg(0x61, 0xc0);
            break;

        case 4: /* 偏绿 */
            ov7725_write_reg(0xa6, 0x1e);
            ov7725_write_reg(0x60, 0x60);
            ov7725_write_reg(0x61, 0x60);
            break;

        case 5: /* 偏蓝 */
            ov7725_write_reg(0xa6, 0x1e);
            ov7725_write_reg(0x60, 0xa0);
            ov7725_write_reg(0x61, 0x40);
            break;

        case 6: /* 复古 */
            ov7725_write_reg(0xa6, 0x1e);
            ov7725_write_reg(0x60, 0x40);
            ov7725_write_reg(0x61, 0xa0);
            break;

    }
}

/**
 * @brief       设置图像输出窗口
 * @param       width   : 输出图像宽度, <=320
 * @param       height  : 输出图像高度, <=240
 * @param       mode    : 输出模式
 *   @arg       0: QVGA输出模式, 可视范围大, 最大输出分辨率: 320*240
 *   @arg       1: VGA输出模式 , 可视范围小, 最大输出分辨率: 320*240
 * @retval      无
 */
void ov7725_window_set(uint16_t width, uint16_t height, uint8_t mode)
{
    uint8_t raw, temp;
    uint16_t sx, sy;

    if (mode)
    {
        sx = (640 - width) / 2;
        sy = (480 - height) / 2;
        ov7725_write_reg(COM7, 0x06);       /* 设置为VGA模式 */
        ov7725_write_reg(HSTART, 0x23);     /* 水平起始位置 */
        ov7725_write_reg(HSIZE, 0xA0);      /* 水平尺寸 */
        ov7725_write_reg(VSTRT, 0x07);      /* 垂直起始位置 */
        ov7725_write_reg(VSIZE, 0xF0);      /* 垂直尺寸 */
        ov7725_write_reg(HREF, 0x00);
        ov7725_write_reg(HOutSize, 0xA0);   /* 输出尺寸 */
        ov7725_write_reg(VOutSize, 0xF0);   /* 输出尺寸 */
    }
    else
    {
        sx = (320 - width) / 2;
        sy = (240 - height) / 2;
        ov7725_write_reg(COM7, 0x46);       /* 设置为QVGA模式 */
        ov7725_write_reg(HSTART, 0x3f);     /* 水平起始位置 */
        ov7725_write_reg(HSIZE, 0x50);      /* 水平尺寸 */
        ov7725_write_reg(VSTRT, 0x03);      /* 垂直起始位置 */
        ov7725_write_reg(VSIZE, 0x78);      /* 垂直尺寸 */
        ov7725_write_reg(HREF,  0x00);
        ov7725_write_reg(HOutSize, 0x50);   /* 输出尺寸 */
        ov7725_write_reg(VOutSize, 0x78);   /* 输出尺寸 */
    }

    raw = ov7725_read_reg(HSTART);
    temp = raw + (sx >> 2);                 /* sx高8位存在HSTART,低2位存在HREF[5:4] */
    ov7725_write_reg(HSTART, temp);
    ov7725_write_reg(HSIZE, width >> 2);    /* width高8位存在HSIZE,低2位存在HREF[1:0] */

    raw = ov7725_read_reg(VSTRT);
    temp = raw + (sy >> 1);                 /* sy高8位存在VSTRT,低1位存在HREF[6] */
    ov7725_write_reg(VSTRT, temp);
    ov7725_write_reg(VSIZE, height >> 1);   /* height高8位存在VSIZE,低1位存在HREF[2] */

    raw = ov7725_read_reg(HREF);
    temp = ((sy & 0x01) << 6) | ((sx & 0x03) << 4) | ((height & 0x01) << 2) | (width & 0x03) | raw;
    ov7725_write_reg(HREF, temp);

    ov7725_write_reg(HOutSize, width >> 2);
    ov7725_write_reg(VOutSize, height >> 1);

    ov7725_read_reg(EXHCH);
    temp = (raw | (width & 0x03) | ((height & 0x01) << 2));
    ov7725_write_reg(EXHCH, temp);

}









