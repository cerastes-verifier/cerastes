// expect: rejected cerastes-uninit_ref_initialized @ L14
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/strict_marked_returns.c
// Pins: 0052 item 4 (question 154): under --strict-init a ref_to_uninit function may not return an unmarked local (here one that holds calloc's initialized block): the caller was told the pointee has no value
// Pre-registered from record 0052 item 4: rejected uninit_ref_initialized at the return (the tag the encoder's first draft uses: "returns p, which may point to an initialized object")
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int *raw(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns_block(r))
{
  int *p = (int *)calloc(1, sizeof(int));
  return p;  // <- here
}
