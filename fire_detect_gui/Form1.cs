using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using OpenCvSharp;
using OpenCvSharp.Extensions;
using Microsoft.WindowsAPICodePack.Dialogs;
using Microsoft.ML.OnnxRuntime;
using Microsoft.ML.OnnxRuntime.Tensors;
using System.Drawing;

using CvSize = OpenCvSharp.Size; // 为 OpenCvSharp.Size 定义别名
using CvPoint = OpenCvSharp.Point; // 为 OpenCvSharp.Point 定义别名

namespace fire_detect_gui
{
    public partial class Form1 : Form
    {
        private VideoCapture capture;  // 视频捕获对象，用于从摄像头获取视频帧
        private bool capFlag = false;  // 标记是否正在使用摄像头
        private string imageFile;  // 用于存储选择的图片文件路径
        private readonly InferenceSession session;  // ONNX模型推理会话
        private readonly float confidenceThreshold = 0.5f;  // 置信度阈值
        private readonly string saveDirectory = @"E:\C&C++\project\fire_detect_gui\detect_image";  // 检测后图片保存目录
        private readonly string saveLogPath = @"E:\C&C++\project\fire_detect_gui\detect_image\image.txt";  // 保存图片路径日志的文件

        public Form1()
        {
            InitializeComponent();
            session = new InferenceSession("E:\\C&C++\\project\\fire_detect_gui\\best.onnx");  // 加载ONNX模型
        }

        // 选择文件按钮事件处理
        private void Choice_file_btn_Click(object sender, EventArgs e)
        {
            using (var openFileDialog = new CommonOpenFileDialog
            {
                Title = "选择一个文件",  // 文件选择框标题
                IsFolderPicker = false,  // 选择文件而非文件夹
                Multiselect = false,  // 不允许多选文件
                Filters = {
                    new CommonFileDialogFilter("检测文件", "*.png;*.jpg;*.mp4"),  // 只允许选择图像和视频文件
                    new CommonFileDialogFilter("所有文件", "*.*")  // 允许选择所有文件
                }
            })
            {
                if (openFileDialog.ShowDialog() == CommonFileDialogResult.Ok)  // 显示文件选择框
                {
                    imageFile = openFileDialog.FileName;  // 获取选择的文件路径
                }
            }
        }

        // 实时检测按钮事件处理
        private void Real_time_btn_Click(object sender, EventArgs e)
        {
            capFlag = true;
            capture = new VideoCapture(0);  // 打开默认摄像头

            var timer = new Timer { Interval = 30 };  // 设置定时器，间隔30ms（约33fps）
            timer.Tick += (s, args) =>
            {
                using (var frame = new Mat())  // 用于存储每一帧的图像
                {
                    capture.Read(frame);  // 从摄像头读取一帧图像
                    if (frame.Empty())  // 如果读取失败，则返回
                        return;

                    var output = DetectFire(frame);  // 检测火焰
                    var detections = PostProcess(output, frame);  // 后处理，得到检测框

                    // 在图像上绘制检测框
                    DrawDetections(frame, detections);

                    // 显示带框的图像
                    var bitmap = OpenCvSharp.Extensions.BitmapConverter.ToBitmap(frame);
                    image.Image = bitmap;
                    image.Refresh();

                    // 更新检测状态
                    UpdateStatus(detections.Count > 0 ? "检测到火焰！" : "未检测到火焰", detections.Count > 0 ? Color.Red : Color.Green);
                    UpdateResult(detections.Count, Color.DarkBlue);
                }
            };
            timer.Start();  // 启动定时器，开始实时视频捕获
        }

        // 开始按钮事件处理（检测单张图片）
        private void Start_btn_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(imageFile) && !capFlag)  // 如果没有选择文件也没有启动摄像头
            {
                MessageBox.Show("请选择一个文件进行检测！");
                return;
            }

            using (var inputImage = new Mat(imageFile))  // 读取选择的图片
            {
                var output = DetectFire(inputImage);  // 检测火焰
                var detections = PostProcess(output, inputImage);  // 后处理，得到检测框

                // 在图像上绘制检测框
                DrawDetections(inputImage, detections);

                // 显示检测结果
                var bitmap = OpenCvSharp.Extensions.BitmapConverter.ToBitmap(inputImage);
                image.Image = bitmap;
                image.Refresh();

                // 更新状态和结果
                UpdateStatus(detections.Count > 0 ? "检测到火焰！" : "未检测到火焰", detections.Count > 0 ? Color.Red : Color.Green);
                UpdateResult(detections.Count, Color.DarkBlue);

                // 保存检测后的图片
                SaveDetectionResult(bitmap);
            }
        }

        // 停止按钮事件处理
        private void Stop_btn_Click(object sender, EventArgs e)
        {
            UpdateStatus("等待检测", Color.Red);
            if (capFlag && capture != null)
            {
                capture.Release();  // 释放摄像头资源
                capFlag = false;  // 更新状态
            }
        }

        // 更新状态文本
        private void UpdateStatus(string message, Color color)
        {
            status.Text = message;
            status.ForeColor = color;
        }

        // 更新检测到的目标数量
        private void UpdateResult(int count, Color color)
        {
            result.Text = count.ToString();
            result.ForeColor = color;
        }

        // 使用ONNX模型进行火焰检测
        private float[] DetectFire(Mat frame)
        {
            var inputTensor = ConvertToTensor(frame);  // 将图像转换为Tensor
            using (var results = session.Run(new List<NamedOnnxValue> { NamedOnnxValue.CreateFromTensor("images", inputTensor) }))  // 进行推理
            {
                var output = results.First().AsEnumerable<float>().ToArray();  // 获取输出结果
                return output;
            }
        }

        // 将图像转换为模型需要的Tensor格式
        private DenseTensor<float> ConvertToTensor(Mat frame)
        {
            const int modelSize = 640;  // 模型输入大小
            using (var resized = new Mat())
            {
                Cv2.Resize(frame, resized, new CvSize(modelSize, modelSize));  // 调整图像大小为640x640
                resized.ConvertTo(resized, MatType.CV_32F, 1 / 255.0);  // 将图像归一化到[0, 1]

                var tensor = new DenseTensor<float>(new[] { 1, 3, modelSize, modelSize });  // 创建Tensor

                // 将图像像素按通道存储到Tensor
                for (int y = 0; y < modelSize; y++)
                {
                    for (int x = 0; x < modelSize; x++)
                    {
                        var pixel = resized.At<Vec3f>(y, x);
                        tensor[0, 0, y, x] = pixel.Item2;  // R
                        tensor[0, 1, y, x] = pixel.Item1;  // G
                        tensor[0, 2, y, x] = pixel.Item0;  // B
                    }
                }
                return tensor;
            }
        }

        // 后处理，获取检测框信息
        private List<Detection> PostProcess(float[] output, Mat frame)
        {
            int imgWidth = frame.Width;  // 图像宽度
            int imgHeight = frame.Height;  // 图像高度
            var detections = new List<Detection>();

            // 每6个值为一组（x, y, width, height, confidence, classId）
            for (int i = 0; i < output.Length; i += 6)
            {
                float confidence = output[i + 4];
                int classId = (int)output[i + 5];  // 获取分类ID

                if (confidence > confidenceThreshold && classId == 0)  // 过滤低置信度和非火焰检测
                {
                    float x = output[i];  // 中心x坐标（归一化）
                    float y = output[i + 1];  // 中心y坐标（归一化）
                    float width = output[i + 2];  // 宽度（归一化）
                    float height = output[i + 3];  // 高度（归一化）

                    // 将归一化坐标转换为图像的实际坐标
                    int xMin = (int)((x - width / 2) * imgWidth);
                    int yMin = (int)((y - height / 2) * imgHeight);
                    int xMax = (int)((x + width / 2) * imgWidth);
                    int yMax = (int)((y + height / 2) * imgHeight);

                    detections.Add(new Detection
                    {
                        XMin = xMin,
                        YMin = yMin,
                        XMax = xMax,
                        YMax = yMax,
                        Confidence = confidence
                    });
                }
            }

            return detections;
        }

        // 绘制检测框
        private void DrawDetections(Mat frame, List<Detection> detections)
        {
            foreach (var detection in detections)
            {
                // 在图像上绘制矩形框
                Cv2.Rectangle(frame, new CvPoint(detection.XMin, detection.YMin),
                    new CvPoint(detection.XMax, detection.YMax),
                    new Scalar(0, 0, 255), 2);
            }
        }

        // 保存检测结果到文件
        private void SaveDetectionResult(Bitmap bitmap)
        {
            if (!System.IO.Directory.Exists(saveDirectory))  // 检查目录是否存在
            {
                System.IO.Directory.CreateDirectory(saveDirectory);  // 创建目录
            }

            string fileName = $"{DateTime.Now:yyyyMMddHHmmss}.jpg";  // 使用当前时间作为文件名
            string filePath = System.IO.Path.Combine(saveDirectory, fileName);

            bitmap.Save(filePath);  // 保存图片

            // 记录图片路径到文件
            System.IO.File.AppendAllText(saveLogPath, $"{filePath}\n");
        }
    }

    // 检测结果类
    public class Detection
    {
        public int XMin { get; set; }
        public int YMin { get; set; }
        public int XMax { get; set; }
        public int YMax { get; set; }
        public float Confidence { get; set; }
    }
}
