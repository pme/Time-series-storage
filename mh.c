#include <stdio.h>
#define EXP8(X) EXP7(X), EXP7(128+X)
#define EXP7(X) EXP6(X), EXP6( 64+X)
#define EXP6(X) EXP5(X), EXP5( 32+X)
#define EXP5(X) EXP4(X), EXP4( 16+X)
#define EXP4(X) EXP3(X), EXP3(  8+X)
#define EXP3(X) EXP2(X), EXP2(  4+X)
#define EXP2(X) EXP1(X), EXP1(  2+X)
#define EXP1(X) EXP0(X), EXP0(  1+X)
#if 0
#define EXP0(X) ((4*(X)+1)*((X)+1) & 0xff)
#else
#define EXP0(X) ((X) ^ ((X)>>4) + 1) * 19 % 257 - 1
#endif

static unsigned char table[256] = { EXP8(0) };

unsigned hash (const char *key)
{
  /* Mc Pearson's Hash Function */
  const unsigned char *k = (const unsigned char*) key;
  unsigned h = 0, c;
  while ((c = *k++) != 0)
     h = table[h^c];
  return h;
}

#if 1
int main(int argc, char *argv[])
{
   char a[100];
   while (scanf("%s", a)>0)
      printf("%s %d\n", a, hash(a));
   return 0;
}
#endif
