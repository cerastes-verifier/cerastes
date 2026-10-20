// expect: golden
// expect: native-ok
// forall over an array of T: the lambda names the template parameter, in a function template and in a member template.
#include <cerastes.h>

// tag::template[]
template <class T>
bool all_pos([[maybe_unused]] const T* data, [[maybe_unused]] int n)
  CERASTES_REQUIRES(n >= 0 && cerastes::forall([](int i, const T* a, int m) { return !(0 <= i && i < m) || a[i] > 0; }, data, n))
{
  return true;
}

template <class U>
struct buffer {
  const U* data;
  int n;

  template <class T>
  bool all_ge([[maybe_unused]] T lo)
    CERASTES_REQUIRES(cerastes::forall([](int i, const U* a, int m, T l) { return !(0 <= i && i < m) || a[i] >= l; }, data, n, lo))
  {
    return true;
  }
};

bool use(const int* p, const long* q, buffer<int>& b)
{
  return all_pos(p, 1) && all_pos(q, 1) && b.all_ge(1) && b.all_ge(2L);
}
// end::template[]
