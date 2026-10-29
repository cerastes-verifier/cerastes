// expect: rejected cerastes-unsupported_expression @ L9
// mode: c23, c++20
// twin: tests/memory/addr_param.c
// Pins: 0037 items 5 and 15: &param in a contract is rejected (0032 item 4's rule, kept; never lifted)
#include <cerastes.h>
#include <stddef.h>

int f(int a)
    CERASTES_REQUIRES(&a != NULL)
{
  return a;
}
