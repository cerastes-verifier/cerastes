// expect: rejected cerastes-annotation_mismatch @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2: a declaration without not_invalidating and the definition with it: annotation_mismatch, reported at the declaration without it (as reject_owning_oneline_mismatch.c reports the prototype)
// Pre-registered from record 0058 item 2: rejected annotation_mismatch at the declaration without the marker (the earlier one); if the encoder reports the later one the line is corrected after the first run (question 160)
#include <cerastes.h>

void bump(int *p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1);

void bump(int *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p += 1;
}
