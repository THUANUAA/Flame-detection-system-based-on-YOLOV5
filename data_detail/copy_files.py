import os
import shutil

# 定义源文件夹和目标文件夹路径
source_folder = 'E:/C&C++/project/sample/images'
target_folder = 'E:/C&C++/project/sample/a'

# 如果目标文件夹不存在，则创建
if not os.path.exists(target_folder):
    os.makedirs(target_folder)

# 遍历源文件夹中的所有文件
for filename in os.listdir(source_folder):
    # 构建完整的源文件路径
    source_file = os.path.join(source_folder, filename)

    # 检查是否是文件（而不是目录）
    if os.path.isfile(source_file):
        # 分离文件名和扩展名
        name, ext = os.path.splitext(filename)

        # 构建目标文件名，修改后缀为.jpg
        new_filename = name + '.jpg'
        target_file = os.path.join(target_folder, new_filename)

        # 复制文件到目标文件夹，并修改后缀
        shutil.copy(source_file, target_file)

print("文件后缀修改完成并保存到目标文件夹。")
