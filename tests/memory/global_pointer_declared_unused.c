// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 2 (replaces tests/memory/negative/reject_pointer_global.c, retired by question 81): a pointer global (an int *, null at program start) that no function names is accepted
#include <cerastes.h>

int *gp;

int ident(int a)
{
  return a;
}
