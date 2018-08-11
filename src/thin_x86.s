   .text
   .globl snippet_start
   .globl snippet_end
   
snippet_start:
   pushq %rsi
   lea 8(%rsp), %rsi  #Save the location of the retaddr into the a parameter
   pushq %rax         #Save all parameter registers, which are live across a call
   pushq %rdi
   pushq %rcx
   pushq %rdx
   pushq %r8
   pushq %r9
   lea -64(%rsp), %rsp
   vmovq %xmm0, 56(%rsp)
   vmovq %xmm1, 48(%rsp)
   vmovq %xmm2, 40(%rsp)
   vmovq %xmm3, 32(%rsp)
   vmovq %xmm4, 24(%rsp)
   vmovq %xmm5, 16(%rsp)
   vmovq %xmm6, 8(%rsp)
   vmovq %xmm7, (%rsp)   
   movq $0x2222222222222222,%rdi #Load the binding val into a parameter
   movq $0x1111111111111111,%rax 
   call *%rax         #Call 'pre()' with the binding and location of original ret
   movq %rax,%r11     #'pre' returns the wrappee in rax, save it
   vmovq 56(%rsp), %xmm0
   vmovq 48(%rsp), %xmm1
   vmovq 40(%rsp), %xmm2
   vmovq 32(%rsp), %xmm3
   vmovq 24(%rsp), %xmm4
   vmovq 16(%rsp), %xmm5
   vmovq 8(%rsp), %xmm6
   vmovq 0(%rsp), %xmm7
   lea 64(%rsp), %rsp
   popq %r9           #Restore the parameter registers
   popq %r8
   popq %rdx
   popq %rcx
   popq %rdi
   popq %rax
   popq %rsi
   addq $8,%rsp #Overwrite original retaddr with one that comes back to this tramp
   callq *%r11 #Call the wrappee (returned from pre)
   subq $8,%rsp 
   movq %rsp,%rsi #Save the location of the retaddr into a parameter
   movq $0x2222222222222222,%rdi #Save the binding val into a parameter
   pushq %rax    #Save the registers used for return values
   pushq %rdx
   lea -16(%rsp), %rsp
   vmovq %xmm0, 8(%rsp)
   vmovq %xmm1, 0(%rsp)
   movq $0x3333333333333333,%rax
   callq *%rax   #Call post, which will put the original return address back
   vmovq 8(%rsp), %xmm0
   vmovq 0(%rsp), %xmm1
   lea 16(%rsp), %rsp
   popq %rdx     #Restore the regsiters used for return values
   popq %rax
   ret
snippet_end:
   nop
   
