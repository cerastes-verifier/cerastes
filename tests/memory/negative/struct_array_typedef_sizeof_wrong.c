// expect: error check @ L20
// mode: c23, c++20
// twin: tests/memory/struct_array_typedef_member.c
// Pins: 0063 item 14: sizeof(s.a) is 3 * sizeof(int), not 2: the check fails
// Pre-registered from record 0063 item 14 (Part C), before any run: the one error
#include <cerastes.h>

typedef int T3[3];

struct S {
  T3 a;
  int n;
};

int use(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
{
  struct S s = {{1, 2, 3}, 4};
  s.a[i] = 8;
  CERASTES_CHECK(sizeof(s.a) == 2 * sizeof(int));  // <- here
  return s.n + s.a[i];
}
