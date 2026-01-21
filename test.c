#include "coroutine.h"
#include <stdio.h>
#include <unistd.h>

// 测试协程1
static void coroutine_func1(void *arg) {
    int id = *(int *)arg;
    printf("协程 %d: 开始执行\n", id);
    
    for (int i = 0; i < 5; i++) {
        printf("协程 %d: 执行步骤 %d\n", id, i + 1);
        coroutine_yield(coroutine_current());
        usleep(100000);  // 模拟一些工作
    }
    
    printf("协程 %d: 执行完毕\n", id);
}

// 测试协程2
static void coroutine_func2(void *arg) {
    int id = *(int *)arg;
    printf("协程 %d: 开始执行\n", id);
    
    for (int i = 0; i < 3; i++) {
        printf("协程 %d: 执行步骤 %d\n", id, i + 1);
        coroutine_yield(coroutine_current());
        usleep(150000);  // 模拟一些工作
    }
    
    printf("协程 %d: 执行完毕\n", id);
}

int main(void) {
    printf("=== 协程测试程序 ===\n\n");
    
    int arg1 = 1;
    int arg2 = 2;
    
    // 创建两个协程
    coroutine_t *co1 = coroutine_create(coroutine_func1, &arg1, 64 * 1024);
    coroutine_t *co2 = coroutine_create(coroutine_func2, &arg2, 64 * 1024);
    
    if (co1 == NULL || co2 == NULL) {
        fprintf(stderr, "创建协程失败\n");
        return 1;
    }
    
    printf("创建协程1和协程2成功\n\n");
    
    // 交替执行两个协程
    printf("开始交替执行协程...\n\n");
    
    while (co1->state != COROUTINE_FINISHED || co2->state != COROUTINE_FINISHED) {
        if (co1->state != COROUTINE_FINISHED) {
            printf("--- 切换到协程1 ---\n");
            coroutine_resume(co1);
        }
        
        if (co2->state != COROUTINE_FINISHED) {
            printf("--- 切换到协程2 ---\n");
            coroutine_resume(co2);
        }
    }
    
    printf("\n所有协程执行完毕\n");
    
    // 清理资源
    coroutine_destroy(co1);
    coroutine_destroy(co2);
    
    printf("\n=== 测试完成 ===\n");
    return 0;
}
