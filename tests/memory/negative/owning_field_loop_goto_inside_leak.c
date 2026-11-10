// expect: error leak @ L41
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_goto_inside.c
// Pins: 0056 item 11 (question 214): the same program with a malloc'd, written block in s.data that is never freed: leak at the final return (owning field `s.data` still holds its block); the loop's gotos stay inside it
// Pre-registered from the designed outcome (not yet run): leak at `return s.n;`, in both modes; question 160 applies to the line
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
    if (i == n) {
      goto skip;
    }
    *pk = i;
  skip:
    i = i + 1;
  }
  if (n == 5) {
    goto out;
  }
  k = 0;
out:
  return s.n;
}
