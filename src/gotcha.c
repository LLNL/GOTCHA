#include <gotcha/gotcha.h>
#include <gotcha/gotcha_utils.h>
int gotcha_wrap(struct gotcha_binding_t* names, void** wrappers, void*** originals, int num_actions){
  int i =0;
  struct link_map * lib_iter = _r_debug.r_map;
  for(;lib_iter != 0 ; lib_iter = lib_iter->l_next){
    FOR_EACH_PLTREL(lib_iter, gotcha_wrap_impl, lib_iter, names, wrappers,num_actions);
  }
  for(i=0;i<num_actions;i++){
    *(originals[i]) = names[i].function_address_pointer;
  }
  return 0;
}

struct gotcha_binding_t* gotcha_prepare_symbols(char** symbol_names, int num_names)
{
   struct link_map *libc;
   struct gotcha_binding_t *binding, *binding_iter;
   signed long result;
   int found = 0, not_found = 0;
   binding = get_bindings(symbol_names,num_names);
   libc = _r_debug.r_map;
   INIT_DYNAMIC(libc);
   for(;libc!=0;libc=libc->l_next)
   {
      
      INIT_DYNAMIC(libc);
      gotcha_assert(gnu_hash || elf_hash);
      if(elf_hash & 0x800000000000){
        continue;
      }
      int binding_check = 0;
      for (binding_check = 0, binding_iter = binding;binding_check<num_names;binding_iter++,binding_check++) {
         if (binding_iter->function_address_pointer != 0x0)
            continue;

         result = -1;
         if (gnu_hash)
            result = lookup_gnu_hash_symbol(binding_iter->name, symtab, strtab, (struct gnu_hash_header *) gnu_hash);
         if (elf_hash && result == -1)
            result = lookup_elf_hash_symbol(binding_iter->name, symtab, strtab, (ElfW(Word) *) elf_hash);

         if (result == -1) {
            not_found++;
         }
         else if(GOTCHA_CHECK_VISIBILITY(symtab[result])) {
            binding_iter->function_address_pointer = (void *) (symtab[result].st_value + libc->l_addr);
            found++;
         }
      }
   }
   return binding;
}
