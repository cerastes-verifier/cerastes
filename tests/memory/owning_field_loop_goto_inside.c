// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 11 (question 214, focused review r6): a `goto` to a label inside the loop (a jump that does not leave it) and a `goto out` after it, a struct local with an owning leaf (NULL) that the loop does not name: the struct is framed out of the loop, which has no exit, and the program verifies
// Pre-registered from the review's r6_goto_inside.c (verified on F2; not yet run in this task): verifies in both modes
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
  s.data = NULL;
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
