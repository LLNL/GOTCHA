
CC = gcc

CFLAGS = -g -O

FPIC = -fPIC

PROGS = main

LIBS = libsym.so libmon.so

all: $(PROGS) $(LIBS)

main: main.c
	$(CC) $(CFLAGS) $^ -o $@ -ldl

libsym.so: dlsym.c
	$(CC) $(CFLAGS) $(FPIC) -shared $^ -o $@

libmon.so: monitor.c
	$(CC) $(CFLAGS) $(FPIC) -shared $^ -o $@ -ldl

clean:
	rm -f *.so *.o *~

vclean: clean
	rm -f $(PROGS) $(LIBS) core.*

