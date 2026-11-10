// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 7: a loop that frees and re-allocates the owning leaf s.data of a struct local every iteration, with no written invariant on the leaf: the loop gets synthetic sites for the leaf (today: a false `permission` "writes *s.data without owning it", S40 (g) g2_named_auto.c)
// Pre-registered from record 0055 item 7 (loops over a named leaf: new, no base form; S40 (g) g2_named*.c are today's rejection and false alarm): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int churn(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct buf s = { NULL, 0 };
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(s.data);
    s.data = (int *)malloc(sizeof(int));
    if (s.data == NULL) {
      return 1;
    }
    *s.data = i;
    i += 1;
  }
  free(s.data);
  return 0;
}
