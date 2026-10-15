// expect: golden
// expect: native-ok
// Pins: contracts on member functions defined in the class: `this` and the
// members declared earlier are in scope (a `this` node, field nodes); a
// const member; old() of a member.
#include <cerastes.h>

struct counter {
  int val;
  int get() const CERASTES_REQUIRES(val >= 0) CERASTES_ENSURES(r, r == val)
  {
    return val;
  }
  void bump() CERASTES_REQUIRES(val < 100) CERASTES_ENSURES(val == cerastes::old(val) + 1)
  {
    val += 1;
  }
};
