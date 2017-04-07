#include <gotcha/gotcha_utils.h>
void *gotcha_malloc(size_t size) { return malloc(size); }
int gotcha_wrap_impl(ElfW(Sym) * symbol, char *name, ElfW(Addr) offset,
                     struct link_map *lmap, struct gotcha_binding_t *syms,
                     int num_actions) {
  int i = 0;
  for (i = 0; i < num_actions; i++) {
    if (gotcha_strcmp(name, syms[i].name) == 0) {
      (*((void **)(lmap->l_addr + offset))) = syms[i].wrapper_pointer;
    }
  }
  return 0;
}

int debug_print_impl(ElfW(Sym) * symbol, char *name, ElfW(Addr) offset,
                     char *filter) {
  if (gotcha_strstr(name, filter)) {
    printf("Symbol name: %s, offset %lu, size %lu\n", name, offset,
           symbol->st_size);
  }
  return 0;
}
int debug_print(struct link_map *libc, char *filter) {
  FOR_EACH_PLTREL(libc, debug_print_impl, filter);
  return 0;
}

int gotcha_prepare_symbols(struct gotcha_binding_t *bindings, int num_names) {
  struct link_map *libc;
  struct gotcha_binding_t *binding_iter;
  signed long result;
  int found = 0, not_found = 0;
  libc = _r_debug.r_map;
  int iter = 0;
  for (iter = 0; iter < num_names; iter++) {
    *(void **)(bindings[iter].function_address_pointer) = NULL;
  }
  INIT_DYNAMIC(libc);
  for (; libc != 0; libc = libc->l_next) {
    INIT_DYNAMIC(libc);
    gotcha_assert(gnu_hash || elf_hash);
    if (elf_hash & 0x800000000000 && !*libc->l_name) {
      continue;
    }
    int binding_check = 0;
    for (binding_check = 0, binding_iter = bindings; binding_check < num_names;
         binding_iter++, binding_check++) {
      if (*(void **)(binding_iter->function_address_pointer) != 0x0) {
        continue;
      }

      result = -1;
      if (gnu_hash) {
        result = lookup_gnu_hash_symbol(binding_iter->name, symtab, strtab,
                                        (struct gnu_hash_header *)gnu_hash);
      }
      if (elf_hash && result == -1) {
        result = lookup_elf_hash_symbol(binding_iter->name, symtab, strtab,
                                        (ElfW(Word) *)elf_hash);
      }

      if (result == -1) {
        not_found++;
      } else if (GOTCHA_CHECK_VISIBILITY(symtab[result])) {
        *(void **)(binding_iter->function_address_pointer) =
            (void *)(symtab[result].st_value + libc->l_addr);
        found++;
      }
    }
  }
  return 0;
}
uint32_t gnu_hash_func(const char *str) {
  uint32_t hash = 5381;
  for (; *str != '\0'; str++) {
    hash = hash * 33 + *str;
  }
  return hash;
}

signed long lookup_gnu_hash_symbol(const char *name, ElfW(Sym) * syms,
                                   char *symnames,
                                   struct gnu_hash_header *header) {
  uint32_t *buckets, *vals;
  uint32_t hash_val;
  uint32_t cur_sym, cur_sym_hashval;

  buckets = (uint32_t *)(((unsigned char *)(header + 1)) +
                         (header->maskwords * sizeof(ElfW(Addr))));
  vals = buckets + header->nbuckets;

  hash_val = gnu_hash_func(name);
  cur_sym = buckets[hash_val % header->nbuckets];
  if (cur_sym == 0) {
    return -1;
  }

  hash_val &= ~1;
  for (;;) {
    cur_sym_hashval = vals[cur_sym - header->symndx];
    if (((cur_sym_hashval & ~1) == hash_val) &&
        (gotcha_strcmp(name, symnames + syms[cur_sym].st_name) == 0)) {
      return (signed long)cur_sym;
    }
    if (cur_sym_hashval & 1) {
      return -1;
    }
    cur_sym++;
  }
}

unsigned long elf_hash(const unsigned char *name) {
  unsigned int h = 0, g;
  while (*name != '\0') {
    h = (h << 4) + *name++;
    if ((g = h & 0xf0000000)) {
      h ^= g >> 24;
    }
    h &= ~g;
  }
  return h;
}

signed long lookup_elf_hash_symbol(const char *name, ElfW(Sym) * syms,
                                   char *symnames, ElfW(Word) * header) {
  ElfW(Word) *nbucket = header + 0;
  ElfW(Word) *buckets = header + 2;
  ElfW(Word) *chains = buckets + *nbucket;

  unsigned int x = elf_hash((const unsigned char *)name);
  signed long y = (signed long)buckets[x % *nbucket];
  while (y != STN_UNDEF) {
    if (gotcha_strcmp(name, symnames + syms[y].st_name) == 0) {
      return y;
    }
    y = chains[y];
  }

  return -1;
}

struct gotcha_binding_t *get_bindings(char **symbol_names, int num_names) {
  struct gotcha_binding_t *binding_list =
      (struct gotcha_binding_t *)gotcha_malloc(sizeof(struct gotcha_binding_t) *
                                               num_names);
  int i = 0;
  for (i = 0; i < num_names; i++) {
    binding_list[i].function_address_pointer = 0x0;
    binding_list[i].name = symbol_names[i];
  }
  return binding_list;
}
void gotcha_free(void **free_me) { free(free_me); }
void gotcha_memcpy(void *dest, void *src, size_t size) {
  memcpy(dest, src, size);
}
int gotcha_strncmp(const char *in_one, const char *in_two, int max_length) {
  int i = 0;
  for (; i < max_length; i++) {
    if (in_one[i] == '\0') {
      return (in_two[i] == '\0') ? 0 : 1;
    }
    if (in_one[i] != in_two[i]) {
      return in_one[i] - in_two[i];
    }
  }
  return 0;
}
char *gotcha_strstr(char *searchIn, char *searchFor) {
  return strstr(searchIn, searchFor);
}
void gotcha_assert(int assertion) { assert(assertion); }

int gotcha_strcmp(const char *in_one, const char *in_two) {
  int i = 0;
  for (;; i++) {
    if (in_one[i] == '\0') {
      return (in_two[i] == '\0') ? 0 : 1;
    }
    if (in_one[i] != in_two[i]) {
      return in_one[i] - in_two[i];
    }
  }
  return 0;
}
