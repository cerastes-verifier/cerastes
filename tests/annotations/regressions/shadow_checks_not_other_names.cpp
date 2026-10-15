// expect: golden
// expect: native-ok
// twin-exempt: regression test for false positives of the shadow and member-hiding checks (CP7, CP8)
// Pins: a member function called through an object other than this (v.size()) is not what the
// loop variable hides, and a global written qualified (::limit) where the class declared its
// member before the contract is not hidden by it. (Cfg::i and Mode::i are rejected: the checks
// fail closed, see the negatives invariant_shadowed_qualified_static_member and _using_enum.)
#include <cerastes.h>

struct V {
  int size() const { return 0; }
};

int other_class_member_function([[maybe_unused]] const V& v, int n)
{
  int s = 0;
  CERASTES_INVARIANT(v.size() == 0) for (int size = 0; size < n; ++size) s += 1;
  return s;
}

int limit = 10;

struct S {
  int limit;
  int get(int x) CERASTES_REQUIRES(x < ::limit) { return x; }
};
