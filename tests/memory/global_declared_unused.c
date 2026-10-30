// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 2 (replaces tests/scalar/negative/reject_global.c, retired by question 81): a mutable global with external linkage that no function names is accepted
#include <cerastes.h>

int counter = 0;

int ident(int a)
{
  return a;
}
