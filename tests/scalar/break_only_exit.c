// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 4: a loop whose condition exit is impossible is left by its break only
// The invariant keeps i <= limit <= 50, so the condition i < 100 never fails: only the break exit remains.
#include <cerastes.h>

int stop_at(int limit)
    CERASTES_REQUIRES(0 <= limit && limit <= 50)
    CERASTES_ENSURES(r, r == limit)
{
  int i = 0;
  CERASTES_INVARIANT(i <= limit)
  CERASTES_DECREASES(limit - i)
  while (i < 100) {
    if (i == limit) {
      break;
    }
    i += 1;
  }
  CERASTES_CHECK(i == limit);
  return i;
}
