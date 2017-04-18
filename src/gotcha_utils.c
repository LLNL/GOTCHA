#include "gotcha_utils.h"
#include "libc_wrappers.h"
#include "elf_ops.h"
#include "gotcha/gotcha.h"

int debug_print_impl(ElfW(Sym) * symbol, char *name, ElfW(Addr) offset,
                     char *filter)
{
  if (gotcha_strstr(name, filter)) {
    printf("Symbol name: %s, offset %lu, size %lu\n", name, offset,
           symbol->st_size);
  }
  return 0;
}

int debug_print(struct link_map *libc, char *filter)
{
  FOR_EACH_PLTREL(libc, debug_print_impl, filter);
  return 0;
}
