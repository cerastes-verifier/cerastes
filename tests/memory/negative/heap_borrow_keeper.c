// expect: error invalid-free @ L24
// mode: c23, c++20
// twin: tests/memory/heap_borrow_then_free.c
// Pins: 0047 item 6 (0028 item 3a): a callee kept a liveness fraction (live(a) in its requires, no ensures), so the caller cannot free the block while a pointer to it may still be in use
// Pre-registered from record 0047 item 6 (full liveness): invalid-free at the free, one error
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

void keep(struct account *a)
    CERASTES_REQUIRES(cerastes_live(a));

int borrow_then_free(void)
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a == NULL) {
    return -1;
  }
  a->balance = 0;
  keep(a);
  free(a);  // <- here
  return 0;
}
