// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 5, 7: a loop in a function with owns(g) frees and re-allocates the owning leaf g.data of a struct global every iteration; the closure of owns(g) gives the loop its sites, and main ends holding the block (exempt)
// Pre-registered from record 0055 item 7 (loops over a global's leaf: new, no base form for the closure through owns(g) in a loop): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

static struct buf g = { NULL, 0 };

void churn(int n)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(g.data);
    g.data = (int *)malloc(sizeof(int));
    if (g.data != NULL) {
      *g.data = i;
    }
    i += 1;
  }
}

int main(void)
{
  churn(3);
  return 0;
}
