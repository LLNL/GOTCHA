#ifndef GNUY_LOOKUPS_H
#define GNUY_LOOKUPS_H
#include <stdint.h>
#include <elf.h>
#include <link.h>
#include "gotcha_utils.h"
//#define CHECK_VISIBILITY(sym)((sym.st_info & 0x3) < 2)
#define CHECK_VISIBILITY(sym)((sym.st_size>0))
struct gnu_hash_header {
   uint32_t nbuckets;
   uint32_t symndx;
   uint32_t maskwords;
   uint32_t shift2;
};
struct gotcha_binding_t {
  char* name;
  void* function_address_pointer;
  void* wrapper_pointer;
};
uint32_t gnu_hash_func(const char *str);
static signed long lookup_gnu_hash_symbol(const char *name, ElfW(Sym) *syms, char *symnames, struct gnu_hash_header *header);
static unsigned long elf_hash(const unsigned char *name);
static signed long lookup_elf_hash_symbol(const char *name, ElfW(Sym) *syms, char *symnames, ElfW(Word) *header);
struct gotcha_binding_t* get_bindings(char** symbol_names, int num_names);
struct gotcha_binding_t* gotcha_prepare_symbols(char** symbol_names, int num_names);
int gotcha_wrap_impl(ElfW(Sym)* symbol, char* name, ElfW(Addr) offset, struct link_map* lmap, struct gotcha_binding_t* syms, void** wrappers, int num_actions);
int gotcha_wrap(struct gotcha_binding_t* names, void** wrappers, int num_actions);
#endif
