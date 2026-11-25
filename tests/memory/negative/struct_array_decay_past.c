// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/struct_array_decay.c
// Pins: 0063 item 8 (0060 item 8): a count of 5 for the four-element array member: the shape row fails at the call, permission
// Pre-registered from record 0063 item 8 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void fill(int *a, int n)
    CERASTES_REQUIRES(0 < n && cerastes_owns_range(a, n))
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  (void)n;
  a[0] = 7;
}

int use(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  fill(s.a, 5);  // <- here
  return s.n;
}
