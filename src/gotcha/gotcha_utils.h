/*
This file is part of GOTCHA.  For copyright information see the COPYRIGHT
file in the top level directory, or at
https://github.com/LLNL/gotcha/blob/master/COPYRIGHT
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (as published by the Free Software
Foundation) version 2.1 dated February 1999.  This program is distributed in the
hope that it will be useful, but WITHOUT ANY WARRANTY; without even the IMPLIED
WARRANTY OF MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms
and conditions of the GNU Lesser General Public License for more details.  You should
have received a copy of the GNU Lesser General Public License along with this
program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA
*/
/*!
 ******************************************************************************
 *
 * \file gotcha_utils.h
 *
 * \brief   Header file containing the internal gotcha mechanisms
 *          for manipulating the running process to redirect calls
 *
 ******************************************************************************
 */
#ifndef GOTCHA_UTILS_H
#define GOTCHA_UTILS_H
#include <sys/mman.h>
#include "gotcha/gotcha_types.h"
// TODO: remove these includes
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
// END TODO
#include <elf.h>
#include <link.h>
/*!
 ******************************************************************************
 * \def GOTCHA_FALSE
 * \brief This is GOTCHA's internal false, to avoid needing additional includes
 ******************************************************************************
 */
#define GOTCHA_FALSE 0


/*!
 ******************************************************************************
 * \def R_SYM(X)
 * \brief Returns an ELF symbol which is correct for the current architecture
 * \param X The value you wish to cast to Symbol type
 ******************************************************************************
 */
#if __WORDSIZE == 64
#define R_SYM(X) ELF64_R_SYM(X)
#else
#define R_SYM(X) ELF32_R_SYM(X)
#endif

/*!
 ******************************************************************************
 * \def BOUNDARY_BEFORE(ptr,pagesize)
 * \brief Returns the address on page boundary before the given pointer
 * \param ptr The address you wish to get the page boundary before
 * \param pagesize The page size you wish to align to
 ******************************************************************************
 */
#define BOUNDARY_BEFORE(ptr, pagesize) \
  (void*)(((ElfW(Addr))ptr) & (-pagesize))

/*!
 ******************************************************************************
 * \def GOTCHA_CHECK_VISIBILITY(sym)
 * \brief Checks whether a given symbol is associated with a real function
 * \param sym The symbol you wish to check
 ******************************************************************************
 */
#define GOTCHA_CHECK_VISIBILITY(sym)((sym.st_size>0))

// TODO: Check these docs to make sure
/*! A struct containing the parameters the GNU Hash function needs to operate */
struct gnu_hash_header {
   uint32_t nbuckets;   //!< The number of buckets to hash symbols into
   uint32_t symndx;     //!< Index of the first symbol accessible via hashtable in the symbol table
   uint32_t maskwords;  //!< Number of words in the hash table's bloom filter
   uint32_t shift2;     //!< The bloom filter's shift count
};

/*!
 ******************************************************************************
 *
 * \fn void gotcha_prepare_symbols(struct gotcha_binding_t* bindings, int num_names)
 *
 * \brief Given a list of function names, create the gotcha structure used to
 *				wrap functions
 *
 * \param bindings     The GOTCHA wrap actions
 * \param num_names 	 The number of symbol names in symbol_names
 *
 ******************************************************************************
 */
int gotcha_prepare_symbols(struct gotcha_binding_t* bindings, int num_names);


/*!
 ******************************************************************************
 *
 * \fn uint32_t gnu_hash_func(const char* str);
 *
 * \brief Given a function name, computes its GNU Hash value 
 *
 * \param str The function name to be hashed
 *
 ******************************************************************************
 */
uint32_t gnu_hash_func(const char *str);

//TODO: check this, seems iffy
/*!
 ******************************************************************************
 *
 * \fn signed long lookup_gnu_hash_symbol(const char *name, 
                                          ElfW(Sym) *syms,
                                          char *symnames, 
                                          struct gnu_hash_header *header);
 *
 * \brief Looks up the index of a symbol in a symbol table given a symbol name 
 *
 * \param name     The name of the function to be looked up
 * \param syms     The pointer to the symbol table
 * \param symnames A pointer into the string table
 * \param header   The parameters the underlying GNU Hash function will use
 *
 ******************************************************************************
 */
signed long lookup_gnu_hash_symbol(const char *name, ElfW(Sym) *syms, char *symnames, struct gnu_hash_header *header);

/*!
 ******************************************************************************
 *
 * \fn uint32_t elf_hash(const unsigned char *name);
 *
 * \brief Given a function name, computes its ELF Hash value 
 *
 * \param name The function name to be hashed
 *
 ******************************************************************************
 */
unsigned long elf_hash(const unsigned char *name);

//TODO: check this, seems iffy
/*!
 ******************************************************************************
 *
 * \fn signed long lookup_elf_hash_symbol(const char *name, 
                                          ElfW(Sym) *syms,
                                          char *symnames, 
                                          ElfW(Word) *header);
 *
 * \brief Looks up the index of a symbol in a symbol table given a symbol name 
 *
 * \param name     The name of the function to be looked up
 * \param syms     The pointer to the symbol table
 * \param symnames A pointer into the string table
 * \param header   The parameters the underlying ELF Hash function will use
 *
 ******************************************************************************
 */
signed long lookup_elf_hash_symbol(const char *name, ElfW(Sym) *syms, char *symnames, ElfW(Word) *header);

/*!
 ******************************************************************************
 *
 * \fn struct gotcha_binding_t* get_bindings(char** symbol_names, 
 *                                           int num_names);
 *
 * \brief given a set of symbol names, create the gotcha struct to handle
 *        wrapping them
 *
 * \param symbol_names the names of the symbols to be wrapped
 * \param num_names    the number of names in the set
 *
 ******************************************************************************
 */
struct gotcha_binding_t* get_bindings(char** symbol_names, int num_names);

/*!
 ******************************************************************************
 *
 * \fn int gotcha_wrap_impl(ElfW(Sym)* symbol, 
 *                          char* name, 
 *                          ElfW(Addr) offset, 
 *                          struct link_map* lmap, 
 *                          struct gotcha_binding_t* syms, 
 *                          void** wrappers, 
 *                          int num_actions);
 *
 * \brief This function rewrites the GOT table to
 *        point to the desired target
 *
 * \param symbol       The symbol being wrapped
 * \param name         The name of the symbol
 * \param offset       How far from the library base address the GOT entry is
 * \param lmap         The link map entry associated with this symbol
 * \param syms         The GOTCHA entries to be processed
 * \param wrappers     The wrapper functions being redirected to
 * \param num_actions  The number of function names being overwritten
 *
 ******************************************************************************
 */
int gotcha_wrap_impl(ElfW(Sym)* symbol, char* name, ElfW(Addr) offset, struct link_map* lmap, struct gotcha_binding_t* syms, int num_actions);

/*!
 ******************************************************************************
 *
 * \def INIT_DYNAMIC(lmap)
 *
 * \brief This macro initializes a set of variables from an link.h
 *        link_map entry
 *
 * \param lmap         The link map entry associated with this symbol
 * \param[out] dynsec  The dynamic section associated with the link_map
 * \param[out] rela
 * \param[out] rel
 * \param[out] jmprel
 * \param[out] symtab
 * \param[out] gnu_hash
 * \param[out] got
 * \param[out] strtab
 *
 ******************************************************************************
 */
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

/*!
 ******************************************************************************
 *
 * \def FOR_EACH_PLTREL_INT(relptr, op, ...)
 *
 * \brief This macro calls an operation on each relocation in a dynamic section.
 *        It should be called only by FOR_EACH_PLTREL, users should not call it
 *
 * \param relptr       The pointer to the first relocation in the section
 * \param op           The operation to be performed on each relocation
 * \param ...          Any additional arguments you wish to pass to op
 *
 ******************************************************************************
 */
#define FOR_EACH_PLTREL_INT(relptr, op, ...)                        \
   rel_count = rel_size / sizeof(*relptr);                     \
   for (i = 0; i < rel_count; i++) {                           \
      ElfW(Addr) offset = relptr[i].r_offset;                  \
      unsigned long symidx = R_SYM(relptr[i].r_info);          \
      ElfW(Sym) *sym = symtab + symidx;                        \
      char *symname = strtab + sym->st_name;                   \
      op(sym, symname, offset, ## __VA_ARGS__);                                \
   }

/*!
 ******************************************************************************
 *
 * \def FOR_EACH_PLTREL(lmap, op, ...)
 *
 * \brief This macro calls an operation on each relocation in the dynamic section
 *        associated with a link map entry
 *
 * \param lmap         The link map whose relocations you want processed
 * \param op           The operation to be performed on each relocation
 * \param ...          Any additional arguments you wish to pass to op
 *
 ******************************************************************************
 */
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


/*!
 ******************************************************************************
 *
 * \fn void* gotcha_malloc(size_t size);
 *
 * \brief A gotcha wrapper around malloc to avoid libc dependencies
 *
 * \param size The number of bytes to allocate
 *
 ******************************************************************************
 */
void* gotcha_malloc(size_t size);

/*!
 ******************************************************************************
 *
 * \fn void* gotcha_realloc(void* buffer, size_t size);
 *
 * \brief A gotcha wrapper around malloc to avoid libc dependencies
 *
 * \param buffer The buffer to 
 * \param size The number of bytes to allocate
 *
 ******************************************************************************
 */
void *gotcha_realloc(void* buffer, size_t size);
/*!
 ******************************************************************************
 *
 * \fn void gotcha_free(void* free_me);
 *
 * \brief A gotcha wrapper around free to avoid libc dependencies
 *
 * \param free_me A pointer to the pointer we wish to free
 *
 ******************************************************************************
 */
void gotcha_free(void* free_me);

/*!
 ******************************************************************************
 *
 * \fn void gotcha_memcpy(void* dest, void* src, size_t size);
 *
 * \brief A gotcha wrapper around memcpy to avoid libc dependencies
 *
 * \param dest Where memory should be copied to
 * \param src  Where memory should be copied from
 * \param size How many bytes to copy
 *
 ******************************************************************************
 */
void gotcha_memcpy(void* dest, void* src, size_t size);
/*!
 ******************************************************************************
 *
 * \fn int gotcha_strncmp(const char* in_one, const char* in_two, int max_length);
 *
 * \brief A gotcha wrapper around strmcmp to avoid libc dependencies
 *
 * \param in_one The first string to be checked
 * \param in_two The second string to be checked
 * \param max_length The maximum number of characters to compare
 *
 ******************************************************************************
 */
int gotcha_strncmp(const char* in_one, const char* in_two, int max_length);
/*!
 ******************************************************************************
 *
 * \fn char* gotcha_strstr(char* searchIn, char* searchFor);
 *
 * \brief A gotcha wrapper around strstr to avoid libc dependencies
 *
 * \param searchIn  A string to search in
 * \param searchFor A substring to search for
 *
 ******************************************************************************
 */
char* gotcha_strstr(char* searchIn, char* searchFor);

/*!
 ******************************************************************************
 *
 * \fn void gotcha_assert(int assertion);
 *
 * \brief A gotcha wrapper around assert to avoid libc dependencies
 *
 * \param assertion Something the programmer says should not equal false
 *
 ******************************************************************************
 */
void gotcha_assert(int assertion);
/*!
 ******************************************************************************
 *
 * \fn int gotcha_strcmp(const char* in_one, const char* in_two);
 *
 * \brief A gotcha wrapper around strcmp to avoid libc dependencies
 *
 * \param in_one The first string to compare
 * \param in_two The second string to compare
 *
 ******************************************************************************
 */
int gotcha_strcmp(const char* in_one, const char* in_two);
#endif
