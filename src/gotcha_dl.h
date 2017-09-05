#ifndef GOTCHA_DL_H
#define GOTCHA_DL_H
void* dlopen_wrapper(const char* filename, int flags);
void* dlsym_wrapper(void* handle, const char* symbol_name);
void handle_libdl();
#endif
