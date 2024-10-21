from PIL import Image


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


def load_decimal_data(file_path, width, height):
    """逐行加载十进制数据并转换为RGB888格式"""
    rgb_data = []

    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            if line:  # 确保不处理空行
                # 将行分割为数字，并逐个转换
                for num_str in line.split():
                    try:
                        pixel = int(num_str)  # 逐个转换为整数
                        rgb_data.append(rgb565_to_rgb888(pixel))
                    except ValueError as e:
                        print(f"无法转换 '{num_str}' 为整数: {e}")

    # 确保数据长度正确
    if len(rgb_data) != width * height:
        raise ValueError("数据长度与指定的图像尺寸不匹配")

    return rgb_data


def main():
    # 定义图像的宽度和高度
    width = 320  # 根据您的图像尺寸设置
    height = 240  # 根据您的图像尺寸设置

    # 加载十进制数据
    rgb_data = load_decimal_data('received_data.txt', width, height)

    # 创建图像
    image = Image.new("RGB", (width, height))
    image.putdata(rgb_data)

    # 保存图像
    image.save('output_image.png')
    print("图像已保存为 output_image.png")


if __name__ == "__main__":
    main()
