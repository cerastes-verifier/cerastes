// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/memory/result_value.c
// Pins: 0040 item 4: old(*r) is rejected: the result has no value on entry (never lifted)
#include <cerastes.h>
#include <stddef.h>

int *pick(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == p && cerastes_old(*r) == 1)
{
  return p;
}
