// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 (0052 item 6(b)): a heap-reading guard beside a block site through a load (*pp == NULL || owns_block(*pp), the read of *pp framed by owns(*pp) before it): the callee frees the old block, allocates a new one and returns it; the caller's p is NULL or holds the block each time
// Pre-registered from record 0054 item 2 (the S39 (f) callee renew; guard reads framed by owns(*pp)): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void renew(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  free(*pp);
  *pp = (int *)malloc(sizeof(int));
  if (*pp != NULL) {
    **pp = 1;
  }
}

int use_renew(void)
    CERASTES_ENSURES(r, r == 0)
{
  int *p = NULL;
  renew(&p);
  renew(&p);
  free(p);
  return 0;
}
