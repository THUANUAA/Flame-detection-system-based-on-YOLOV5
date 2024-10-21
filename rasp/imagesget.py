import serial
from PIL import Image
import os
import time


def receive_serial_data(port, baudrate, width, height):
    # 打开串口
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"串口 {port} 打开成功.")
    except serial.SerialException as e:
        print(f"打开串口失败: {e}")
        return None

    total_bytes = 0  # 记录接收到的总字节数
    rgb_data = []  # 用于存储RGB数据
    buffer = bytearray()  # 用于存储接收到的数据

    start_signal = b"begin\r"
    end_signal = b"end\r\n"
    recording = False  # 标记是否已经开始记录数据

    print("准备接收数据...")

    while True:
        if ser.in_waiting > 0:
            # 一次读取所有可用数据
            data = ser.read(ser.in_waiting)
            buffer.extend(data)  # 将数据存入缓冲区

            if not recording:
                # 检查缓冲区中是否包含开始信号
                start_idx = buffer.find(start_signal)
                if start_idx != -1:
                    recording = True
                    # 开始信号后面一个字节跳过（因为这个字节无效）
                    buffer = buffer[start_idx + len(start_signal) + 1:]
                    print("检测到开始信号，开始接收数据...")

            if recording:
                # 检查缓冲区中是否包含结束信号
                end_idx = buffer.find(end_signal)
                if end_idx != -1:
                    # 截取结束信号前的数据作为有效数据
                    valid_data = buffer[:end_idx]
                    # 处理接收到的字节
                    while len(valid_data) >= 2:
                        value = int.from_bytes(valid_data[:2], byteorder='big')
                        rgb_data.append(rgb565_to_rgb888(value))
                        total_bytes += 2
                        valid_data = valid_data[2:]
                    print("检测到结束信号，数据接收完毕.")
                    break

    ser.close()  # 关闭串口
    print(f"接收数据结束, 总共接收到 {total_bytes} 字节.")
    return rgb_data


def rgb565_to_rgb888(rgb565):
    """将RGB565转换为RGB888格式"""
    r = (rgb565 >> 11) & 0x1F
    g = (rgb565 >> 5) & 0x3F
    b = rgb565 & 0x1F
    # 将颜色值扩展到0-255范围
    r = (r << 3) | (r >> 2)
    g = (g << 2) | (g >> 4)
    b = (b << 3) | (b >> 2)
    return (r, g, b)


def get_next_image_number():
    """获取下一个图像编号"""
    if not os.path.exists("image.txt"):
        return 1  # 如果文件不存在，从1开始

    with open("image.txt", "r") as f:
        lines = f.readlines()
        if len(lines) == 0:
            return 1  # 文件为空，从1开始
        last_line = lines[-1].strip()  # 去除换行符
        # 查找最后一行中图片编号的位置
        start_idx = last_line.rfind("image") + len("image")
        end_idx = last_line.rfind(".png")
        if start_idx != -1 and end_idx != -1:
            last_image_number = int(last_line[start_idx:end_idx])
            return last_image_number + 1  # 返回下一个编号
        else:
            return 1  # 如果格式不对，重新从1开始


def create_image(rgb_data, width, height):
    """创建并保存图像"""
    if len(rgb_data) != width * height:
        raise ValueError("数据长度与指定的图像尺寸不匹配")

    # 创建图像
    image = Image.new("RGB", (width, height))
    image.putdata(rgb_data)

    # 上下翻转并旋转270度
    image = image.transpose(Image.FLIP_TOP_BOTTOM).rotate(270)

    # 构造保存路径
    image_number = get_next_image_number()
    save_path = f'image/image{image_number}.png'
    image.save(save_path)
    print(f"图像已保存为 {save_path}")

    # 将图像路径写入 image.txt
    with open("image.txt", "a") as f:  # 使用 "a" 模式以追加内容到文件
        f.write(save_path + "\n")
    print(f"图像路径 {save_path} 已写入 image.txt")


def main():
    # 定义图像的宽度和高度
    width = 320  # 根据您的图像尺寸设置
    height = 240  # 根据您的图像尺寸设置
    port = '/dev/ttyS0'  # 串口号
    baudrate = 115200  # 波特率

    while True:
        # 接收串口数据并转换为RGB888格式
        rgb_data = receive_serial_data(port, baudrate, width, height)

        if rgb_data:  # 确保成功接收数据
            create_image(rgb_data, width, height)



if __name__ == "__main__":
    main()

