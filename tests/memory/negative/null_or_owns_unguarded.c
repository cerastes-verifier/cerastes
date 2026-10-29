// expect: error null-dereference @ L12
// mode: c23, c++20
// twin: tests/memory/null_or_owns.c
// Pins: 0037 item 5: the contract accepts NULL, the body does not guard the dereference
#include <cerastes.h>
#include <stddef.h>

int get(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, (p == NULL || cerastes_owns(*p)) && (p == NULL ? r == 0 : (r == *p && *p == cerastes_old(*p))))
{
  return *p;
}
