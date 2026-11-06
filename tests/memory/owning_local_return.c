// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3: owning locals at a return: the one returned (the function is cerastes::owning, so the caller gets its block) is not a leak, and another owning local freed before the return is not either
// Pre-registered from record 0052 item 3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *first(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return NULL;
  }
  *a = 1;
  CERASTES_OWNING int *b = (int *)malloc(sizeof(int));
  if (b == NULL) {
    return a;
  }
  *b = 2;
  free(b);
  return a;
}

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

int use_both(void)
{
  int *x = first();
  if (x == NULL) {
    return -1;
  }
  int *y = second();
  if (y == NULL) {
    free(x);
    return -1;
  }
  int v = (*x == *y) ? 1 : 0;
  free(y);
  free(x);
  return v;
}
