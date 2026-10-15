// expect: golden
// expect: native-ok
// A constructor and a destructor with contracts.
#include <cerastes.h>

// tag::ctor[]
struct range {
  int lo;
  int hi;
  range(int l, int h) CERASTES_REQUIRES(l <= h) CERASTES_ENSURES(lo == l && hi == h) : lo(l), hi(h) {}
  ~range() CERASTES_REQUIRES(lo <= hi) {}
};
// end::ctor[]
