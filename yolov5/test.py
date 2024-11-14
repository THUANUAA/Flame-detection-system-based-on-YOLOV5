import serial
import torch
import os
from pathlib import Path
from PIL import Image
import cv2
import threading
from utils.general import non_max_suppression, scale_boxes
from utils.dataloaders import LoadImages
from models.common import DetectMultiBackend
from ultralytics.utils.plotting import Annotator, colors
import time

# RGB565到RGB888转换


def rgb565_to_rgb888(rgb565):
    r = (rgb565 >> 11) & 0x1F
    g = (rgb565 >> 5) & 0x3F
    b = (rgb565) & 0x1F
    r = (r << 3) | (r >> 2)
    g = (g << 2) | (g >> 4)
    b = (b << 3) | (b >> 2)
    return (r, g, b)

# 串口接收数据并转换为RGB888格式


def receive_serial_data(port, baudrate, width, height, data_ready_event, image_data_list):
    start_time = time.time()  # 记录开始时间
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"串口 {port} 打开成功.")
    except serial.SerialException as e:
        print(f"打开串口失败: {e}")
        return None

    total_bytes = 0  # 记录接收到的总字节数
    rgb_data = []  # 用于存储RGB数据
    buffer = bytearray()  # 用于存储接收到的数据
    start_signal = b"begin\r"  # 开始信号
    end_signal = b"end\r\n"  # 结束信号
    recording = False  # 用于指示是否开始记录数据

    print("准备接收数据...")

    while True:
        if ser.in_waiting > 0:  # 检查是否有数据可读
            data = ser.read(ser.in_waiting)  # 读取所有可用数据
            buffer.extend(data)  # 将读取的数据添加到缓冲区

            if not recording:
                # 如果尚未开始记录，检查是否收到 "begin" 信号
                start_idx = buffer.find(start_signal)
                if start_idx != -1:
                    recording = True
                    buffer = buffer[start_idx + len(start_signal):]  # 清理开始信号之前的部分
                    print("检测到开始信号，开始接收数据...")

            if recording:
                # 检查是否收到 "end" 信号
                end_idx = buffer.find(end_signal)
                if end_idx != -1:
                    valid_data = buffer[:end_idx]  # 获取有效数据
                    while len(valid_data) >= 2:  # 确保每次处理2个字节
                        value = int.from_bytes(valid_data[:2], byteorder='big')  # 转换为大端字节序
                        rgb_data.append(rgb565_to_rgb888(value))  # 转换为RGB888格式
                        total_bytes += 2  # 更新总字节数
                        valid_data = valid_data[2:]  # 移除已处理的两个字节
                    print("检测到结束信号，数据接收完毕.")
                    # 将图像数据加入列表并通知检测线程
                    image_data_list.append(rgb_data)
                    data_ready_event.set()  # 通知检测线程
                    break

    ser.close()  # 关闭串口
    elapsed_time = time.time() - start_time  # 计算接收数据所花费的时间
    print(f"接收数据结束, 总共接收到 {total_bytes} 字节, 耗时 {elapsed_time:.2f} 秒.")

# 获取下一个图像编号


def get_next_image_number():
    if not os.path.exists("image.txt"):
        return 1

    with open("image.txt", "r") as f:
        lines = f.readlines()
        if len(lines) == 0:
            return 1
        last_line = lines[-1].strip()
        start_idx = last_line.rfind("image") + len("image")
        end_idx = last_line.rfind(".png")
        if start_idx != -1 and end_idx != -1:
            last_image_number = int(last_line[start_idx:end_idx])
            return last_image_number + 1
        else:
            return 1

# 创建并保存图像


def create_image(rgb_data, width, height):
    start_time = time.time()  # 记录开始时间
    if len(rgb_data) != width * height:
        raise ValueError("数据长度与指定的图像尺寸不匹配")

    image = Image.new("RGB", (width, height))
    image.putdata(rgb_data)
    image = image.transpose(Image.FLIP_TOP_BOTTOM).rotate(270)

    image_number = get_next_image_number()
    save_path = f'../sample/image/image{image_number}.png'
    image.save(save_path)
    print(f"图像已保存为 {save_path}")

    with open("image.txt", "a") as f:
        f.write(save_path + "\n")
    print(f"图像路径 {save_path} 已写入 image.txt")

    elapsed_time = time.time() - start_time  # 计算保存图像所花费的时间
    print(f"图像保存完毕, 耗时 {elapsed_time:.2f} 秒.")
    return save_path

# 加载模型


def load_model(weights='runs/train/exp15/weights/best.pt', device='cpu'):
    start_time = time.time()  # 记录开始时间
    global model
    device = torch.device(device)
    model = DetectMultiBackend(weights, device=device)
    elapsed_time = time.time() - start_time  # 计算加载模型所花费的时间
    print(f"模型加载完毕, 耗时 {elapsed_time:.2f} 秒.")

# 图像检测


def process_image(im, im0s, path, save_dir, conf_thres, iou_thres, max_det):
    pred = model(im)
    pred = non_max_suppression(pred, conf_thres, iou_thres, max_det)

    for det in pred:
        if len(det):
            det[:, :4] = scale_boxes(im.shape[2:], det[:, :4], im0s.shape).round()
            annotator = Annotator(im0s, line_width=3)

            for *xyxy, conf, cls in reversed(det):
                c = int(cls)
                label = f"{model.names[c]} {conf:.2f}"
                annotator.box_label(xyxy, label, color=colors(c))

            im0 = annotator.result()
            save_path = save_dir / Path(path).name
            cv2.imwrite(str(save_path), im0)

# 图像检测线程


def image_detection_thread(save_path, save_dir, conf_thres=0.25, iou_thres=0.45, max_det=1000):
    print(f"开始进行图像检测：{save_path}")
    start_time = time.time()  # 记录开始时间

    # 图像检测
    image = cv2.imread(save_path)
    dataset = LoadImages(save_path, img_size=640)
    for data in dataset:
        path = data[0]
        im = data[1]
        im0s = data[2]
        im = torch.from_numpy(im).to('cpu').float() / 255.0
        if len(im.shape) == 3:
            im = im[None]

        process_image(im, im0s, path, save_dir, conf_thres, iou_thres, max_det)

    elapsed_time = time.time() - start_time  # 计算图像检测所花费的时间
    print(f"图像检测完成，线程结束：{save_path}, 耗时 {elapsed_time:.2f} 秒.")

# 主程序


def main():
    port = 'COM4'  # 串口号
    baudrate = 128000    # 波特率
    width = 320          # 图像宽度
    height = 240         # 图像高度
    save_dir = Path('runs/detect/exp')

    # 加载图像检测模型
    load_model(weights='runs/train/exp15/weights/best.pt', device='cpu')

    # 用于线程同步的事件
    data_ready_event = threading.Event()
    image_data_list = []  # 用来存储接收到的图像数据

    def serial_thread():
        while True:
            # 串口接收数据
            receive_serial_data(port, baudrate, width, height, data_ready_event, image_data_list)

    def detection_thread():
        while True:
            # 等待接收信号，接收到信号后处理图像
            data_ready_event.wait()
            # 获取最新接收到的图像数据
            image_data = image_data_list.pop(0)
            # 保存图像并获得路径
            save_path = create_image(image_data, width, height)
            # 启动新的图像检测线程
            threading.Thread(target=image_detection_thread, args=(save_path, save_dir), daemon=True).start()
            # 清除信号，准备下次通知
            data_ready_event.clear()

    # 启动串口接收线程和图像检测线程
    threading.Thread(target=serial_thread, daemon=True).start()
    threading.Thread(target=detection_thread, daemon=True).start()

    # 防止主线程退出
    while True:
        time.sleep(1)


if __name__ == "__main__":
    main()
