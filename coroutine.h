#ifndef COROUTINE_H
#define COROUTINE_H

#include <stddef.h>

// 协程状态
typedef enum {
    COROUTINE_READY,    // 就绪
    COROUTINE_RUNNING,  // 运行中
    COROUTINE_SUSPENDED, // 挂起
    COROUTINE_FINISHED   // 完成
} coroutine_state_t;

// 上下文结构体（用于保存寄存器）
// 必须在 coroutine_t 之前定义，因为 coroutine_t 需要使用它
typedef struct context {
    void *rbx;    // 保存 rbx
    void *rbp;    // 保存 rbp
    void *r12;    // 保存 r12
    void *r13;    // 保存 r13
    void *r14;    // 保存 r14
    void *r15;    // 保存 r15
    void *rsp;    // 保存栈指针
    void *rip;    // 保存指令指针
} context_t;

// 协程结构体
typedef struct coroutine {
    void *stack;              // 栈指针
    size_t stack_size;        // 栈大小
    coroutine_state_t state;  // 状态
    void (*func)(void *);     // 协程函数
    void *arg;                // 协程函数参数
    struct coroutine *caller; // 调用者协程
    char *stack_top;          // 栈顶（用于对齐）
    context_t ctx;            // 保存的上下文
} coroutine_t;

// API函数声明

/**
 * 创建协程
 * @param func 协程函数
 * @param arg 协程函数参数
 * @param stack_size 栈大小（字节）
 * @return 协程指针，失败返回NULL
 */
coroutine_t *coroutine_create(void (*func)(void *), void *arg, size_t stack_size);

/**
 * 销毁协程
 * @param co 协程指针
 */
void coroutine_destroy(coroutine_t *co);

/**
 * 切换到指定协程
 * @param from 当前协程的上下文（用于保存）
 * @param to 目标协程的上下文（用于恢复）
 */
void context_switch(context_t *from, context_t *to);

/**
 * 启动协程（首次运行）
 * @param co 协程指针
 */
void coroutine_resume(coroutine_t *co);

/**
 * 让出执行权（协程主动挂起）
 * @param co 协程指针
 */
void coroutine_yield(coroutine_t *co);

/**
 * 获取当前运行的协程
 * @return 当前协程指针
 */
coroutine_t *coroutine_current(void);

#endif // COROUTINE_H
