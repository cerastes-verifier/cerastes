// expect: error uninitialized @ L13
// mode: c23, c++20
// twin: tests/memory/heap_must_init_block.c
// Pins: 0042 item 4 (audit S4): must_init promises the whole allocation with its contents on return, and the body writes nothing
// Pre-registered from record 0047 item 4: uninitialized at the closing brace of init (the return row of the marked parameter's owns_block); re-registered after audit round 1 (record 0047 item 4): audit round 2 S4: BlockMode::Kept and an owns_block borrow on a must_init parameter had no pair
#include <cerastes.h>
#include <stdlib.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p))
{
}  // <- here

int use(void)
    CERASTES_ENSURES(r, r <= 2147483647)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 0;
  }
  init(p);
  int v = *p;
  free(p);
  return v;
}
