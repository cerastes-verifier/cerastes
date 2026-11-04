// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/bodiless_owns_two_tied.c
// Pins: 0045 item 5 (audit B1): owns(**pp) && owns(**qq) bodiless and untied: the contract could return one object twice or another object: permission at the contract (the clause is one line; the harness may report one row per atom: see the report)
#include <cerastes.h>
#include <stddef.h>

void two(int **pp, int **qq)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*qq) && cerastes_owns(**pp) && cerastes_owns(**qq))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(*qq) && cerastes_owns(**pp) && cerastes_owns(**qq));  // <- here

void use(int **pp, int **qq)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*qq) && cerastes_owns(**pp) && cerastes_owns(**qq))
{
  two(pp, qq);
}
