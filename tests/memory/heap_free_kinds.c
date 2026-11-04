// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 items 2, 6: free takes a possibly null pointer (free(NULL) does nothing), a block no member of which was written (free needs no value) and a block the caller owned whole (owns_block in a requires, consumed)
// Pre-registered from record 0047 items 2, 4, 6: verifies; free(p) and free(s) of possibly-null, never-written blocks pass the free rows
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int x;
  int y;
};

int release_unset(void)
    CERASTES_ENSURES(r, r == 0)
{
  int *p = (int *)malloc(sizeof(int));
  struct S *s = (struct S *)malloc(sizeof(struct S));
  free(p);
  free(s);
  return 0;
}

void release_owned(struct S *s)
    CERASTES_REQUIRES(cerastes_owns_block(s))
{
  free(s);
}

int main(void)
{
  struct S *s = (struct S *)malloc(sizeof(struct S));
  if (s != NULL) {
    s->x = 1;
    s->y = 2;
    release_owned(s);
  }
  return release_unset();
}
