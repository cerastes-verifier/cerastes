// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 5: a struct local with an owning field: the leaf is stored by name (the value rule), written through, and freed before the scope exits, so the lifetime-end row at each return finds no block held
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): verifies in both modes (a named leaf is an owner in memory, ghost held_s_data; after the free it holds nothing)
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
  *s.data = 5;
  free(s.data);
  return 0;
}
