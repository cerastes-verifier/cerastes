// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/owns_block_global_bodiless.c
// Pins: 0054 item 2 ("consumed"): the call took gp's cell and the block it names; the later access needs a permission main no longer holds
// Pre-registered from record 0054 item 2: permission at the access after the call (as heap_consume_use_after.c)
#include <cerastes.h>
#include <stdlib.h>

static int *gp = NULL;

void consume_gp(void)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns_block(gp));

int main(void)
{
  gp = (int *)malloc(sizeof(int));
  if (gp == NULL) {
    return 1;
  }
  *gp = 1;
  consume_gp();
  *gp = 2;  // <- here
  return 0;
}
