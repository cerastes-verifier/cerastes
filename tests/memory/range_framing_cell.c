// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 5 (flip A1; was tests/memory/negative/array_reject_contract_element.c): p[0] in a requires beside owns(*p): the cell atom frames the element read at the same address
// Pre-registered from record 0060 item 5, before the code: verifies
#include <cerastes.h>

int first(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p[0] > 0)
{
  return *p;
}
