// expect: frontend-error attr_result_name_no_result_type @ L12
// expect: native-ok
// twin: tests/annotations/contracts/ctor_dtor.cpp
// Mutation of the twin: a result name on a destructor.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

struct range {
  int lo;
  int hi;
  range(int l, int h) CERASTES_REQUIRES(l <= h) CERASTES_ENSURES(lo == l && hi == h) : lo(l), hi(h) {}
  ~range() CERASTES_REQUIRES(lo <= hi) CERASTES_ENSURES(r, lo <= hi) {}
};
