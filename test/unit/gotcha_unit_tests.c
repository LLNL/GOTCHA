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

#include <gotcha/gotcha.h>
#include <check.h>
#include "testing_lib.h"
//#include <gotcha_utils.h>
//#include "elf_ops.h"
//#include "tool.h"

struct tool* new_tool;
void setup_infrastructure(){
  new_tool = create_tool("internal_test_tool");
}
void teardown_infrastructure(){
  free(new_tool);
}

int dummy_main(int argc, char* argv[]){return 4;}
START_TEST(symbol_prep_test)
{
  int(*my_main)(int argc, char* argv[]) = 0;
  struct gotcha_binding_t bindings[] = {
    { "main", &dummy_main, &my_main  }
  };
  struct binding_t* internal_bindings = add_binding_to_tool(new_tool, bindings, 1);
  gotcha_prepare_symbols(internal_bindings,1);
  ck_assert_msg((my_main!=0), "gotcha_prepare_symbols was not capable of finding function main");
}
END_TEST

int(*orig_func)();
int wrap_sample_func(){
  return orig_func()+5;
}

START_TEST(symbol_wrap_test){
  struct gotcha_binding_t bindings[] = {
    { "simpleFunc", &wrap_sample_func, &orig_func }
  };
  gotcha_wrap(bindings,1,"internal_test_tool");
  int x = simpleFunc(); 
  ck_assert_msg((x!=TESTING_LIB_RET_VAL),"gotcha_wrap did not redirect a call to the wrapper function");
}
END_TEST

Suite* gotcha_core_suite(){
  Suite* s = suite_create("Gotcha Core");
  TCase* core_case = tcase_create("Wrapping");
  tcase_add_checked_fixture(core_case, setup_infrastructure, teardown_infrastructure);
  tcase_add_test(core_case, symbol_prep_test);
  tcase_add_test(core_case, symbol_wrap_test);
  suite_add_tcase(s, core_case);
  return s;
}

int main(){
  int num_fails;
  Suite* run_suite = gotcha_core_suite();
  SRunner* runner = srunner_create(run_suite);
  srunner_run_all(runner, CK_NORMAL);
  num_fails = srunner_ntests_failed(runner);
  srunner_free(runner);
  return num_fails;
}
//int gotcha_wrap_impl(ElfW(Sym) * symbol, char *name, ElfW(Addr) offset,
//                     struct link_map *lmap, binding_t *bindings,
//                     int num_actions) {
//  int result;
//  binding_ref_t *ref;
//  struct gotcha_binding_t *user_binding;
//  result = lookup_hashtable(&bindings->binding_hash, name, (void **) &ref);
//  if (result != 0)
//     return 0;
//
//  user_binding = ref->binding->user_binding + ref->index;
//  (*((void **)(lmap->l_addr + offset))) = user_binding->wrapper_pointer;
//
//  debug_printf(3, "Remapped call to %s at 0x%lx in %s to wrapper at 0x%p\n",
//               name, (lmap->l_addr + offset), LIB_NAME(lmap), 
//               user_binding->wrapper_pointer);
//
//  return 0;
//}
//size_t get_page_size(void)
//{
//  size_t n;
//  char *p;
//  int u;
//  for (n = 1; n; n *= 2) {
//    p = mmap(0, n * 2, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
//    if (p == MAP_FAILED) return -1;
//    u = munmap(p + n, n);
//    munmap(p, n * 2);
//    if (!u) return n;
//  }
//  return -1;
//}
//#define MAX(a,b) (a>b?a:b)
//enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* user_bindings, int num_actions, char* tool_name){
//  int page_size = get_page_size();
//  int i;
//  enum gotcha_error_t ret_code;
//  struct link_map *lib_iter;
//  tool_t *tool;
//  for(lib_iter=_r_debug.r_map;lib_iter;lib_iter=lib_iter->l_next){
//    INIT_DYNAMIC(lib_iter);
//    if(got){
//      int res = mprotect(BOUNDARY_BEFORE(got,page_size),MAX(rel_count*rel_size,page_size),PROT_WRITE|PROT_READ|PROT_EXEC);
//      if(!res){
//        debug_printf(1, "GOTCHA attempted to mark the GOT table as writable and was unable to do so, calls to wrapped functions will likely fail\n");
//      }
//    }
//  } 
//  debug_init();
//  debug_printf(1, "User called gotcha_wrap for tool %s with %d bindings\n",
//               tool_name, num_actions);
//
//  if (debug_level >= 3) {
//    for (i = 0; i < num_actions; i++) {
//       debug_bare_printf(3, "\t%d: %s will map to %p\n", i, user_bindings[i].name,
//                         user_bindings[i].wrapper_pointer);
//    }
//  }
//
//  if (!tool_name)
//     tool_name = "[UNSPECIFIED]";
//  tool = get_tool(tool_name);
//  if (!tool)
//     tool = create_tool(tool_name);
//  if (!tool) {
//     error_printf("Failed to create tool %s\n", tool_name);
//     return GOTCHA_INTERNAL;
//  }
//
//  binding_t *bindings = add_binding_to_tool(tool, user_bindings, num_actions);
//  if (!bindings) {
//     error_printf("Failed to create bindings for tool %s\n", tool_name);
//     return GOTCHA_INTERNAL;
//  }
//
//  gotcha_prepare_symbols(bindings, num_actions);
//
//  for (lib_iter = _r_debug.r_map; lib_iter != 0; lib_iter = lib_iter->l_next) {
//    debug_printf(2, "Looking for wrapped callsites in %s\n", LIB_NAME(lib_iter));
//    FOR_EACH_PLTREL(lib_iter, gotcha_wrap_impl, lib_iter, bindings, num_actions);
//  }
//
//  ret_code = GOTCHA_SUCCESS;
//  for(i = 0; i<num_actions;i++){
//    if(user_bindings[i].function_address_pointer==0){
//       debug_printf(1, "Returning GOTCHA_FUNCTION_NOT_FOUND from gotcha_wrap" 
//                    "because of entry %d\n", i);
//      ret_code = GOTCHA_FUNCTION_NOT_FOUND;
//    }
//  }
//
//  debug_printf(1, "Returning code %d from gotcha_wrap\n", ret_code);
//  return ret_code;
//}
