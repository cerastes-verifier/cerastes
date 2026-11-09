// expect: error leak @ L21
// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/owning_address_taken_owns_only.c
// Pins: 0054 item 6 ("Positions", S39 f2x): the callee with owns(*pp) alone clears v while v holds a block: reported at the caller's end label, not at the call; the message names the block. Owner question 187: the run reports leak at the return (v's exit row, L21) as well as at the closing brace (the end label, L22), both registered. Pre-registered from record 0054 item 6: leak at the closing brace (the end label), NOT at the call
#include <cerastes.h>
#include <stdlib.h>

void clear(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  *pp = NULL;
}

int use_clear(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  clear(&v);
  return 0;
}  // <- here
