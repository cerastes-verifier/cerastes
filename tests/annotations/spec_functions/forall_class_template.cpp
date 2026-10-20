// expect: golden
// expect: native-ok
// Pins: forall over an array of T in the contract of a member of a class template (the
// lambda names the class's T): dependent in the prototype, substituted in each
// instance (Buf<int> and Buf<long>); a function template's form is
// forall_function_template.cpp.
#include <cerastes.h>

template <class T>
struct Buf {
  T* data;
  int n;
  bool all_pos() const CERASTES_REQUIRES(cerastes::forall([](int i, const T* a, int m) { return !(0 <= i && i < m) || a[i] > 0; }, data, n))
  {
    return true;
  }
};

bool use(Buf<int>& b, Buf<long>& c)
{
  return b.all_pos() && c.all_pos();
}
