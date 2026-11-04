// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 item 5: the function returns a pointer into an object it does not own entirely, so the block its ensures promises is missing: permission at the return
// Pre-registered from record 0047 item 5 (the return row of the desugared ensures): permission at the return
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

static struct account shared;

CERASTES_OWNING struct account *create_account(void)
{
  return &shared;  // <- here
}
