CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
ASFLAGS = -g
LDFLAGS = 

# 目标文件
OBJS = coroutine.o context_switch.o test.o
TARGET = coroutine_test

# 默认目标
all: $(TARGET)

# 链接可执行文件
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

# 编译C源文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译汇编源文件
%.o: %.S
	$(CC) $(ASFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) $(TARGET)

# 运行测试
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
