// expect: rejected cerastes-annotation_mismatch @ L8
// expect: rejected cerastes-annotation_mismatch @ L12
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 4: a declaration with must_init and the definition with ref_to_uninit: annotation_mismatch, one per attribute that the two disagree about (each names both declarations), so one at each declaration
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

void init(int *p CERASTES_REF_TO_UNINIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}
