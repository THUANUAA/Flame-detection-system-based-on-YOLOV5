import serial


def receive_serial_data(port, baudrate, filename):
    # 打开串口
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"串口 {port} 打开成功.")
    except serial.SerialException as e:
        print(f"打开串口失败: {e}")
        return

    recording = False  # 标志位，表示是否正在记录数据
    total_bytes = 0  # 记录接收到的总字节数

    with open(filename, 'w') as f:  # 打开文件用于写入十进制字符串
        print("准备接收数据...")
        buffer = bytearray()  # 用于存储接收到的数据

        while True:
            if ser.in_waiting > 0:  # 检查是否有数据可读
                data = ser.read(ser.in_waiting)  # 读取所有可用数据
                buffer.extend(data)  # 将读取的数据添加到缓冲区

                recording = True  # 开始记录数据

                if total_bytes > 768000:
                    recording = False  # 停止记录数据
                    print("接收数据结束.")
                    print(f"总共接收到 {total_bytes} 字节数据.")
                    break  # 退出循环

                # 如果正在记录数据，处理接收到的字节
                if recording:
                    while len(buffer) >= 2:  # 确保至少有两个字节
                        # 每次处理两个字节（16位）
                        value = int.from_bytes(buffer[:2], byteorder='big')  # 转换为大端字节序
                        f.write(f"{value}\n")  # 将十进制数写入文件
                        total_bytes += 2  # 更新总字节数
                        print(f"目前总字节数：{total_bytes}")
                        buffer = buffer[2:]  # 移除已处理的两个字节

    ser.close()  # 关闭串口


if __name__ == "__main__":
    port = 'COM7'  # 串口号
    baudrate = 115200  # 波特率
    filename = 'received_data.txt'  # 保存的文件名

    receive_serial_data(port, baudrate, filename)
