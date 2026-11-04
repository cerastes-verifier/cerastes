// expect: error leak @ L30
// mode: c23, c++20
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 items 5, 7: the owning result gave the caller a block, which it must free or hand back
// Pre-registered from record 0047 items 5, 7: leak at the caller's closing brace (one row per function; the non-null path holds the block); re-registered after audit round 1 (record 0047 item 7): closing brace
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

CERASTES_OWNING struct account *create_account(void)
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a != NULL) {
    a->balance = 0;
  }
  return a;
}

int use_account(void)
{
  struct account *a = create_account();
  if (a == NULL) {
    return -1;
  }
  int b = a->balance;
  return b;
}  // <- here
