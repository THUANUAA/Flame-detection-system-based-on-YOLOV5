# 获取用户输入的生成数量
num_files = int(input("请输入要生成的文件数量: "))
num_index = int(input("从哪一个文件开始生成："))
# 文件路径的前缀和后缀
file_prefix = "E:/C&C++/project/sample/images/fire."
file_suffix = ".png"

# 输出文件名的目标txt文件
output_file = 'E:/C&C++/project/sample/datapath/val.txt'

# 生成文件名并写入txt文件
with open(output_file, 'w') as f:
    for i in range(num_index, num_files + 1+num_index):  # 根据用户输入生成文件名
        file_name = f"{file_prefix}{i}{file_suffix}"  # 拼接文件名
        f.write(file_name + '\n')  # 写入文件
        print(f"生成文件名: {file_name}")

print(f"所有文件名已写入到 {output_file}")
