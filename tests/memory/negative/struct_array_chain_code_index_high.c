// expect: error out-of-bounds @ L26
// mode: c23, c++20
// twin: tests/memory/struct_array_chain_code.c
// Pins: 0063 item 1 (question 306): the access through the pointer member is bounded by the array member's length: i == 4 is out-of-bounds
// Pre-registered from record 0063 item 1 (Part C), before any run: the one error
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
    CERASTES_REQUIRES(0 <= i && i <= 4)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct R rr;
  rr.q = &s;
  rr.z = 1;
  rr.q->a[i] = 7;  // <- here
  return rr.z;
}
