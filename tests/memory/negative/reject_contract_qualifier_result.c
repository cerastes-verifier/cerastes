// expect: rejected cerastes-unsupported_expression @ L9
// mode: c23, c++20
// twin: tests/memory/result_value.c
// Pins: 0040 item 2 (audit round 2 B1): a contract equates an int * result with a const int * parameter (r == v): the caller could write the const object; rejected at the contract
#include <cerastes.h>
#include <stddef.h>

int *unconst(const int *v)
    CERASTES_ENSURES(r, r == v);

int f(void)
{
  const int c = 1;
  int *p = unconst(&c);
  *p = 2;
  CERASTES_CHECK(c == 2);
  return 0;
}
