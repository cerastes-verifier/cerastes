// expect: verified
// expect: native-ok
// Pins: 0024 item 3: types never trigger the skip; size_t is 8 bytes on this target (target-dependent by design, like plain_char.c)
#include <cerastes.h>
#include <cstddef>

static_assert(sizeof(std::size_t) == 8, "size_t is 64 bits");

int one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}
