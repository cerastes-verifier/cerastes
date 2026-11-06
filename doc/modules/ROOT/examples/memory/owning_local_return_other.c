// expect: error leak @ L21
// mode: c23, c++20
// The function returns b and forgets a: a still holds its block at the return.
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
  return b;
}
// end::source[]
