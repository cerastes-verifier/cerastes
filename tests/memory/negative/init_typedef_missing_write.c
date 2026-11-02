// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/init_typedef.c
// Pins: 0042 item 4 (audit S4): the typedef's marker holds: a definition that does not write *p fails postcondition
#include <cerastes.h>

typedef void fn_t(int *p CERASTES_MUST_INIT);
fn_t init;

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}  // <- here
