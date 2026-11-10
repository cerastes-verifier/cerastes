// expect: error leak @ L31
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_return_noname.c
// Pins: 0056 item 11 (question 214): the same function with a malloc'd, written block in s.data and the early `return 0;` in the loop that leaves it owned: leak at the return, reported on the owning field (the loop's own return row), and no spurious lifetime row for the address of s.data
// Pre-registered from the re-audit's r1_return_in_loop_leak.c and edg-backend report 5 (one error, leak @ L26 of the program without header, no lifetime): leak at the in-loop return, in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
{
  struct buf s;
  s.n = 0;
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = 1;
  }
  int k = 0;
  int *pk = &k;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *pk = i;
    if (i == n) {
      return 0;
    }
    i = i + 1;
  }
  free(s.data);
  s.data = NULL;
  return 1;
}
