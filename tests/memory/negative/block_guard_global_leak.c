// expect: error leak @ L31
// mode: c23, c++20
// twin: tests/memory/block_guard_global.c
// Pins: 0052 item 6b: main does not free the block the global guard hands it: leak at main's closing brace (main is checked like every function, question 123)
// Pre-registered from record 0052 item 6b: leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

static int g = 1;

int *make(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(g) && g == cerastes_old(g) && (g > 0 ? cerastes_owns_block(r) : r == NULL))
{
  if (g > 0) {
    int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      abort();
    }
    *p = g;
    return p;
  }
  return NULL;
}

int main(void)
{
  int *p = make();
  (void)p;
  return 0;
}  // <- here
