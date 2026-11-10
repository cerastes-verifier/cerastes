// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_field_local_free.c
// Pins: 0055 item 5 (lifetime end): the same function without the free: the owning field s.data still holds its block when s's lifetime ends, reported at the return statement as for an owning local (owning_local_leak_return.c)
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): leak at the return statement (the lifetime-end row of s, "owning field `s.data` still holds its block when `s`'s lifetime ends"; the closing brace on the next line is the end-of-function row, which the held leaf is exempt from as an owning local is); frozen base: leak at the closing brace instead
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
  return 0;  // <- here
}
