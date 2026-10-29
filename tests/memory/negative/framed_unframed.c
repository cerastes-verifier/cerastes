// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/framed_guarded.c
// Pins: 0037 item 7: no atom frames the read of *p in the contract: permission at the clause, reported in source terms
#include <cerastes.h>
#include <stddef.h>

int f(int *p)
    CERASTES_REQUIRES(*p > 0)
{
  return 0;
}
