// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D's loop column; owning members, question 293) and 0054 item 5 (owning globals in loops): a loop in a function with owns(g) in both lists writes the array of the block an owning global holds; init replaces the block, destroy frees it
// Pre-registered from the plan's Part D table (section 7: `owner_loop_sites`, `owner_invariants`, `owner_rows` with an array leaf's lines) and record 0064 item 15 (owning members, question 293), read with records 0054 item 5 and 0055 item 7 (loops over owners), and the precedents owning_global_loop_init.c and struct_array_owner_global.c, before any run of this file: verifies in both modes; the rule is record 0064 item 15's clause "A heap block's array part in a loop" (written 2026-11-26 with il-analyst's code), and these expectations are predictions from it, not measurements
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

CERASTES_OWNING static struct S *g = NULL;

void init(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = (struct S *)calloc(1, sizeof(struct S));
}

void fill(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  if (g == NULL) {
    return;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    g->a[i] = i;
    i = i + 1;
  }
}

void destroy(void)
    CERASTES_REQUIRES(cerastes_owns(g))
{
  free(g);
}

int main(void)
{
  init();
  fill();
  destroy();
  return 0;
}
