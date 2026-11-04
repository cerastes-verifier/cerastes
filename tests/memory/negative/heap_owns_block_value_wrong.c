// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/heap_owns_block_value.c
// Pins: 0047 item 4: the value clause of the ensures fails (the block itself is handed back whole)
// Pre-registered from record 0047 item 4: postcondition at the return
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

struct account *create_account(void)
    CERASTES_ENSURES(r, cerastes_owns_block(r) && r->balance == 0)
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a == NULL) {
    abort();
  }
  a->balance = 1;
  return a;  // <- here
}
