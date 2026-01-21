# x86-64 上下文切换汇编实现
# 保存和恢复寄存器状态

.text
.global context_switch

# context_switch(from, to)
# rdi = from (context_t *)
# rsi = to (context_t *)
context_switch:
    # 保存当前上下文到 from
    movq %rbx, 0(%rdi)   # 保存 rbx
    movq %rbp, 8(%rdi)   # 保存 rbp
    movq %r12, 16(%rdi)  # 保存 r12
    movq %r13, 24(%rdi)  # 保存 r13
    movq %r14, 32(%rdi)  # 保存 r14
    movq %r15, 40(%rdi)  # 保存 r15
    movq %rsp, 48(%rdi)  # 保存栈指针
    movq (%rsp), %rax    # 获取返回地址（rip）
    movq %rax, 56(%rdi)  # 保存返回地址（rip）
    
    # 恢复目标上下文到 to
    movq 0(%rsi), %rbx   # 恢复 rbx
    movq 8(%rsi), %rbp   # 恢复 rbp
    movq 16(%rsi), %r12  # 恢复 r12
    movq 24(%rsi), %r13  # 恢复 r13
    movq 32(%rsi), %r14  # 恢复 r14
    movq 40(%rsi), %r15  # 恢复 r15
    movq 48(%rsi), %rsp  # 恢复栈指针
    movq 56(%rsi), %rax  # 获取返回地址（rip）
    movq %rax, (%rsp)    # 设置返回地址
    
    ret                 # 返回到新的rip地址
