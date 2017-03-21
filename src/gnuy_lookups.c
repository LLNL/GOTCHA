#include "gnuy_lookups.h"
int gotcha_wrap_impl(ElfW(Sym)* symbol, char* name, ElfW(Addr) offset, struct link_map* lmap, struct gotcha_binding_t* syms, void** wrappers, int num_actions){
  int i=0;
  for(i =0 ; i < num_actions;i++){
    if(gotcha_strcmp(name,syms[i].name)==0){
      (*((void**)(lmap->l_addr + offset))) = wrappers[i];
    }
  }
  return 0;
}
int gotcha_wrap(struct gotcha_binding_t* names, void** wrappers, int num_actions){
  int i =0;
  struct link_map * lib_iter = _r_debug.r_map;
  for(;lib_iter != 0 ; lib_iter = lib_iter->l_next){
    FOR_EACH_PLTREL(lib_iter, gotcha_wrap_impl, lib_iter, names, wrappers, num_actions);
  }
  return 0;
}

uint32_t gnu_hash_func(const char *str) {
   uint32_t hash = 5381;
   for (; *str != '\0'; str++)
      hash = hash * 33 + *str;
   return hash;
}

static signed long lookup_gnu_hash_symbol(const char *name, ElfW(Sym) *syms, char *symnames, struct gnu_hash_header *header)
{
   uint32_t *buckets, *vals;
   uint32_t hash_val;
   uint32_t cur_sym, cur_sym_hashval;

   buckets = (uint32_t *) (((unsigned char *) (header+1)) + (header->maskwords * sizeof(ElfW(Addr))));
   vals = buckets + header->nbuckets;
   
   hash_val = gnu_hash_func(name);
   cur_sym = buckets[hash_val % header->nbuckets];
   if (cur_sym == 0)
      return -1;

   hash_val &= ~1;
   for (;;) {
      cur_sym_hashval = vals[cur_sym - header->symndx];
      if (((cur_sym_hashval & ~1) == hash_val) && 
          (gotcha_strcmp(name, symnames + syms[cur_sym].st_name) == 0))
         return (signed long) cur_sym;
      if (cur_sym_hashval & 1)
         return -1;
      cur_sym++;
   }
}

static unsigned long elf_hash(const unsigned char *name)
{
   unsigned long h = 0, g;
   while (*name) {
      h = (h << 4) + *name++;
      if ((g = h & 0xf0000000))
         h ^= h >> 24;
      h &= ~g;
   }
   return h;
}

static signed long lookup_elf_hash_symbol(const char *name, ElfW(Sym) *syms, char *symnames, ElfW(Word) *header)
{
   ElfW(Word) *nbucket = header + 0;
   /*ElfW(Word) *nchain = header + 1;*/
   ElfW(Word) *buckets = header + 2;
   ElfW(Word) *chains = buckets + *nbucket;
   
   unsigned int hash_idx = elf_hash((const unsigned char *) name) % *nbucket;
   signed long idx = (signed long) buckets[hash_idx];
   while (idx != STN_UNDEF) {
      if (gotcha_strcmp(name, symnames + syms[idx].st_name) == 0)
         return idx;
      idx = chains[idx];
   }
   
   return -1;
}


struct gotcha_binding_t* get_bindings(char** symbol_names, int num_names){
  struct gotcha_binding_t* binding_list = (struct gotcha_binding_t*)gotcha_malloc(sizeof(struct gotcha_binding_t)*num_names);  
  int i = 0;
  for(i = 0; i < num_names;i++){
    binding_list[i].function_address_pointer = 0x0;
    binding_list[i].name = symbol_names[i];
  }
  return binding_list;
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
            result = lookup_gnu_hash_symbol(binding->name, symtab, strtab, (struct gnu_hash_header *) gnu_hash);
         if (elf_hash && result == -1)
            result = lookup_elf_hash_symbol(binding->name, symtab, strtab, (ElfW(Word) *) elf_hash);

         if (result == -1) {
            printf("Warning, Could not bind symbol %s in libc\n", binding->name);
            not_found++;
         }
         else if(CHECK_VISIBILITY(symtab[result])) {
            binding->function_address_pointer = (void *) (symtab[result].st_value + libc->l_addr);
            found++;
         }
      }
   }
   return binding;
}
