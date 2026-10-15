// expect: golden
// expect: native-ok
// Pins: contracts on a constructor (after the parameter list, before the
// mem-initializer list) and a destructor; no result binder is allowed here
// (negative twin: ensures_binder_ctor).
#include <cerastes.h>

struct range {
  int lo;
  int hi;
  range(int l, int h) CERASTES_REQUIRES(l <= h) CERASTES_ENSURES(lo == l && hi == h) : lo(l), hi(h) {}
  ~range() CERASTES_REQUIRES(lo <= hi) {}
};
