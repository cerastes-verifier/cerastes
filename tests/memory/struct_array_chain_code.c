// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 1 (question 306): in code, rr.q->a[i] through a pointer member read from a structure local is an ordinary access (the root's null row, the bound, the permission of the structure local); only a contract place through a heap read with an array member on its path is rejected (struct_array_reject_annotations.c)
// Pre-registered from record 0063 item 1 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

struct R {
  struct S *q;
  int z;
};

int use(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
    CERASTES_ENSURES(r, r == 7)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct R rr;
  rr.q = &s;
  rr.z = 1;
  rr.q->a[i] = 7;
  return rr.q->a[i];
}
