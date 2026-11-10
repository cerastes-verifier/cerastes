// expect: error leak @ L36
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_main_holds.c
// Pins: 0056 item 5, audit round 2 S3: main stores a block into the struct global's owning leaf g.data and the loop drops it by type, `*pd = NULL;` with `pd = &g.data`: the leaf is carried under the entry snapshot hs, so the back-edge row of the loop (the leak row item 5 adds) reports the dropped block at the loop body's closing brace, and not only main's end (the base's error)
// Pre-registered from audit round 2 S3 (probe d_drop.c: leak at the loop's closing brace on the L-2 build, leak at main's end on the base): leak at the closing brace of the loop body, in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf g;

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

int main(void)
{
  g.data = (int *)malloc(sizeof(int));
  if (g.data != NULL) {
    *g.data = 1;
  }
  int **pd = &g.data;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *pd = NULL;
    i = tick(i);
  }
  return 0;
}
