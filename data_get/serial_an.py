import serial


def receive_and_save_data(serial_port, baud_rate, output_file):
    """通过串口接收数据并直接保存"""
    try:
        # 打开串口
        ser = serial.Serial(serial_port, baud_rate, timeout=1)
        print(f"连接到串口: {serial_port}，波特率: {baud_rate}")

        with open(output_file, "w") as file:
            receiving = False  # 是否正在接收数据
            byte_count = 0  # 记录接收到的字节数

            while True:
                # 读取串口数据
                if ser.in_waiting:
                    data = ser.readline().decode('utf-8').strip()  # 读取一行并去除多余字符
                    if data == "begin":
                        receiving = True
                        print("开始接收数据...")
                        continue
                    elif data == "end":
                        print(f"接收完毕，总共接收到的字节数: {byte_count}")
                        break  # 结束接收

                    if receiving:
                        # 直接保存接收到的数据
                        print(f"接收到: {data}")
                        file.write(data + "\n")  # 将数据写入文件
                        byte_count += 1  # 统计接收到的字节数

    except serial.SerialException as e:
        print(f"串口错误: {e}")
    except KeyboardInterrupt:
        print("程序终止")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()


if __name__ == "__main__":
    serial_port = "COM7"  # 根据实际情况修改串口号
    baud_rate = 115200     # 根据实际情况修改波特率
    output_file = "data.txt"

    receive_and_save_data(serial_port, baud_rate, output_file)
