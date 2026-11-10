// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2 ("a declaration nothing calls is checked on its own"): a not_invalidating declaration whose contract takes owns(*p) and returns nothing, never called, is rejected like a called one
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the declaration (as reject_marker_uncalled_must_init.c)
#include <cerastes.h>

void keep(int *p CERASTES_NOT_INVALIDATING) CERASTES_REQUIRES(cerastes_owns(*p));  // <- here

int main(void)
{
  return 0;
}
