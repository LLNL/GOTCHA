#include <gotcha/gotcha_utils.h>
void* gotcha_malloc(size_t size){
  return malloc(size);
}
int gotcha_wrap_impl(ElfW(Sym)* symbol, char* name, ElfW(Addr) offset, struct link_map* lmap, struct gotcha_binding_t* syms, void** wrappers, int num_actions){
  int i=0;
  for(i =0 ; i < num_actions;i++){
    if(gotcha_strcmp(name,syms[i].name)==0){
      (*((void**)(lmap->l_addr + offset))) = wrappers[i];
    }
  }
  return 0;
}
uint32_t gnu_hash_func(const char *str) {
   uint32_t hash = 5381;
   for (; *str != '\0'; str++)
      hash = hash * 33 + *str;
   return hash;
}

signed long lookup_gnu_hash_symbol(const char *name, ElfW(Sym) *syms, char *symnames, struct gnu_hash_header *header)
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

unsigned long elf_hash(const unsigned char *name)
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

signed long lookup_elf_hash_symbol(const char *name, ElfW(Sym) *syms, char *symnames, ElfW(Word) *header)
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
void gotcha_free(void** free_me){
  free(free_me);
}
void gotcha_memcpy(void* dest, void* src, size_t size){
  memcpy(dest, src, size);
}
int gotcha_strncmp(const char* in_one, const char* in_two, int max_length){
  int i=0;
  for(;i<max_length;i++){
     if(in_one[i]=='\0'){
       return (in_two[i]=='\0') ? 0 : 1;
     }
     if(in_one[i] != in_two[i]){
       return in_one[i] - in_two[i];
     } 
  }
  return 0;
}
char* gotcha_strstr(char* searchIn, char* searchFor){
  return strstr(searchIn, searchFor);
}
void gotcha_assert(int assertion){
  assert(assertion);
}

int gotcha_strcmp(const char* in_one, const char* in_two){
  int i=0;
  for(;;i++){
     if(in_one[i]=='\0'){
       return (in_two[i]=='\0') ? 0 : 1;
     }
     if(in_one[i] != in_two[i]){
       return in_one[i] - in_two[i];
     } 
  }
  return 0;
}
