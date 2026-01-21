CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
ASFLAGS = -g
LDFLAGS = 

# 协程库目标文件
COROUTINE_OBJS = coroutine.o context_switch.o
COROUTINE_LIB = libcoroutine.a

# 测试程序
TEST_OBJS = test.o
TEST_TARGET = coroutine_test

# Echo Server
ECHO_SERVER_OBJS = echo_server.o echo_server_main.o
ECHO_SERVER_TARGET = echo_server

# 测试客户端
CLIENT_OBJS = test_client.o
CLIENT_TARGET = test_client

# 默认目标
all: $(COROUTINE_LIB) $(TEST_TARGET) $(ECHO_SERVER_TARGET) $(CLIENT_TARGET)

# 创建协程库
$(COROUTINE_LIB): $(COROUTINE_OBJS)
	ar rcs $@ $^

# 测试程序
$(TEST_TARGET): $(TEST_OBJS) $(COROUTINE_LIB)
	$(CC) $(LDFLAGS) -o $@ $< -L. -lcoroutine

# Echo Server
$(ECHO_SERVER_TARGET): $(ECHO_SERVER_OBJS) $(COROUTINE_LIB)
	$(CC) $(LDFLAGS) -o $@ $(ECHO_SERVER_OBJS) -L. -lcoroutine

# 测试客户端
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $<

# 编译C源文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译汇编源文件
%.o: %.S
	$(CC) $(ASFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(COROUTINE_OBJS) $(COROUTINE_LIB)
	rm -f $(TEST_OBJS) $(TEST_TARGET)
	rm -f $(ECHO_SERVER_OBJS) $(ECHO_SERVER_TARGET)
	rm -f $(CLIENT_OBJS) $(CLIENT_TARGET)

# 运行协程测试
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# 运行 echo server（后台）
run-server: $(ECHO_SERVER_TARGET)
	./$(ECHO_SERVER_TARGET) &

# 运行客户端测试
run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

.PHONY: all clean test run-server run-client
