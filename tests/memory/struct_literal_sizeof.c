// expect: verified
// expect: native-ok
// c-only: compound literal objects and restrict do not exist in C++ (decision 0044 items 7 and 11)
// Pins: 0044 item 7 (audit N5): a compound literal in sizeof is unevaluated: no hidden object, no store (the file would not verify if one were allocated for the &(...) operand)
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
    CERASTES_ENSURES(r, r == 1)
{
  int n = 0;
  unsigned long z = sizeof((struct S){1, 2}) + sizeof(&(struct S){3, 4}) + sizeof((struct S){.x = 5, .y = 6}.y);
  return z > 0 ? 1 - n : 0;
}
