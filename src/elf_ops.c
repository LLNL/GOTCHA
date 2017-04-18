#include "elf_ops.h"
#include <elf.h>

struct gnu_hash_header {
   uint32_t nbuckets;   //!< The number of buckets to hash symbols into
   uint32_t symndx;     //!< Index of the first symbol accessible via hashtable in the symbol table
   uint32_t maskwords;  //!< Number of words in the hash table's bloom filter
   uint32_t shift2;     //!< The bloom filter's shift count
};

static uint32_t gnu_hash_func(const char *str) {
  uint32_t hash = 5381;
  for (; *str != '\0'; str++) {
    hash = hash * 33 + *str;
  }
  return hash;
}

signed long lookup_gnu_hash_symbol(const char *name, ElfW(Sym) * syms,
                                   char *symnames,
                                   void *sheader) {
  uint32_t *buckets, *vals;
  uint32_t hash_val;
  uint32_t cur_sym, cur_sym_hashval;
  struct gnu_hash_header *header = (struct gnu_hash_header *) (sheader);

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

static unsigned long elf_hash(const unsigned char *name) {
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
