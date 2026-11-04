// expect: error uninitialized @ L26
// mode: c23, c++20
// twin: tests/memory/heap_ref_to_uninit_result.c
// Pins: 0047 items 5, 8: the caller reads the cell before writing it
// Pre-registered from record 0047 item 8: uninitialized at the read
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

CERASTES_REF_TO_UNINIT struct account *create_raw(void)
    CERASTES_ENSURES(r, cerastes_owns_block(r))
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a == NULL) {
    abort();
  }
  return a;
}

int use_raw(void)
{
  struct account *a = create_raw();
  int b = a->balance;  // <- here
  free(a);
  return b;
}
