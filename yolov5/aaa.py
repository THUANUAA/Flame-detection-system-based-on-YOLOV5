import time
import threading
import torch
import os
from pathlib import Path
from utils.general import (
    non_max_suppression,
    scale_boxes,
)
from utils.dataloaders import LoadImages
from models.common import DetectMultiBackend
from ultralytics.utils.plotting import Annotator, colors
import cv2

model = None
device = None


def load_model(weights='runs/train/exp15/weights/best.pt', device='cpu'):
    """加载模型"""
    global model
    device = torch.device(device)  # 转换为 torch.device 对象
    model = DetectMultiBackend(weights, device=device)


def process_image(im, im0s, path, save_dir, conf_thres, iou_thres, max_det):
    """处理单张图片的推理与保存结果"""
    # 解码
    im = torch.from_numpy(im).to(device).float()  # 转为 Tensor
    im /= 255.0  # 归一化

    # 确保只有3维的张量（加入批次维度）
    if len(im.shape) == 3:
        im = im[None]  # 添加 batch 维度

    # 推理
    pred = model(im)  # 推理
    pred = non_max_suppression(pred, conf_thres, iou_thres, max_det=max_det)

    # 处理每张图片的检测结果
    for det in pred:  # 每张图片的检测
        if len(det):
            det[:, :4] = scale_boxes(im.shape[2:], det[:, :4], im0s.shape).round()
            annotator = Annotator(im0s, line_width=3)

            # 绘制框
            for *xyxy, conf, cls in reversed(det):
                c = int(cls)  # 类别索引
                label = f"{model.names[c]} {conf:.2f}"
                annotator.box_label(xyxy, label, color=colors(c))

            # 保存结果图片
            im0 = annotator.result()
            save_path = save_dir / Path(path).name
            cv2.imwrite(str(save_path), im0)

            # 输出信号：检测到目标
            print(f"检测到目标，已保存图像：{save_path}")
        else:
            print("未检测到目标")


def run(weights='runs/train/exp15/weights/best.pt', source_dir='E:/C&C++/project/sample/images', imgsz=640, conf_thres=0.25, iou_thres=0.45, max_det=1000, device='cpu', save_dir='runs/detect/exp'):
    # 记录总的运行时间
    start_time = time.time()

    # 加载模型
    load_model(weights, device)

    # 获取图片路径列表
    image_paths = [os.path.join(source_dir, f"fire.{i}.png")
                   for i in range(1, 101)]  # 假设图片命名为 fire.1.png 到 fire.700.png

    # 创建保存目录
    save_dir = Path(save_dir)
    save_dir.mkdir(parents=True, exist_ok=True)

    # 记录每张图像处理的时间
    for path in image_paths:
        print(f"开始处理图片：{path}")
        # 加载图片
        dataset = LoadImages(path, img_size=imgsz, stride=model.stride)

        for data in dataset:
            path = data[0]
            im = data[1]
            im0s = data[2]

            # 使用多线程处理每张图片
            threading.Thread(target=process_image, args=(
                im, im0s, path, save_dir, conf_thres, iou_thres, max_det)).start()

    # 总的运行时间
    total_time = time.time() - start_time
    print(f"所有图片处理完成，耗时：{total_time:.4f}秒")


if __name__ == "__main__":
    run()
