// expect: verified
// expect: native-ok
// mode: c23, c++20
// A struct global with an owning field: a function fills it under owns(g), and main ends holding the block (a static owner need not free).
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int len;
};

static struct buf g = { NULL, 0 };

void g_put(int v)
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
  g_put(1);
  g_put(2);
  return 0;
}
// end::source[]
