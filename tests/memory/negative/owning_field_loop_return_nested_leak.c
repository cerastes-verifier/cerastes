// expect: error leak @ L36
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_return_nested.c
// Pins: 0056 item 11 (question 214): the same nested loops with a malloc'd, written block in s.data, freed after the loops, and the early `return 0;` in the inner loop that leaves it owned: leak at that return (owning field `s.data` still holds its block)
// Pre-registered from the shape of the re-audit's r1 (leak at the in-loop return; not yet run): leak at the nested return, in both modes; question 160 applies to the line
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
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= 2)
    CERASTES_DECREASES(2 - j)
    while (j < 2) {
      *pk = j;
      if (i == n) {
        if (j == 1) {
          return 0;
        }
      }
      j = j + 1;
    }
    i = i + 1;
  }
  free(s.data);
  s.data = NULL;
  return 1;
}
