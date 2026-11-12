// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/range_owning_param.c
// Pins: 0060 item 13 (M4.5e2; the pin record 0060 item 1 says owning(n) on a function lacked): owning(n) on a function (its result) is rejected, naming M4.5e2
// Pre-registered from record 0060 item 13 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

[[cerastes::owning(3)]] int *f(int n)
{
  (void)n;
  return NULL;
}
