// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 5: a function that is not main stores the owning leaf of a struct global by name under owns(g) (its closure: g.data == NULL || owns_block(g.data) in both lists): it frees the block it holds and takes a new one, which it gives back through its ensures
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

static struct buf g = { NULL, 0 };

void renew(int v)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g.data);
  g.data = (int *)malloc(sizeof(int));
  if (g.data != NULL) {
    *g.data = v;
  }
}

int main(void)
{
  renew(1);
  renew(2);
  return 0;
}
