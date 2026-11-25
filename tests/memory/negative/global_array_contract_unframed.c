// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/global_array_contract_read.c
// Pins: 0063 item 7: the element read g[0] in a requires with no owns(g) before it: the contract of f reads g[0] without owning it (permission at the clause, as global_old_unframed.c)
// Pre-registered from record 0063 item 7 (Part B), before any run: the one error
#include <cerastes.h>

int g[3];

void f(void)
    CERASTES_REQUIRES(g[0] == 1)  // <- here
{
}
