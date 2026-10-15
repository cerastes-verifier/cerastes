// expect: golden
// expect: native-ok
// Pins: decision 0006 item 5: a reference parameter in ensures reads the object
// on exit, old(x) reads it on entry (a call, not a special node); the same
// shape with a pointer parameter and old(*p).
#include <cerastes.h>

void incr(int& x) CERASTES_REQUIRES(x < 1000) CERASTES_ENSURES(x == cerastes::old(x) + 1)
{
  x += 1;
}

void incr_ptr(int* p) CERASTES_REQUIRES(*p < 1000) CERASTES_ENSURES(*p == cerastes::old(*p) + 1)
{
  *p += 1;
}
