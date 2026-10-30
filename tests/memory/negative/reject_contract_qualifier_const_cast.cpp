// expect: rejected cerastes-unsupported_expression @ L9
// mode: c++20
// twin: tests/memory/result_value.c
// Pins: 0040 item 2 (re-audit of audit round 2's B1): const_cast in an ensures: rejected
#include <cerastes.h>
#include <stddef.h>

int *unconst(const int *v)
    CERASTES_ENSURES(r, const_cast<const int *>(r) == v);

int f()
{
  const int c = 1;
  int *p = unconst(&c);
  *p = 2;
  CERASTES_CHECK(c == 2);
  return 0;
}
