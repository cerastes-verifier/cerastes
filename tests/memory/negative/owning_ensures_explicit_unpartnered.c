// expect: error permission @ L10
// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/owning_ensures_explicit.c
// Pins: 0040 item 3 (audit S7): the explicit ensures(owns(*p)) is unconditional but owning's requires is p == NULL || owns(*p): on the path p == NULL the ensures atom has no requires partner (0039 item 7's partners by path), at the clause and at the return
#include <cerastes.h>
#include <stddef.h>

void touch(CERASTES_OWNING int *p)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}
