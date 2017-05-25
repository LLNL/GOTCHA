#ifndef GOTCHA_DL_H
#define GOTCHA_DL_H
void*(*orig_dlopen)(const char* filename, int flags);
void*(*orig_dlsym)(void* handle, const char* name);
void* dlopen_wrapper(const char* filename, int flags);
void* dlsym_wrapper(void* handle, const char* symbol_name);
#endif
