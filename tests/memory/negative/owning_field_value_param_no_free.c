// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/owning_field_value_param.c
// Pins: 0055 item 6: the callee takes the block (its requires gives owns_block(s.data)) but neither frees it nor hands it on: "owning field `s.data` still holds its block when `s`'s lifetime ends", at the return
// Pre-registered from record 0055 item 6 (copies and values): leak at the return statement (lifetime-end row of the parameter's copy); frozen base: leak at the closing brace of consume
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int consume(struct buf s)
    CERASTES_REQUIRES(s.data == NULL || cerastes_owns_block(s.data))
{
  int n = s.n;
  return n;  // <- here
}

int use(void)
{
  struct buf b = { NULL, 1 };
  b.data = (int *)malloc(sizeof(int));
  if (b.data == NULL) {
    return -1;
  }
  *b.data = 1;
  return consume(b);
}
