
import os

# 指定文件夹路径和输出的txt文件路径
folder_path = 'E:/C&C++/project/sample/labels'  # 替换为你文件夹的路径
output_file = 'E:/C&C++/project/sample/datapath/train.txt'        # 输出txt文件名

# 获取文件夹中的文件名
file_names = os.listdir(folder_path)

# 将文件名按行写入txt文件
with open(output_file, 'w') as f:
    for file_name in file_names:
        if file_name != 'classes.txt':
            if file_name.endswith('.txt'):  # 如果是.txt文件
                file_name = file_name.replace('.txt', '.png')  # 将后缀改为.png
            f.write('E:/C&C++/project/sample/images/'+file_name + '\n')

print(f"文件名已写入到 {output_file}")
