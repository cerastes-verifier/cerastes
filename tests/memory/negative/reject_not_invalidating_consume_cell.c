// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2 ("The check on the contract"): the marked parameter's requires takes owns(*p) and no ensures atom returns it: unsupported_annotation at the parameter ("the contract of set takes *p without returning it")
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter (the declaration line, as reject_marker_ref_no_owns.c); the exact message is not checked
#include <cerastes.h>

void set(int *p CERASTES_NOT_INVALIDATING)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
{
  *p = 0;
}

int use(void)
{
  int x = 1;
  set(&x);
  return 0;
}
