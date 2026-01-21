# Linux 协程实现示例

这是一个基于C语言和汇编实现的简单协程库，用于Linux x86-64平台。

## 文件说明

- `coroutine.h` - 协程库头文件，定义API和数据结构
- `coroutine.c` - 协程库的C语言实现
- `context_switch.S` - 上下文切换的汇编实现（x86-64）
- `test.c` - 测试程序
- `Makefile` - 构建文件

## 功能特性

- 协程创建和销毁
- 协程启动（resume）
- 协程让出（yield）
- 上下文切换使用汇编实现，性能高效

## 编译说明

在Linux x86-64系统上，使用以下命令编译：

```bash
make
```

编译完成后，运行测试：

```bash
make run
# 或
./coroutine_test
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

## 注意事项

1. 本实现是简化版本，主要用于学习和演示
2. 实际生产环境建议使用更成熟的协程库（如libco、libtask等）
3. 栈溢出检查未实现，使用时需确保栈空间充足
4. 仅支持Linux x86-64平台

## 许可证

本代码仅供学习和研究使用。
