// expect: error leak @ L17
// mode: c23, c++20
// twin: tests/memory/owns_block_load_guard.c
// Pins: 0054 item 2 (the leak exemption reads the post-state site only): the old block of *pp, still held by the callee after the overwrite, is not the one the ensures site returns, so it leaks at the closing brace
// Pre-registered from record 0054 item 2 ("The leak exemption", S39 f1z's non-owner counterpart): leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

void renew(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp != NULL) {
    **pp = 1;
  }
}  // <- here
