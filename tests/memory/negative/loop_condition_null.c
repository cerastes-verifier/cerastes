// expect: error null-dereference @ L15
// mode: c23, c++20
// twin: tests/memory/loop_condition_cell.c
// Pins: 0038 item 2: the condition's own checks are obligations: *p with p null
#include <cerastes.h>
#include <stddef.h>

int count_to(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n && *p < n) {
    i += 1;
  }
  return i;
}
