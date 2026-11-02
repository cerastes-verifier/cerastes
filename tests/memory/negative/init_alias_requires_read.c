// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/init_alias_post.c
// Pins: 0042 item 6: a pre-state read of a marked pointee through an alias is never framed by the marked atom: permission at the clause
#include <cerastes.h>

void f(int *p CERASTES_MUST_INIT, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && q == p)
    CERASTES_REQUIRES(*q > 0)  // <- here
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 1;
}
