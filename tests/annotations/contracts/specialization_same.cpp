// expect: golden
// expect: native-ok
// Pins: decision 0006 item 6: explicit specializations match the primary
// template's contract: one repeats it verbatim, the other has none and has the
// primary's. (Twin of the spec_* negatives.)
#include <cerastes.h>

template <class T> T twice(T v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r >= v);

template <> int twice<int>(int v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r >= v)
{
  return v + v;
}

template <> long twice<long>(long v)
{
  return v + v;
}
