// expect: rejected wrong_entity_for_attribute @ L14
// mode: c23, c++20
// twin: tests/scalar/goto_cleanup.c
// Pins: 0015 item 2: a check written before a label (the attribute would apply to a labeled statement)
#include <cerastes.h>

int labeled(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  if (x > 5) {
    goto out;
  }
  x += 1;
  CERASTES_CHECK(x > 0) out:
  return x;
}
