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

static char* vdso_aliases[];

static ElfW(auxv_t) *get_auxv_contents();
static struct link_map *get_vdso_from_auxv();
static struct link_map *get_vdso_from_aliases();
static struct link_map *get_vdso_from_maps();
int is_vdso(struct link_map *map);
#endif
