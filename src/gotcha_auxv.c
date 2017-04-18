#include "gotcha/gotcha_auxv.h"
#include "gotcha/gotcha_utils.h"

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

static ElfW(auxv_t) *get_auxv_contents()
{
   char name[] = "/proc/self/auxv";
   int fd;
   char *buffer = NULL;
   ssize_t buffer_size = 0x1000, offset = 0, result;
   
   fd = open(name, O_RDONLY);
   if (fd == -1) {
      return NULL;
   }

   buffer = gotcha_malloc(buffer_size);
   for (;;) {
      result = read(fd, buffer+offset, buffer_size-offset);
      if (result == -1) {
         if (errno == EINTR)
            continue;
         close(fd);
         gotcha_free(buffer);
         return NULL;
      }
      if (result == 0) {
         close(fd);
         break;
      }
      offset += result;
      if (offset == buffer_size) {
         buffer_size *= 2;
         buffer = gotcha_realloc(buffer, buffer_size);
      }
   }

   return (ElfW(auxv_t) *) buffer;
}

static struct link_map *get_vdso_from_auxv()
{
   struct link_map *m;
   ElfW(auxv_t) *auxv, *a;
   ElfW(Ehdr) *vdso_ehdr = NULL;
   ElfW(Phdr) *vdso_phdrs = NULL;
   ElfW(Half) vdso_phdr_num, p;
   ElfW(Addr) vdso_dynamic;

   auxv = get_auxv_contents();
   for (a = auxv; a->a_type != AT_NULL; a++) {
      if (a->a_type == AT_SYSINFO_EHDR) {
         vdso_ehdr = (ElfW(Ehdr) *) a->a_un.a_val;
         break;
      }
   }
   free(auxv);
   if (!vdso_ehdr)
      return NULL;
   
   vdso_phdrs = (ElfW(Phdr) *) (vdso_ehdr->e_phoff + ((unsigned char *) vdso_ehdr));
   vdso_phdr_num = vdso_ehdr->e_phnum;

   for (p = 0; p < vdso_phdr_num; p++) {
      if (vdso_phdrs[p].p_type == PT_DYNAMIC) {
         vdso_dynamic = (ElfW(Addr)) vdso_phdrs[p].p_vaddr;
      }
   }

   for (m = _r_debug.r_map; m; m = m->l_next) {
      if (m->l_addr + vdso_dynamic == (ElfW(Addr)) m->l_ld) {
         return m;
      }
   }
   return NULL;
}

static char* vdso_aliases[] = { "linux-vdso.so",
                                "linux-gate.so",
                                NULL };

static struct link_map *get_vdso_from_aliases()
{
   struct link_map *m;
   char **aliases;

   for (m = _r_debug.r_map; m; m = m->l_next) {
      for (aliases = vdso_aliases; *aliases; aliases++) {
         if (m->l_name && strcmp(m->l_name, *aliases) == 0) {
            return m;
         }
      }
   }
   return NULL;
}

static struct link_map *get_vdso_from_maps()
{
   FILE *maps = fopen("/proc/self/maps", "r");
   ElfW(Addr) addr_begin, addr_end, dynamic;
   char name[4096], line[4096];
   struct link_map *m;
   int result;
   
   for (;;) {
      fgets(line, 4097, maps);
      result = sscanf(line, "%lx-%lx %*s %*s %*s %*s %4096s\n", &addr_begin, &addr_end, name);
      if (result != 3) {
         continue;
      }
      if (strcmp(name, "[vdso]") == 0) {
         fclose(maps);
         break;
      }
      if (feof(maps)) {
         fclose(maps);
         return NULL;
      }
   }

   for (m = _r_debug.r_map; m; m = m->l_next) {
      dynamic = (ElfW(Addr)) m->l_ld;
      if (dynamic >= addr_begin && dynamic < addr_end)
         return m;
   }
   
   return NULL;
}

int is_vdso(struct link_map *map)
{
   static int vdso_checked = 0;
   static struct link_map *vdso = NULL;
   struct link_map *result;

   if (!map)
      return 0;
   if (vdso_checked)
      return (map == vdso);
   
   vdso_checked = 1;

   result = get_vdso_from_aliases();
   if (result) {
      vdso = result;
      return (map == vdso);
   }

   result = get_vdso_from_maps();
   if (result) {
      vdso = result;
      return (map == vdso);
   }

   result = get_vdso_from_auxv();
   if (result) {
      vdso = result;
      return (map == vdso);
   }
   return 0;
}
