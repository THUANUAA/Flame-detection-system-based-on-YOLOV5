import serial
from PIL import Image, ImageTk
import tkinter as tk


def receive_data(ser):
    """通过串口接收数据，并实时更新"""
    data_buffer = []
    byte_count = 0
    receiving = False

    while True:
        if ser.in_waiting:
            data = ser.readline().decode('utf-8').strip()
            if data == "begin":
                receiving = True
                data_buffer.clear()  # 开始接收时清空缓冲区
                print("开始接收数据...")
                continue
            elif data == "end":
                print(f"接收完毕，总共接收到的字节数: {byte_count}")
                return data_buffer

            if receiving:
                try:
                    pixel = int(data)  # 直接将接收到的数据转换为整数
                    data_buffer.append(pixel)
                    byte_count += 1  # 统计接收到的字节数
                except ValueError as e:
                    print(f"无法转换 '{data}' 为整数: {e}")


def update_image(rgb_data, width, height):
    """更新Tkinter图像"""
    image = Image.new("RGB", (width, height))
    image.putdata(rgb_data)

    # 对图像进行翻转和/或旋转调整方向
    image = image.transpose(Image.FLIP_TOP_BOTTOM)  # 上下翻转
    # 或者你可以使用旋转
    image = image.rotate(270)  # 旋转 270 度

    return ImageTk.PhotoImage(image)


def rgb565_to_rgb888(rgb565):
    """将RGB565转换为RGB888格式"""
    r = (rgb565 >> 11) & 0x1F
    g = (rgb565 >> 5) & 0x3F
    b = (rgb565 & 0x1F)
    # 将颜色值扩展到0-255范围
    r = (r << 3) | (r >> 2)
    g = (g << 2) | (g >> 4)
    b = (b << 3) | (b >> 2)
    return (r, g, b)


def process_data(data_buffer, width, height):
    """处理串口接收到的数据"""
    rgb_data = []
    for pixel in data_buffer:
        rgb_data.append(rgb565_to_rgb888(pixel))

    if len(rgb_data) != width * height:
        raise ValueError("数据长度与指定的图像尺寸不匹配")

    return rgb_data


def refresh_image(window, label, ser, width, height):
    """定时刷新图像"""
    # 通过串口接收数据
    data_buffer = receive_data(ser)

    if data_buffer:  # 如果接收到数据
        # 处理数据
        rgb_data = process_data(data_buffer, width, height)

        # 更新Tkinter的图像
        img = update_image(rgb_data, width, height)
        label.config(image=img)
        label.image = img  # 保持对图像的引用，防止被垃圾回收

    # 设置每隔200毫秒刷新一次
    window.after(200, refresh_image, window, label, ser, width, height)


def main():
    # 定义图像的宽度和高度
    width = 320  # 根据您的图像尺寸设置
    height = 240  # 根据您的图像尺寸设置
    serial_port = "COM7"  # 根据实际情况修改串口号
    baud_rate = 115200     # 根据实际情况修改波特率

    # 创建串口连接
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    print(f"连接到串口: {serial_port}，波特率: {baud_rate}")

    # 创建Tkinter窗口
    window = tk.Tk()
    window.title("实时图像显示")

    # 创建一个标签用于显示图像
    label = tk.Label(window)
    label.pack()

    # 启动实时图像刷新
    window.after(200, refresh_image, window, label, ser, width, height)

    # 开启Tkinter主循环
    window.mainloop()

    ser.close()  # 确保关闭串口


if __name__ == "__main__":
    main()
