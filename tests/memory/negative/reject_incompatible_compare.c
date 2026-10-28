// expect: rejected incompatible_operands @ L13
// mode: c23, c++20
// twin: tests/memory/address_compare.c
// Pins: 0032 item 3: C only warns (#42, incompatible pointer operands; C++ refuses them), verify mode makes it an error
#include <cerastes.h>

int compare_pointers(void)
{
  int a = 0;
  long b = 0;
  int *pi = &a;
  long *pl = &b;
  return pi == pl;
}
