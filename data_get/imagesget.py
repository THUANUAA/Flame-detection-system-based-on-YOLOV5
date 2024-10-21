import serial
from PIL import Image
import os


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

    recording = False  # 用于指示是否开始记录数据

    print("准备接收数据...")

    while True:
        if ser.in_waiting > 0:  # 检查是否有数据可读
            data = ser.read(ser.in_waiting)  # 读取所有可用数据

            if not recording:
                # 如果尚未开始记录，检查是否收到 "begin"
                if b"begin" in data:
                    recording = True
                    print("开始记录数据...")

            if recording:
                buffer.extend(data)  # 将读取的数据添加到缓冲区

                if b"end" in data:  # 检查是否收到 "end"
                    print("结束记录数据.")
                    break

                # 处理接收到的字节
                while len(buffer) >= 2 and total_bytes < 153600:  # 确保至少有两个字节，且未超过153600字节
                    value = int.from_bytes(buffer[:2], byteorder='big')  # 转换为大端字节序
                    rgb_data.append(rgb565_to_rgb888(value))  # 转换为RGB888格式
                    total_bytes += 2  # 更新总字节数
                    buffer = buffer[2:]  # 移除已处理的两个字节

    print("接收数据结束.")
    print(f"总共接收到 {total_bytes} 字节数据.")
    ser.close()  # 关闭串口
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
    save_path = 'image.png'
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
    port = 'COM7'  # 串口号
    baudrate = 115200  # 波特率

    # 接收串口数据并转换为RGB888格式
    rgb_data = receive_serial_data(port, baudrate, width, height)

    if rgb_data:  # 确保成功接收数据
        create_image(rgb_data, width, height)


if __name__ == "__main__":
    main()
