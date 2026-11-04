// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 7: every normal exit of a heap function holds no allocation: freed on each path, a pointer reused after its block was freed, a null result holds nothing
// Pre-registered from record 0047 item 7: verifies (the leak row at each return is a forperm over the token field and no token is held)
#include <cerastes.h>
#include <stdlib.h>

int branches(int c)
    CERASTES_ENSURES(r, r == 0 || r == 1 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  if (c != 0) {
    *p = 1;
    free(p);
    return 1;
  }
  free(p);
  return 0;
}

int replace(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  free(p);
  p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  free(p);
  return 0;
}

int main(void)
{
  return branches(1) + replace();
}
