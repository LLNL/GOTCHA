/*
This file is part of GOTCHA.  For copyright information see the COPYRIGHT
file in the top level directory, or at
https://github.com/LLNL/gotcha/blob/master/COPYRIGHT
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (as published by the Free
Software Foundation) version 2.1 dated February 1999.  This program is
distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the terms and conditions of the GNU Lesser General Public License
for more details.  You should have received a copy of the GNU Lesser General
Public License along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <stdlib.h>
#include "thin.h"
#include "libc_wrappers.h"
#include "tool.h"
#include "gotcha_utils.h"

#define INITIAL_SAVESTATE_SIZE 128
static __thread void *savestate = NULL;

typedef struct {
   void *stack_location;
   void *retaddr;
   void *opaque_handle;
   void *unused;
} stacksave_entry_t;

typedef struct {
   unsigned long savesize;
   unsigned long cursize;
   void *unused;
   void *unused2;
} stacksave_info_t;


/**
 * grow_stackstate, push_stack_addr, and pop_stack_state manage a
 * per-thread stack that tracks return addresses.  When we enter a 
 * thin trampoline we override the original return address to point
 * into the trampoline.  After the trampoline finishes, we need to get
 * the original return back into place.  
 * These stack routines manage the location where we save the return
 * addresses.
 **/
static void grow_stackstate(unsigned long newsize)
{
   stacksave_info_t *oldstack = (stacksave_info_t *) savestate;
   unsigned long oldsize;
   stacksave_info_t *newstack;

   newstack = (stacksave_info_t *) gotcha_malloc(newsize * sizeof(stacksave_info_t));
   newstack->savesize = newsize;
   savestate = (void *) newstack;   
   if (!oldstack) {
      newstack->cursize = 1;
      return;
   }
   
   oldsize = oldstack->cursize;
   memcpy(newstack, oldstack, oldsize * sizeof(stacksave_entry_t));
   gotcha_free(oldstack);
   savestate = (void *) newstack;
}

static void push_stack_addr(void **addr, void *opaque_handle)
{
   unsigned long cur;
   stacksave_info_t *info = NULL;
   stacksave_entry_t *entries;
   if (!savestate) {
      grow_stackstate(INITIAL_SAVESTATE_SIZE);
   }
   info = (stacksave_info_t *) savestate;
   if (info->savesize == info->cursize) {
      grow_stackstate(info->savesize * 2);
      info = (stacksave_info_t *) savestate;
   }
   entries = (stacksave_entry_t *) savestate;
   assert(info->cursize >= 1);
   assert(info->cursize < info->savesize);
   cur = info->cursize;
   while ((cur != 1) && (entries[cur-1].stack_location <= (void *) addr))
      cur--;
   entries[cur].stack_location = (void *) addr;
   entries[cur].retaddr = *addr;
   entries[cur].opaque_handle = opaque_handle;
   entries[cur].unused = NULL;
   info->cursize = cur+1;
}

static void* pop_stack_state(void **addr, void **opaque_handle)
{
   stacksave_info_t *info = (stacksave_info_t *) savestate;
   stacksave_entry_t *entries = (stacksave_entry_t *) savestate;
   int cur = info->cursize-1;
   
   while ((cur != 0) && (entries[cur].stack_location != (void *) addr)) {
      cur--;
   }
   assert(cur);
   info->cursize = cur;
   *opaque_handle = entries[cur].opaque_handle;
   return entries[cur].retaddr;
}

/**
 * pre and post are infrastructure for the thin wrappers.  Rather than
 * have the assembly snippet call the user code directly it calls these
 * routines.  They setup datastructures and call onwards to the user wrappers. 
 **/
static void* pre(gotcha_binding_t *binding, void **retaddr)
{
   internal_binding_t *int_binding = *((internal_binding_t **) (binding->function_handle));
   void *opaque_handle = NULL, *wrappee;
   sigfree_pre_wrapper_t wrapper = int_binding->pre_wrapper;
   if (wrapper) {
      wrapper((gotcha_wrappee_handle_t *) int_binding, &opaque_handle);
   }  
   push_stack_addr(retaddr, opaque_handle);
   wrappee = gotcha_get_wrappee(*binding->function_handle);
   debug_printf(3, "In pre thin wrapper for %s. Next wrappee at %p, "
                "retaddr at %p\n", binding->name, wrappee, retaddr);
   return wrappee;
}

static void post(gotcha_binding_t *binding, void **retaddr)
{
   void *orig_retaddr;
   void *opaque_handle;
   internal_binding_t *int_binding = *((internal_binding_t **) (binding->function_handle));

   orig_retaddr = pop_stack_state(retaddr, &opaque_handle);
   sigfree_post_wrapper_t wrapper = int_binding->post_wrapper;
   if (wrapper) {
      wrapper((gotcha_wrappee_handle_t *) int_binding, opaque_handle);
   }
   *retaddr = orig_retaddr;
   debug_printf(3, "In post thin wrapper for %s. Set retaddr back to %p\n",
                binding->name, orig_retaddr);
}

void *create_thin_wrapper(gotcha_binding_t *binding, void *tramp_memory, int binding_num)
{
   debug_printf(2, "Creating thin wrapper around %s\n", binding->name);
   return create_trampoline(pre, post, binding, tramp_memory, binding_num);
}

extern unsigned char snippet_start, snippet_end;

void *allocate_trampoline_memory(int entries)
{
   void *mem;
   size_t size;
   size = (size_t) (&snippet_end - &snippet_start);
   size *= entries;
   mem = mmap(NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
   if (mem == MAP_FAILED)
      return NULL;
   return mem;
}

void *create_trampoline(void *prewrapper, void *postwrapper, void *param, void *trampmem, int num)
{
   unsigned char *mem;
   size_t size, i;

   size = (size_t) (&snippet_end - &snippet_start);
   
   mem = ((unsigned char *) trampmem) + (num * size);
   memcpy(mem, &snippet_start, size);


#if defined(__x86_64__)
   for (i = 0; i < size-8; i++) {
      if (*((unsigned long *) (mem + i)) == 0x1111111111111111)
         memcpy(mem + i, &prewrapper, 8);
      if (*((unsigned long *) (mem + i)) == 0x2222222222222222)
         memcpy(mem + i, &param, 8);
      if (*((unsigned long *) (mem + i)) == 0x3333333333333333)
         memcpy(mem + i, &postwrapper, 8);
   }
#elif defined(__PPC64__)
   unsigned char *paramc = (unsigned char *) &param;
   unsigned char *prec = (unsigned char *) &prewrapper;
   unsigned char *postc = (unsigned char *) &postwrapper;
   for (i = 0; i < size-2; i++) {
      if (*((unsigned short *) (mem + i)) == 0x1111)
         memcpy(mem + i, paramc + 6, 2);
      if (*((unsigned short *) (mem + i)) == 0x2222)
         memcpy(mem + i, paramc + 4, 2);
      if (*((unsigned short *) (mem + i)) == 0x3333)
         memcpy(mem + i, paramc + 2, 2);         
      if (*((unsigned short *) (mem + i)) == 0x4444)
         memcpy(mem + i, paramc, 2);

      if (*((unsigned short *) (mem + i)) == 0x5555)
         memcpy(mem + i, prec + 6, 2);
      if (*((unsigned short *) (mem + i)) == 0x6666)
         memcpy(mem + i, prec + 4, 2);
      if (*((unsigned short *) (mem + i)) == 0x7777)
         memcpy(mem + i, prec + 2, 2);         
      if (*((unsigned short *) (mem + i)) == 0x8888)
         memcpy(mem + i, prec, 2);
      
      if (*((unsigned short *) (mem + i)) == 0x9999)
         memcpy(mem + i, postc + 6, 2);
      if (*((unsigned short *) (mem + i)) == 0xaaaa)
         memcpy(mem + i, postc + 4, 2);
      if (*((unsigned short *) (mem + i)) == 0xbbbb)
         memcpy(mem + i, postc + 2, 2);         
      if (*((unsigned short *) (mem + i)) == 0xcccc)
         memcpy(mem + i, postc, 2);      
   }   
#endif
   
   debug_printf(2, "Created thin trampoline at %p to +%lu with parameter at %p\n",
                 mem, (unsigned long) size, param);
   return mem;
}
