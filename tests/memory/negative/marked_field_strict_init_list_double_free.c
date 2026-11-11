// expect: error invalid-free @ L26
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/marked_field_strict_init_list.c
// Pins: 0058 item 4: the default checks stay under --strict-init: the leaf's block is freed twice
// Pre-registered from record 0058 item 4: one invalid-free at the second free
#include <cerastes.h>
#include <stdlib.h>

struct pt {
  int *q CERASTES_REF_TO_UNINIT;
  int n;
};

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct pt a = { NULL, 0 };
  struct buf b = { (int *)malloc(sizeof(int)), 0 };
  free(b.data);
  free(b.data);  // <- here
  return a.n;
}
