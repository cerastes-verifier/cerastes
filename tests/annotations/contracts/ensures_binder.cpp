// expect: golden
// expect: native-ok
// Pins: ensures(r, p): the first argument is a result binder with the return
// type; r in p is a binder reference, x a parameter reference.
#include <cerastes.h>

int abs_val(int x) CERASTES_ENSURES(r, r >= 0 && (r == x || r == -x))
{
  return x < 0 ? -x : x;
}

long widen(int x) CERASTES_ENSURES(r, r == x);
