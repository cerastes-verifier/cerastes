// expect: error uninitialized @ L23
// mode: c23, c++20
// twin: tests/memory/struct_ref_to_uninit.c
// Pins: 0043 item 9, 0042 item 6: ref_to_uninit promises no value: s.y was uninitialized and the callee did not write it
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void reset(struct S *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->x = 0;
}

int use_reset(void)
{
  struct S s;
  reset(&s);
  return s.y;  // <- here
}
