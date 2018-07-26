   .text
   .globl snippet_start
   .globl snippet_end
   
snippet_start:
   pushq %rsi
   lea 8(%rsp), %rsi           
   pushq %rax
   pushq %rdi
   pushq %rcx
   pushq %rdx
   pushq %r8
   pushq %r9
   movq $0x2222222222222222,%rdi
   movq $0x1111111111111111,%rax
   call *%rax
   movq %rax,%r11
   popq %r9
   popq %r8
   popq %rdx
   popq %rcx
   popq %rdi
   popq %rax
   popq %rsi
   addq $8,%rsp
   callq *%r11
   subq $8,%rsp
   movq %rsp,%rsi
   movq $0x2222222222222222,%rdi
   pushq %rax
   pushq %rdx
   movq $0x3333333333333333,%rax
   callq *%rax
   popq %rdx
   popq %rax
   ret
snippet_end:
   nop
   
