// expect: verified
// expect: native-ok
// c-only: C converts a void* implicitly (initializer, assignment, return, argument); C++ needs a cast
// Pins: 0047 item 2 (0028 item 1): in C the implicit conversion of malloc's result counts as the typed view in an initializer, an assignment, a return and an argument
// Pre-registered from record 0047 item 2: verifies (four conversion sites)
#include <cerastes.h>
#include <stdlib.h>

void release(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  free(p);
}

CERASTES_OWNING int *make_zero(void)
{
  return calloc(1, sizeof(int));
}

int implicit_forms(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  free(p);
  p = malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  free(p);
  release(malloc(sizeof(int)));
  int *z = make_zero();
  if (z == NULL) {
    return -1;
  }
  free(z);
  return 0;
}
