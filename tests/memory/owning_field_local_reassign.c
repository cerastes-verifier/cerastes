// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 5: a struct local's owning field is reassigned after its block was freed: the reassignment row (0052 item 3) finds the old value held by nothing
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data == NULL) {
    return -1;
  }
  *s.data = 1;
  free(s.data);
  s.data = (int *)malloc(sizeof(int));
  if (s.data == NULL) {
    return -1;
  }
  *s.data = 2;
  free(s.data);
  return 0;
}
