// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 items 1, 2: an explicit #include <vlibc/stdlib.h> reaches vlibc's pinned header, whose malloc, calloc and free are the trusted contracts
// Pre-registered from record 0047 items 1, 2: verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

int roundtrip(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  int v = *p;
  free(p);
  return v;
}

int zeroed(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)calloc(1, sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int v = *p;
  free(p);
  return v;
}
