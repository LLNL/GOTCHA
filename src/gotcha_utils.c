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
