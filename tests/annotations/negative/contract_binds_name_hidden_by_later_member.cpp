// expect: frontend-error cerastes-contract_name_hides_member @ L11
// expect: native-ok
// twin: tests/annotations/contracts/member_inclass.cpp
// Mutation of the twin: the contract names `limit`, which looks up the namespace variable when the contract is parsed, but the complete class later declares a member `limit`: the tag is reported at the contract's use of the name.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int limit = 10;
struct counter {
  int val;
  int room(int x) CERASTES_REQUIRES(x < limit) { return x + limit; }
  int limit = 3;
  int get() const CERASTES_REQUIRES(val >= 0) CERASTES_ENSURES(r, r == val)
  {
    return val;
  }
  void bump() CERASTES_REQUIRES(val < 100) CERASTES_ENSURES(val == cerastes::old(val) + 1)
  {
    val += 1;
  }
};
