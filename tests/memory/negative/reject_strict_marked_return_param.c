// expect: rejected cerastes-uninit_ref_initialized @ L14
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/strict_marked_returns.c
// Pins: 0052 item 4 (question 154): under --strict-init a ref_to_uninit function may not return a pointer parameter that is not marked (its pointee may be initialized): anything but a marked local, a null constant or a call of a ref_to_uninit function is rejected
// Pre-registered from record 0052 item 4: rejected uninit_ref_initialized at the return (the tag the encoder's first draft uses)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int *pass_on(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(r, r == NULL || cerastes_owns_block(r))
{
  return p;  // <- here
}
