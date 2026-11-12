// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/range_framing_cell.c
// Pins: 0060 item 5: p[1] under owns(*p) is a different slot: the contract reads it without owning it (permission at the clause, no bound row)
// Pre-registered from record 0060 item 5, before the code
#include <cerastes.h>

int first(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p[1] > 0)
{
  return *p;
}
