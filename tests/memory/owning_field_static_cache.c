// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 5, 0041 item 12: a static struct local with an owning field as a cache: the function fills it once and keeps the block (the implicit owns(cache) of the function and of every caller carries the closure of the field), and main ends holding it (exempt)
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int next(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  static struct buf cache;
  if (cache.data == NULL) {
    cache.data = (int *)malloc(sizeof(int));
    if (cache.data == NULL) {
      return -1;
    }
  }
  *cache.data = 1;
  return *cache.data - 1;
}

int main(void)
{
  int a = next();
  int b = next();
  return a + b;
}
