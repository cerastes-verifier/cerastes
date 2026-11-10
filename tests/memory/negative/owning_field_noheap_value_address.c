// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/owning_field_noheap_value.c
// Pins: 0055 items 4, 5 (audit round 2 (j)): the heap-program rule on its own: with no allocation anywhere, an owning field can receive no block the function holds, so the address of a local is a value-rule error at the initializer (h_value_noheap.c)
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), h_value_noheap.c: permission at the initializer
#include <cerastes.h>
#include <stddef.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int main(void)
{
  int x = 1;
  struct buf s = { &x, 0 };  // <- here
  return s.n + x;
}
