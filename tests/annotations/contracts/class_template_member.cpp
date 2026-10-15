// expect: golden
// expect: native-ok
// Pins: contracts on members of a class template: in-class (prototype and
// instance for box<int>), and an out-of-class member definition that repeats
// the in-class contract identically (must-match, decision 0006 item 6).
#include <cerastes.h>

template <class T>
struct box {
  T v;
  T get() const CERASTES_REQUIRES(v >= 0) { return v; }
  T twice() const CERASTES_ENSURES(r, r >= v);
};

template <class T>
T box<T>::twice() const CERASTES_ENSURES(r, r >= v)
{
  return v + v;
}

int use(const box<int>& b)
{
  return b.get() + b.twice();
}
