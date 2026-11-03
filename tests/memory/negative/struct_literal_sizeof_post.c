// expect: error postcondition @ L25
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_sizeof.c
// Pins: 0044 item 7: the same program with the wrong result: postcondition at the return
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int bump(int *n)
    CERASTES_REQUIRES(cerastes_owns(*n) && *n < 100)
    CERASTES_ENSURES(r, cerastes_owns(*n) && *n == cerastes_old(*n) + 1 && r == *n)
{
  *n += 1;
  return *n;
}

int f(void)
    CERASTES_ENSURES(r, r == 2)
{
  int n = 0;
  unsigned long z = sizeof((struct S){1, 2}) + sizeof(&(struct S){3, 4}) + sizeof((struct S){.x = 5, .y = 6}.y);
  return z > 0 ? 1 - n : 0;  // <- here
}
