// expect: error unsequenced @ L18
// mode: c23, c++20
// twin: tests/memory/struct_array_sequencing.c
// Pins: 0063 item 11 (0039 item 3): without i != j the write and the increment may reach one element of the array member
// Pre-registered from record 0063 item 11 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int shift(struct S *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 4 && 0 <= j && j < 4 && 0 <= p->a[j] && p->a[j] < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == p->a[j])
{
  p->a[i] = p->a[j]++;  // <- here
  return p->a[j];
}
