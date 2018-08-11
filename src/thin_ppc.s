   .text
   .globl snippet_start
   .globl snippet_end
   
snippet_start:
   mflr %r0
#   std %r0,16(%r1)
#   std %r31,-8(%r1)
   stdu %r1,-120(%r1)
   std %r12,96(%r1)
   std %r0,88(%r1)
   std %r3,80(%r1)
   std %r4,72(%r1)
   std %r5,64(%r1)
   std %r6,56(%r1)
   std %r7,48(%r1)
   std %r8,40(%r1)
   std %r9,32(%r1)
   std %r10,24(%r1)
   lis %r3,0x1111
   ori %r3,%r3,0x2222
   sldi %r3,%r3,32   
   oris %r3,%r3,0x3333
   ori %r3,%r3,0x4444
   addi %r4,%r1,88
   lis %r12,0x5555
   ori %r12,%r12,0x6666
   sldi %r12,%r12,32
   oris %r12,%r12,0x7777
   ori %r12,%r12,0x8888
   mtctr %r12
   bctrl
   mtctr %r3
   mr %r12,%r3
   ld %r0,88(%r1)
   ld %r3,80(%r1)
   ld %r4,72(%r1)
   ld %r5,64(%r1)
   ld %r6,56(%r1)
   ld %r7,48(%r1)
   ld %r8,40(%r1)
   ld %r9,32(%r1)
   ld %r10,24(%r1)
   addi %r1,%r1,120
   bctrl
   stdu %r1,-120(%r1)
   std %r12,96(%r1)   
   std %r0,88(%r1)
   std %r3,80(%r1)
   std %r4,72(%r1)
   std %r5,64(%r1)
   std %r6,56(%r1)
   std %r7,48(%r1)
   std %r8,40(%r1)
   std %r9,32(%r1)
   std %r10,24(%r1)
   lis %r3,0x1111
   ori %r3,%r3,0x2222
   sldi %r3,%r3,32
   oris %r3,%r3,0x3333
   ori %r3,%r3,0x4444
   addi %r4,%r1,88   
   lis %r12,0x9999
   ori %r12,%r12,0xaaaa
   sldi %r12,%r12,32   
   oris %r12,%r12,0xbbbb
   ori %r12,%r12,0xcccc
   mtctr %r12
   bctrl
   ld %r12,96(%r1)
   ld %r0,88(%r1)   
   ld %r3,80(%r1)
   ld %r4,72(%r1)
   ld %r5,64(%r1)
   ld %r6,56(%r1)
   ld %r7,48(%r1)
   ld %r8,40(%r1)
   ld %r9,32(%r1)
   ld %r10,24(%r1)
   addi %r1,%r1,120
   mtlr %r0
   blr
snippet_end:
   nop
   
