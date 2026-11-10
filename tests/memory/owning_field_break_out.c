// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 5 (lifetime end at a jump): break out of a loop body whose struct local's leaf was freed first
// Pre-registered from audit round 2 on Parts B/C (probes r3/): brk.c with the free before the break: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    struct buf s = { NULL, 0 };
    s.data = (int *)malloc(sizeof(int));
    if (s.data != NULL) {
      *s.data = 1;
    }
    if (i == 1) {
      free(s.data);
      break;
    }
    free(s.data);
    s.data = NULL;
    i = i + 1;
  }
  return i;
}
