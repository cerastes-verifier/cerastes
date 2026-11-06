// expect: error unsequenced @ L21
// mode: c23, c++20
// twin: tests/memory/free_arg_sequenced.c
// Pins: 0052 item 6e (audit 2 N8): two((free(p), 0), p): the arguments are unsequenced, and free's effect on *p's object is registered, so it is unsequenced, not permission
// Pre-registered from record 0052 item 6e: unsequenced at the call
#include <cerastes.h>
#include <stdlib.h>

int two(int a, int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(r, cerastes_live(q) && r == a);

int use_two(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  int v = two((free(p), 0), p);  // <- here
  return v;
}
