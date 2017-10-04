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

#include <stdio.h>
#include <string.h>
#include <gotcha/gotcha_cpp.h>
#include <iostream>

int main()
{
   int result;
   void* last_alloc = NULL;
   int* test_allocation; 
   int last_allocation_size = 0;
   using free_arg_type = void*;
   free_arg_type last_freed_address = NULL;
   gotcha_quick_wrap_handle(malloc_handle_1,malloc,[&](original_malloc orig, unsigned long size){
       std::cout << "Allocation of size " << size << std::endl;
       void* ret = orig(size);
       last_allocation_size = size;
       return ret;
       //return (void*)NULL;
   });    
   {
   gotcha_quick_wrap_handle(malloc_handle_2,malloc,[&](original_malloc orig, unsigned long size){
       std::cout << "Outer malloc wrapper called on size " << size << std::endl;
       void* ret = orig(size);
       return ret;
       //return (void*)NULL;
   });    
   gotcha_quick_wrap_handle(free_handle,free,[&](original_free orig, free_arg_type to_free){
       last_freed_address = to_free;
       std::cout << "Freeing " << (void*)to_free << std::endl;
       orig(to_free);
   });
   test_allocation = (int*) malloc(sizeof(int) * 1000);
   //malloc_handle_1.unwrap(); 
   }

   free(test_allocation);
   if(last_allocation_size != sizeof(int)*1000){
     std::cout << "Size of last allocation not correctly captured;";
     return -1;
   }
   //free_handle.unwrap();
   test_allocation = (int*) malloc(sizeof(int) * 1000);
   free(test_allocation);

   std::cout << "Test success\n";
   return 0;
}
