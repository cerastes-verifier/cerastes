// expect: error out-of-bounds @ L23
// mode: c23, c++20
// twin: tests/memory/struct_array_global_loop.c
// Pins: 0063 items 9 and 10: the loop runs to i == 4, one past g.a: out-of-bounds at the read in the loop
// Pre-registered from record 0063 item 9 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

static struct S g = {{1, 2, 3, 4}, 5};

int main(void)
{
  int t = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 5)
  CERASTES_DECREASES(5 - i)
  while (i <= 4) {
    t = g.a[i];  // <- here
    i += 1;
  }
  return t - t;
}
