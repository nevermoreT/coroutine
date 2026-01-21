# coroutine

coroutine tutorial from Revisiting Coroutines

## Linux 协程实现示例

这是一个基于C语言和汇编实现的简单协程库，用于Linux x86-64平台。

## 文件说明

### 协程库
- `coroutine.h` - 协程库头文件，定义API和数据结构
- `coroutine.c` - 协程库的C语言实现
- `context_switch.S` - 上下文切换的汇编实现（x86-64）
- `test.c` - 协程库测试程序

### Echo Server
- `echo_server.h` - Echo Server 头文件
- `echo_server.c` - 基于协程和 epoll 的 Echo Server 实现
- `echo_server_main.c` - Echo Server 主程序
- `test_client.c` - Echo Server 测试客户端
- `test_echo_server.sh` - Echo Server 自动化测试脚本

### 构建
- `Makefile` - 构建文件

## 功能特性

### 协程库
- 协程创建和销毁
- 协程启动（resume）
- 协程让出（yield）
- 上下文切换使用汇编实现，性能高效

### Echo Server
- 基于协程和 Linux epoll 的高性能网络服务器
- 每个客户端连接使用独立协程处理
- 非阻塞 I/O 与协程调度完美结合
- 支持并发处理多个客户端连接

## 编译说明

在Linux x86-64系统上，使用以下命令编译：

```bash
make
```

编译完成后，运行测试：

```bash
# 运行协程库测试
make test
# 或
./coroutine_test

# 运行 Echo Server 测试
chmod +x test_echo_server.sh
./test_echo_server.sh
```

### Echo Server 使用

启动服务器：

```bash
./echo_server [端口号]
# 默认端口 8888
```

在另一个终端运行测试客户端：

```bash
./test_client [主机] [端口]
# 默认连接到 127.0.0.1:8888
```

清理编译产物：

```bash
make clean
```

## 使用方法

```c
#include "coroutine.h"

void my_coroutine(void *arg) {
    // 协程代码
    for (int i = 0; i < 10; i++) {
        printf("Step %d\n", i);
        coroutine_yield(coroutine_current());
    }
}

int main() {
    int arg = 42;
    coroutine_t *co = coroutine_create(my_coroutine, &arg, 64 * 1024);
    coroutine_resume(co);
    coroutine_destroy(co);
    return 0;
}
```

## 实现细节

### 上下文切换

上下文切换使用x86-64汇编实现，保存和恢复以下寄存器：
- `rbx`, `rbp` - 基址寄存器
- `r12`-`r15` - 被调用者保存寄存器
- `rsp` - 栈指针
- `rip` - 指令指针（返回地址）

### 栈管理

每个协程拥有独立的栈空间，默认大小为64KB。栈指针需要16字节对齐以满足x86-64 ABI要求。

### 协程状态

- `COROUTINE_READY` - 就绪，尚未运行
- `COROUTINE_RUNNING` - 正在运行
- `COROUTINE_SUSPENDED` - 已挂起
- `COROUTINE_FINISHED` - 执行完毕

## Echo Server 示例

Echo Server 展示了如何使用协程库构建高性能网络服务器：

- 使用 epoll 进行事件驱动
- 每个客户端连接由独立协程处理
- 非阻塞 I/O 操作
- 协程自动调度，无需手动管理线程

## 注意事项

1. 本实现是简化版本，主要用于学习和演示
2. 实际生产环境建议使用更成熟的协程库（如libco、libtask等）
3. 栈溢出检查未实现，使用时需确保栈空间充足
4. 仅支持Linux x86-64平台
5. Echo Server 需要 Linux epoll 支持

## 许可证

本代码仅供学习和研究使用。
