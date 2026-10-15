// expect: frontend-error cerastes-contract_name_hides_member @ L13
// expect: native-ok
// twin: tests/annotations/regressions/shadow_checks_not_other_names.cpp
// Mutation of the twin: the class declares the member limit after the declaration whose
// contract names the global limit, and defines the function outside the class.
// Pins: the member is compared with the position of the name in the contract, not with the
// routine's position, which is the out-of-class definition's (CP8).
#include <cerastes.h>

int limit = 10;

struct S {
  int get(int x) CERASTES_REQUIRES(x < limit);
  int limit = 3;
};

int S::get(int x) { return x; }
