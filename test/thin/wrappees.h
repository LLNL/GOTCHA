#if !defined(WRAPPEES_H_)
#define WRAPPEES_H_

struct nine_t {
   long nine;
   char unused;
};

struct bigparam_t {
   int buffer[1024];
};

int many_params(int one, double two, float three, char four, char *five, long six, void *seven, short eight, struct nine_t nine, int *ten, long eleven_twelve[2]);

int bigparam(struct bigparam_t bp);

double fp_func(double one, double two, double three, double four,
               double five, double six, double seven, double eight,
               double nine, double ten);

typedef int (*call_recurse_t)(int);
int recurse(int num, call_recurse_t cr);

#endif
