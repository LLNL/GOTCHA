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

#ifndef GOTCHA_AUXV_H
#define GOTCHA_AUXV_H

#include <elf.h>
#include <link.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "testing_infrastructure.h"

int is_vdso(struct link_map *map);
int get_auxv_pagesize();
#ifdef GOTCHA_ENABLE_COVERAGE_TESTING
TEST_ONLY_VISIBILITY struct link_map *get_vdso_from_auxv();
TEST_ONLY_VISIBILITY int parse_auxv_contents();
TEST_ONLY_VISIBILITY struct link_map *get_vdso_from_aliases();
TEST_ONLY_VISIBILITY int read_line(char *line, int size, int fd);
TEST_ONLY_VISIBILITY int read_hex(char *str, unsigned long *val);
TEST_ONLY_VISIBILITY int read_word(char *str, char *word, int word_size);
TEST_ONLY_VISIBILITY struct link_map *get_vdso_from_maps();
#endif


#endif
