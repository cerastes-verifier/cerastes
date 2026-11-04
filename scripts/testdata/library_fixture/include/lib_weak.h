// Library fixture: `#pragma weak` on a library routine that has a body.  Another definition may replace a weak one at
// link time, so its body proves nothing about the calls: rejected (decision 0046 item 8, audit round 2, S2).  Only a
// bodiless library routine is exempt from the redirection checks; lib_body.h is the twin without the pragma.
#ifndef LIB_WEAK_H
#define LIB_WEAK_H
#include <cerastes.h>

int lib_weak_id(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}
#pragma weak lib_weak_id

#endif
