// expect: golden
// expect: native-ok
// Pins: forall in the contract of a member template of a class template, the lambda naming both
// the class's U (const U * a) and the member template's own T (T l): dependent in the prototype,
// substituted in the instances Buf<int>::all_ge<int> and Buf<int>::all_ge<long>.
// Twin: tests/annotations/negative/forall_member_template_pack.cpp.
#include <cerastes.h>

template <class U>
struct Buf {
  U* data;
  int n;
  template <class T>
  bool all_ge([[maybe_unused]] T lo) CERASTES_REQUIRES(cerastes::forall([](int i, const U* a, int m, T l) { return !(0 <= i && i < m) || a[i] >= l; }, data, n, lo))
  {
    return true;
  }
};

bool use(Buf<int>& b)
{
  return b.all_ge(1) && b.all_ge(2L);
}
