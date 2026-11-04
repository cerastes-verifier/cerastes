// expect: error uninitialized @ L13
// mode: c23, c++20
// twin: tests/memory/heap_ref_to_uninit_param.c
// Pins: 0047 item 8 (audit S4): the marked parameter holds no value on entry, so the callee reads its cell before writing it
// Pre-registered from record 0047 item 8: uninitialized at the callee's read; re-registered after audit round 1 (record 0047 item 4): audit round 2 S4: BlockMode::Kept and an owns_block borrow on a must_init parameter had no pair
#include <cerastes.h>
#include <stdlib.h>

int touch(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(r, cerastes_owns_block(p) && r <= 2147483647)
{
  return *p;  // <- here
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 3;
  int t = touch(p);
  int v = *p;
  free(p);
  return v - v + t - t;
}
