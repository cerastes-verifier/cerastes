// expect: golden
// expect: native-ok
// An explicit specialization repeats the primary template's contract, or carries none.
#include <cerastes.h>

// tag::spec[]
template <class T> T twice(T v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r >= v);

template <> int twice<int>(int v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r >= v)
{
  return v + v;
}

template <> long twice<long>(long v)
{
  return v + v;
}
// end::spec[]
