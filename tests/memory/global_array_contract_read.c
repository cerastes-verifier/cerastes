// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 7 (flip A of M4.5e1e1, the staging pin global_array_reject_contract_read.c retired): owns(g) and an element read g[0] of an array global in requires and ensures, the read framed by the atom and kept (a body that does not write); unchanged, line for line (twin negative/global_array_contract_unframed.c)
// Pre-registered from record 0063 item 7 (Part B), before any run: verifies
// (the staging pin's expect line is retired by this flip; the program is unchanged, line for line)
// (the staging pin's expect line is retired by this flip; the program is unchanged, line for line)
#include <cerastes.h>
#include <stddef.h>

int g[3];

void f(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[0] == 1)
    CERASTES_ENSURES(cerastes_owns(g) && g[0] == 1)
{
}
