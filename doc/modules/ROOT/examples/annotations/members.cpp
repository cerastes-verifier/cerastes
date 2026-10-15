// expect: golden
// expect: native-ok
// Contracts on members: this and earlier members are in scope; an out-of-class definition.
#include <cerastes.h>

// tag::members[]
struct counter {
  int val;
  int get() const CERASTES_REQUIRES(val >= 0) CERASTES_ENSURES(r, r == val)
  {
    return val;
  }
  void set(int v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(val == v);
};

void counter::set(int w) CERASTES_REQUIRES(w >= 0) CERASTES_ENSURES(val == w)
{
  val = w;
}
// end::members[]
