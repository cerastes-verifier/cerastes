// expect: error uninitialized @ L34
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_struct_copy.c
// Pins: 0055 item 7 and 0054 item 5's wef heuristic (a false alarm pinned, the program is correct; a question for the owner, M4.8): the same loop as owning_field_loop_struct_copy.c with an `int n` member in the structure: the copy `s = t;` also stores an int, so the older stores_type clause (a loop that stores values of the pointee type is held to initializing the objects it owns) makes wef true, and the loop's back-edge row reports that s.data may end an iteration with no value, though the program never reads the block. Pre-existing: the base reports the same error at the same line (checked on cerastes-base)
// Pre-registered from edg-backend report 4 (F/c/ss2.c, reformatted: uninitialized at the loop's closing brace on the base and on the build with 208-211, 213, in both modes; a pin records what the encoder reports)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

int f(void)
{
  struct buf s = { NULL, 0 };
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    struct buf t = { NULL, 0 };
    t.data = (int *)malloc(sizeof(int));
    free(s.data);
    s = t;
    t.data = NULL;
    i = tick(i);
  }
  free(s.data);
  s.data = NULL;
  return 0;
}
