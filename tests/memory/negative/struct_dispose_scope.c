// expect: error lifetime @ L19
// mode: c23, c++20
// twin: tests/memory/struct_dispose.c
// Pins: 0043 item 4: the callee kept s.x, so the scope exit finds the object not whole: lifetime at the closing brace
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void dispose(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->x));

void leak(void)
{
  struct S s = {1, 2};
  dispose(&s);
}  // <- here
