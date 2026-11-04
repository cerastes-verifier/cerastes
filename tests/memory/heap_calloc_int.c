// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 items 2, 8: calloc's view holds the value 0 with the flag set, so the cell is read with no write first and equals 0; the sizes are constants (nmemb * size = sizeof(int))
// Pre-registered from record 0047 items 2, 8 (calloc: flags set, values zero): verifies
#include <cerastes.h>
#include <stdlib.h>

int zeroed(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)calloc(1, sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int v = *p;
  CERASTES_CHECK(v == 0);
  free(p);
  return v;
}

int zeroed_by_object(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *q = (int *)calloc(1, sizeof *q);
  if (q == NULL) {
    return -1;
  }
  int w = *q;
  free(q);
  return w;
}
