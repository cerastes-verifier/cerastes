// expect: rejected cerastes-annotation_mismatch @ L10
// mode: c23, c++20
// twin: tests/memory/init_typedef.c
// Pins: 0042 item 4 (audit S4): the typedef carries must_init and the definition does not: annotation_mismatch at the later declaration
#include <cerastes.h>

typedef void fn_t(int *p CERASTES_MUST_INIT);
fn_t init;

void init(int *p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  *p = 0;
}
