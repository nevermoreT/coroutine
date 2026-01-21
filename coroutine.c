#include "coroutine.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

// 当前运行的协程
static coroutine_t *current_coroutine = NULL;

// 协程入口函数包装
static void coroutine_entry(void);

// 主协程上下文（用于保存主线程的上下文）
static context_t main_context;

coroutine_t *coroutine_create(void (*func)(void *), void *arg, size_t stack_size) {
    if (func == NULL || stack_size == 0) {
        return NULL;
    }
    
    coroutine_t *co = (coroutine_t *)malloc(sizeof(coroutine_t));
    if (co == NULL) {
        return NULL;
    }
    
    // 分配栈空间（需要16字节对齐）
    co->stack = malloc(stack_size);
    if (co->stack == NULL) {
        free(co);
        return NULL;
    }
    
    co->stack_size = stack_size;
    co->func = func;
    co->arg = arg;
    co->state = COROUTINE_READY;
    co->caller = NULL;
    
    // 栈顶对齐到16字节边界（x86-64 ABI要求）
    uintptr_t stack_ptr = (uintptr_t)co->stack + stack_size;
    stack_ptr = (stack_ptr & ~0xF);  // 16字节对齐
    co->stack_top = (char *)stack_ptr;
    
    // 初始化上下文
    memset(&co->ctx, 0, sizeof(context_t));
    
    // 设置初始栈指针（为协程入口函数预留空间）
    // 栈向下增长，需要预留空间用于函数调用
    char *initial_rsp = co->stack_top - 256;  // 预留256字节
    
    // 在栈上设置返回地址（模拟函数调用）
    // 这样当context_switch恢复时，ret指令会跳转到coroutine_entry
    void **stack_frame = (void **)initial_rsp;
    stack_frame[0] = (void *)coroutine_entry;  // 返回地址
    
    // 设置上下文
    co->ctx.rsp = initial_rsp;
    co->ctx.rip = (void *)coroutine_entry;
    
    return co;
}

void coroutine_destroy(coroutine_t *co) {
    if (co == NULL) {
        return;
    }
    
    if (co->stack != NULL) {
        free(co->stack);
    }
    
    free(co);
}

// 协程入口函数
static void coroutine_entry(void) {
    coroutine_t *co = current_coroutine;
    
    if (co && co->func) {
        co->state = COROUTINE_RUNNING;
        co->func(co->arg);
        co->state = COROUTINE_FINISHED;
    }
    
    // 协程执行完毕，返回到调用者
    if (co && co->caller) {
        coroutine_t *caller = co->caller;
        co->state = COROUTINE_FINISHED;
        current_coroutine = caller;
        caller->state = COROUTINE_RUNNING;
        
        // 切换回调用者
        context_switch(&co->ctx, &caller->ctx);
    } else {
        // 没有调用者，切换回主协程
        co->state = COROUTINE_FINISHED;
        current_coroutine = NULL;
        context_switch(&co->ctx, &main_context);
    }
}

void coroutine_resume(coroutine_t *co) {
    if (co == NULL || co->state == COROUTINE_FINISHED) {
        return;
    }
    
    coroutine_t *prev = current_coroutine;
    co->caller = prev;
    
    if (co->state == COROUTINE_READY) {
        // 首次启动协程
        co->state = COROUTINE_RUNNING;
        current_coroutine = co;
        
        // 保存当前上下文（主协程或之前的协程）
        context_t *from_ctx;
        if (prev == NULL) {
            // 从主协程切换
            from_ctx = &main_context;
        } else {
            // 从其他协程切换
            from_ctx = &prev->ctx;
            prev->state = COROUTINE_SUSPENDED;
        }
        
        // 切换到新协程
        context_switch(from_ctx, &co->ctx);
        
        // 如果返回到这里，说明协程已经yield或完成
        current_coroutine = prev;
        if (prev != NULL) {
            prev->state = COROUTINE_RUNNING;
        }
    } else if (co->state == COROUTINE_SUSPENDED) {
        // 恢复挂起的协程
        co->state = COROUTINE_RUNNING;
        current_coroutine = co;
        
        // 保存当前上下文
        context_t *from_ctx;
        if (prev == NULL) {
            from_ctx = &main_context;
        } else {
            from_ctx = &prev->ctx;
            prev->state = COROUTINE_SUSPENDED;
        }
        
        // 切换到目标协程
        context_switch(from_ctx, &co->ctx);
        
        // 如果返回到这里，说明协程已经yield或完成
        current_coroutine = prev;
        if (prev != NULL) {
            prev->state = COROUTINE_RUNNING;
        }
    }
}

void coroutine_yield(coroutine_t *co) {
    if (co == NULL || co != current_coroutine) {
        return;
    }
    
    coroutine_t *caller = co->caller;
    co->state = COROUTINE_SUSPENDED;
    
    // 切换到调用者（可能是主协程或其他协程）
    if (caller != NULL) {
        current_coroutine = caller;
        caller->state = COROUTINE_RUNNING;
        context_switch(&co->ctx, &caller->ctx);
    } else {
        // 没有调用者，切换回主协程
        current_coroutine = NULL;
        context_switch(&co->ctx, &main_context);
    }
    
    // 如果返回到这里，说明其他协程resume了这个协程
    current_coroutine = co;
    co->state = COROUTINE_RUNNING;
}

coroutine_t *coroutine_current(void) {
    return current_coroutine;
}
