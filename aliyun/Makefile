# 定义编译器
CC = gcc

# 定义编译选项
CFLAGS = -Wall -I/usr/include -lpaho-mqtt3c -lsqlite3

# 定义源文件和目标文件
SRC = mqtt.c
OBJ = mqtt

# 默认目标
all: $(OBJ)

# 生成可执行文件
$(OBJ): $(SRC)
	$(CC) $(SRC) -o $(OBJ) $(CFLAGS)

# 清理目标文件
clean:
	rm -f $(OBJ)

