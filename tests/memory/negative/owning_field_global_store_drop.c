// expect: error leak @ L20
// mode: c23, c++20
// twin: tests/memory/owning_field_global_store.c
// Pins: 0055 item 5 (reassignment of a global's leaf): the function overwrites the block g.data holds at entry without freeing it: "assigning to owning field `g.data` drops the block it holds" (owning_global_reassign_leak.c)
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): leak at the store to g.data (the reassignment row); frozen base: leak at the closing brace of renew
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

static struct buf g = { NULL, 0 };

void renew(int v)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  g.data = (int *)malloc(sizeof(int));  // <- here
  if (g.data != NULL) {
    *g.data = v;
  }
}

int main(void)
{
  renew(1);
  renew(2);
  return 0;
}
