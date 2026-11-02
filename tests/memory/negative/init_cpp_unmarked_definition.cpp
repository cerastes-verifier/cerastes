// expect: rejected cerastes-annotation_mismatch @ L11
// mode: c++20
// twin: tests/memory/init_cpp.cpp
// Pins: 0042 item 4: a declaration marked and the definition not: annotation_mismatch, reported at the later declaration (0006 item 3, 0040 item 3)
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(cerastes::owns(*p) && *p == 0);

void init(int *p)  // <- here
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(cerastes::owns(*p) && *p == 0)
{
  *p = 0;
}
