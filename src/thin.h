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

#if !defined(THIN_H_)
#define THIN_H_

#include "gotcha/gotcha.h"
#include "tool.h"

void *create_trampoline(void *prewrapper, void *postwrapper, void *param, void *tramp_memory, int num);
void *create_thin_wrapper(gotcha_binding_t *binding, void *tramp_memory, int binding_num);
void *allocate_trampoline_memory(int num_actions);

#endif