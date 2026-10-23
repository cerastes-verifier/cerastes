// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_contract_wf.c
// Pins: 0022 item 5 (owner, 2026-10-23) in a contract: r == x compares an enumeration result with a possibly negative int in the ensures: rejected at the clause
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 3)
    CERASTES_ENSURES(r, r == x)
{
  return (enum color)x;
}
