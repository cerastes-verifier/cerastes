// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pre-registered from record 0054 item 3 and question 180: the owning local v takes the block of the plain pointer w on the last iteration; the invariant says when, so after the loop v holds it and is freed.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  int *w = (int *)malloc(sizeof(int));
  if (w == NULL) {
    return 1;
  }
  *w = 0;
  CERASTES_OWNING int *v = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && (i < 3 || v != NULL))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == 2 && v == NULL) {
      v = w;
    }
    *w = i;
    i += 1;
  }
  free(v);
  return 0;
}
// end::source[]
