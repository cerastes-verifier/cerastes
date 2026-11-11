// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/memory/array_case_after_plain.c
// Pins: 0059 item 6 (0019 item 6): the default case jumps past the declaration, so a[0] was never written there: the read is uninitialized
// Pre-registered from the M4.5e1a sweep (report 5, s13 plain)
#include <cerastes.h>

int pick(int c)
{
  switch (c) {
    int a[2];
  case 1:
    a[0] = 1;
    return a[0];
  default:
    return a[0];
  }
}
