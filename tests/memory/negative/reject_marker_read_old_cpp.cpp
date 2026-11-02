// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 6 (question 86): the C++ spelling cerastes::old(*p) of a marked parameter (the C spelling cerastes_old is in reject_marker_read_old.c, in both modes)
#include <cerastes.h>

void f(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(cerastes::owns(*p) && *p == cerastes::old(*p))  // <- here
{
  *p = 0;
}
