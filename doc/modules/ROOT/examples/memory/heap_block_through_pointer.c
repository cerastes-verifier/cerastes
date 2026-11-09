// expect: verified
// expect: native-ok
// mode: c23, c++20
// A function allocates into *pp and hands the allocation to its caller through cerastes_owns_block(*pp); the pointer cell *pp is owned (and returned) beside it.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(r, cerastes_owns(*pp) && (r != 0 || cerastes_owns_block(*pp)))
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp == NULL) {
    return 1;
  }
  **pp = 5;
  return 0;
}

int main(void)
{
  int *p = NULL;
  if (make(&p) != 0) {
    return 1;
  }
  *p = 6;
  free(p);
  return 0;
}
// end::source[]
