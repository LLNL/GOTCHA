#ifndef GOTCHA_DL_H
#define GOTCHA_DL_H

#include "hash.h"
#include "tool.h"

void handle_libdl();
extern void update_all_library_gots(hash_table_t *bindings);
extern int prepare_symbol(struct internal_binding_t *binding);

#endif
