// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/init_alias_post.c
// Pins: 0042 item 6: a post-state read through an alias of a ref_to_uninit-only pointee is not framed either (the flag is not promised): permission at the clause
#include <cerastes.h>

void f(int *p CERASTES_REF_TO_UNINIT, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && q == p)
    CERASTES_ENSURES(cerastes_owns(*p) && q == p)
    CERASTES_ENSURES(*q == *q)  // <- here
{
  *p = 1;
}
