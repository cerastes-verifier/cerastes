// expect: golden
// expect: native-ok
// Pins: must-match for members: a member declared in the class with a contract
// and defined outside with the same contract and a renamed parameter, and a
// member declared with a contract and defined outside with none.
#include <cerastes.h>

struct counter {
  int val;
  void set(int v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(val == v);
  int get() const CERASTES_REQUIRES(val >= 0) CERASTES_ENSURES(r, r == val);
};

void counter::set(int w) CERASTES_REQUIRES(w >= 0) CERASTES_ENSURES(val == w)
{
  val = w;
}

int counter::get() const
{
  return val;
}
