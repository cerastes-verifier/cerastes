// expect: rejected subscript_out_of_range @ L12
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 4 (as updated): a constant index past the end is rejected even in a branch that cannot run (if (0)): EDG's #175 is an error wherever it fires; clang and GCC are silent there (the stricter reading)
// Pre-registered from record 0059 item 4 (as updated)
#include <cerastes.h>

int f(void)
{
  int a[3] = {1, 2, 3};
  if (0) {
    return a[3];
  }
  return a[0];
}
