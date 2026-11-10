// expect: error uninitialized @ L36
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_struct_copy.c
// Pins: 0056 item 2 (the structure-copy route): the same loop and, after it, a read of the leaf under a test: the copy stored an unwritten block into the leaf, the route (a store of the pointer leaf, which the copy expands to) is seen, so wef is false and the read is uninitialized; a scan that dropped the route would make wef true and move the error to the loop's back-edge row (at the loop's closing brace, as owning_field_loop_struct_copy_int_false_alarm.c reports)
// Pre-registered from the designed outcome (probe build/m4.5d2b2c/T9/ss6r.c on the build with 208-211, 213: uninitialized at the read `x = *s.data;`): uninitialized at the read, in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
};

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

int f(void)
{
  struct buf s = { NULL };
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    struct buf t = { NULL };
    t.data = (int *)malloc(sizeof(int));
    free(s.data);
    s = t;
    t.data = NULL;
    i = tick(i);
  }
  int x = 0;
  if (s.data != NULL) {
    x = *s.data;
  }
  free(s.data);
  s.data = NULL;
  return x;
}
