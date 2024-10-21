# 定义编译器
CC = g++

# 定义编译选项
CFLAGS = -Wall -I/usr/include -lpaho-mqtt3c -lpython3.12

# 定义源文件和目标文件
SRC = client.cpp
OBJ = client

# 默认目标
all: $(OBJ)

# 生成可执行文件
$(OBJ): $(SRC)
	$(CC) $(SRC) -o $(OBJ) $(CFLAGS)

# 清理目标文件
clean:
	rm -f $(OBJ)

