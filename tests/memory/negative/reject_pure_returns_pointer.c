// expect: rejected cerastes-pure_function @ L17
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3: a read pure function returns an integer, an enumeration or bool: a pointer result is rejected with the new tag (M4.5f1), at the call
// Pre-registered from record 0062 (Part B), before the code: rejected
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE const int *same(const int *p)
{
  return p;
}

int use(void)
{
  int x = 1;
  CERASTES_CHECK(same(&x) != NULL);
  return x;
}
