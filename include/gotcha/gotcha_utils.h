#ifndef GOTCHA_UTILS_H
#define GOTCHA_UTILS_H
#include <sys/mman.h>
#include "gotcha_types.h"
// TODO: remove these includes
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
// END TODO
#include <elf.h>
#include <link.h>
#define GOTCHA_FALSE 0
#if __WORDSIZE == 64
#define R_SYM(X) ELF64_R_SYM(X)
#else
#define R_SYM(X) ELF32_R_SYM(X)
#endif
#define BOUNDARY_BEFORE(ptr, size) \
  (void*)(((ElfW(Addr))ptr) & (-size))
#ifndef NULL
#define NULL 0
#endif
#define GOTCHA_CHECK_VISIBILITY(sym)((sym.st_size>0))
struct gnu_hash_header {
   uint32_t nbuckets;
   uint32_t symndx;
   uint32_t maskwords;
   uint32_t shift2;
};
uint32_t gnu_hash_func(const char *str);
signed long lookup_gnu_hash_symbol(const char *name, ElfW(Sym) *syms, char *symnames, struct gnu_hash_header *header);
unsigned long elf_hash(const unsigned char *name);
signed long lookup_elf_hash_symbol(const char *name, ElfW(Sym) *syms, char *symnames, ElfW(Word) *header);
struct gotcha_binding_t* get_bindings(char** symbol_names, int num_names);
int gotcha_wrap_impl(ElfW(Sym)* symbol, char* name, ElfW(Addr) offset, struct link_map* lmap, struct gotcha_binding_t* syms, void** wrappers, int num_actions);
#define INIT_DYNAMIC(lmap)                                      \
   ElfW(Dyn) *dynsec = NULL, *dentry = NULL;                    \
   ElfW(Rela) *rela = NULL;                                     \
   ElfW(Rel) *rel = NULL;                                       \
   ElfW(Addr) jmprel = 0;                                       \
   ElfW(Sym) *symtab = NULL;                                    \
   ElfW(Addr) gnu_hash = 0x0, elf_hash = 0x0;                   \
   ElfW(Addr) got = 0x0;                                        \
   char *strtab = NULL;                                         \
   unsigned int rel_size = 0, rel_count, is_rela = 0, i;        \
   dynsec = lmap->l_ld;                                         \
   if (!dynsec)                                                 \
      return -1;                                                \
   for (dentry = dynsec; dentry->d_tag != DT_NULL; dentry++) {  \
      switch (dentry->d_tag) {                                  \
         case DT_PLTRELSZ: {                                    \
            rel_size = (unsigned int) dentry->d_un.d_val;       \
            break;                                              \
         }                                                      \
         case DT_PLTGOT: {                                      \
            got = dentry->d_un.d_ptr;                           \
            break;                                              \
         }                                                      \
         case DT_HASH: {                                        \
            elf_hash = dentry->d_un.d_val;                      \
            break;                                              \
         }                                                      \
         case DT_STRTAB: {                                      \
            strtab = (char *) dentry->d_un.d_ptr;               \
            break;                                              \
         }                                                      \
         case DT_SYMTAB: {                                      \
            symtab = (ElfW(Sym) *) dentry->d_un.d_ptr;          \
            break;                                              \
         }                                                      \
         case DT_PLTREL: {                                      \
            is_rela = (dentry->d_un.d_val == DT_RELA);          \
            break;                                              \
         }                                                      \
         case DT_JMPREL: {                                      \
            jmprel = dentry->d_un.d_val;                        \
            break;                                              \
         }                                                      \
         case DT_GNU_HASH: {                                    \
            gnu_hash = dentry->d_un.d_val;                      \
            break;                                              \
         }                                                      \
      }                                                         \
   }                                                            \
   (void) rela;                                                 \
   (void) rel;                                                  \
   (void) jmprel;                                               \
   (void) symtab;                                               \
   (void) gnu_hash;                                             \
   (void) elf_hash;                                             \
   (void) got;                                                  \
   (void) strtab;                                               \
   (void) rel_size;                                             \
   (void) rel_count;                                            \
   (void) is_rela;                                              \
   (void) i;

#define FOR_EACH_PLTREL_INT(relptr, op, ...)                        \
   rel_count = rel_size / sizeof(*relptr);                     \
   for (i = 0; i < rel_count; i++) {                           \
      ElfW(Addr) offset = relptr[i].r_offset;                  \
      unsigned long symidx = R_SYM(relptr[i].r_info);          \
      ElfW(Sym) *sym = symtab + symidx;                        \
      char *symname = strtab + sym->st_name;                   \
      op(sym, symname, offset, ## __VA_ARGS__);                                \
   }

#define FOR_EACH_PLTREL(lmap, op, ...) {                            \
      INIT_DYNAMIC(lmap)                                       \
      ElfW(Addr) offset = lmap->l_addr;                        \
      if (is_rela) {                                           \
         rela = (ElfW(Rela) *) jmprel;                         \
         FOR_EACH_PLTREL_INT(rela, op, ## __VA_ARGS__);                        \
      }                                                        \
      else {                                                   \
         rel = (ElfW(Rel) *) jmprel;                           \
         FOR_EACH_PLTREL_INT(rel, op, ## __VA_ARGS__);                         \
      }                                                        \
   }

#define FOR_EACH_SYM(lmap, op, ...) {                               \
      INIT_DYNAMIC(lmap)                                       \
      ElfW(Addr) offset = lmap->l_addr;                        \
      ElfW(Sym*) sym_iter = symtab;                            \
      if(((unsigned long)sym_iter)<0xffffffffff700158){        \
         while(sym_iter->st_size < 10000) {                    \
            char *symname = strtab + sym_iter->st_name;        \
            op(sym_iter,symname, ## __VA_ARGS__);                              \
            sym_iter++;                                        \
         }                                                     \
      }                                                        \
   }
void* gotcha_malloc(size_t size);
void gotcha_free(void** free_me);
void gotcha_memcpy(void* dest, void* src, size_t size);
int gotcha_strncmp(const char* in_one, const char* in_two, int max_length);
char* gotcha_strstr(char* searchIn, char* searchFor);
void gotcha_assert(int assertion);
int gotcha_strcmp(const char* in_one, const char* in_two);
#endif
