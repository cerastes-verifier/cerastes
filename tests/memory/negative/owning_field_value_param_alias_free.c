// expect: error invalid-free @ L18
// mode: c23, c++20
// twin: tests/memory/owning_field_value_param_alias.c
// Pins: 0055 item 6: the same callee frees the alias: it holds no block of s.data (nothing in its contract gave one), so the free is not of an allocation it owns
// Pre-registered from record 0055 item 6 (copies and values): invalid-free at the free (the frozen base, by hand: the same)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int peek(struct buf s)
    CERASTES_ENSURES(r, r == s.n)
{
  int n = s.n;
  free(s.data);  // <- here
  return n;
}

int use(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  struct buf b = { NULL, 3 };
  b.data = (int *)malloc(sizeof(int));
  if (b.data == NULL) {
    return -1;
  }
  int v = peek(b);
  free(b.data);
  return v;
}
