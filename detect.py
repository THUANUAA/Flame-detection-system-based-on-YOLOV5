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

# RGB565 转 RGB888


def rgb565_to_rgb888(rgb565):
    r = (rgb565 >> 11) & 0x1F
    g = (rgb565 >> 5) & 0x3F
    b = (rgb565) & 0x1F
    r = (r << 3) | (r >> 2)
    g = (g << 2) | (g >> 4)
    b = (b << 3) | (b >> 2)
    return (r, g, b)

# 串口数据接收


def receive_serial_data(port, baudrate, width, height, data_ready_event, image_data_list):
    start_time = time.time()
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"串口 {port} 打开成功.")
    except serial.SerialException as e:
        print(f"打开串口失败: {e}")
        return None

    total_bytes = 0
    rgb_data = []
    buffer = bytearray()
    start_signal = b"begin\r"
    end_signal = b"end\r\n"
    recording = False

    print("准备接收数据...")

    while True:
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting)
            buffer.extend(data)

            if not recording:
                start_idx = buffer.find(start_signal)
                if start_idx != -1:
                    recording = True
                    buffer = buffer[start_idx + len(start_signal):]
                    print("检测到开始信号，开始接收数据...")

            if recording:
                end_idx = buffer.find(end_signal)
                if end_idx != -1:
                    valid_data = buffer[:end_idx]
                    while len(valid_data) >= 2:
                        value = int.from_bytes(valid_data[:2], byteorder='big')
                        rgb_data.append(rgb565_to_rgb888(value))
                        total_bytes += 2
                        valid_data = valid_data[2:]
                    print("检测到结束信号，数据接收完毕.")
                    image_data_list.append(rgb_data)
                    data_ready_event.set()  # 通知图像检测线程
                    break

    ser.close()
    elapsed_time = time.time() - start_time
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
    start_time = time.time()
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

    elapsed_time = time.time() - start_time
    print(f"图像保存完毕, 耗时 {elapsed_time:.2f} 秒.")
    return save_path

# 加载模型


def load_model(weights='runs/train/exp15/weights/best.pt', device='cpu'):
    start_time = time.time()
    global model
    device = torch.device(device)
    model = DetectMultiBackend(weights, device=device)
    elapsed_time = time.time() - start_time
    print(f"模型加载完毕, 耗时 {elapsed_time:.2f} 秒.")

# 处理图像检测


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
    start_time = time.time()

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

    elapsed_time = time.time() - start_time
    print(f"图像检测完成，线程结束：{save_path}, 耗时 {elapsed_time:.2f} 秒.")

# 主程序


def main():
    port = 'COM4'  # 串口号
    baudrate = 128000  # 波特率
    width = 320  # 图像宽度
    height = 240  # 图像高度
    save_dir = Path('runs/detect/exp')

    # 加载图像检测模型
    load_model(weights='runs/train/exp15/weights/best.pt', device='cpu')

    # 用于线程同步的事件
    data_ready_event = threading.Event()
    image_data_list = []  # 存储接收到的图像数据

    def serial_thread():
        while True:
            receive_serial_data(port, baudrate, width, height, data_ready_event, image_data_list)

    def detection_thread():
        while True:
            data_ready_event.wait()
            image_data = image_data_list.pop(0)
            save_path = create_image(image_data, width, height)
            threading.Thread(target=image_detection_thread, args=(save_path, save_dir), daemon=True).start()
            data_ready_event.clear()

    threading.Thread(target=serial_thread, daemon=True).start()
    threading.Thread(target=detection_thread, daemon=True).start()

    # 防止主线程退出
    while True:
        time.sleep(1)


if __name__ == "__main__":
    main()
