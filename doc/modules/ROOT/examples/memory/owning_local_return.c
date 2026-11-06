// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning function hands its caller the block of one owning local and frees the other.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *second(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return NULL;
  }
  *a = 1;
  CERASTES_OWNING int *b = (int *)malloc(sizeof(int));
  if (b == NULL) {
    free(a);
    return NULL;
  }
  *b = 2;
  free(a);
  return b;
}
// end::source[]
