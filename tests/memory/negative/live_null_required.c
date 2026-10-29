// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/live_null.c
// Pins: 0037 items 5 and 8: live(p) needs p != NULL, so the call with NULL fails permission
#include <cerastes.h>
#include <stddef.h>

int is_null(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(p) && r == (p == NULL));

int use_is_null(void)
{
  return is_null(NULL);
}
