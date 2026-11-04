// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10's follow-up (re-audit B1, M4.5d2v): a pointer local that holds a member's address (int *x = &s->b) is a heap place of the loop although the loop's body uses it, and the block freed after the loop is the one it points into
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int a;
  int b;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct S *s = (struct S *)calloc(1, sizeof(struct S));
  if (s == NULL) {
    return -1;
  }
  int *x = &s->b;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *x = i;
    i += 1;
  }
  free(s);
  return 0;
}
