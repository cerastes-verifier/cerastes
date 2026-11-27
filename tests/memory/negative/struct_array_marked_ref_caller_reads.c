// expect: error uninitialized @ L27
// mode: c23, c++20
// twin: tests/memory/struct_array_marked_ref.c
// Pins: 0064 item 15 (294): the caller wrote a[0] only; ref_to_uninit does not promise the other elements: reading a[1] after the call is uninitialized (the precedent negative/range_marker_ref_to_uninit_caller_read.c)
// Pre-registered from record 0064 item 15 and the precedent range_marker_ref_to_uninit_caller_read.c (uninitialized at the caller's read), before any run: unmeasured for a member array
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void wr(struct S *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->a[0] = 7;
}

int keeps(void)
{
  struct S s;
  s.a[0] = 1;
  wr(&s);
  return s.a[1];  // <- here
}
