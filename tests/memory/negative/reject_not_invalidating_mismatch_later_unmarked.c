// expect: rejected cerastes-annotation_mismatch @ L12
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2: a declaration with not_invalidating and the definition without it: annotation_mismatch, reported at the later declaration, as 0042 item 4 reports a marker
// Pre-registered from record 0058 item 2: rejected annotation_mismatch at the later declaration (as reject_owning_mismatch.c)
#include <cerastes.h>

void bump(int *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1);

void bump(int *p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p += 1;
}
