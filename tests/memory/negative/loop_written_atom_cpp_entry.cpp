// expect: error permission @ L15
// mode: c++20
// twin: tests/memory/loop_written_atom_cpp.cpp
// Pins: 0040 item 5: only live(p) is held at entry: the loop enters without owning *p
#include <cerastes.h>
#include <stddef.h>

void touch(int *p, int n)
    CERASTES_REQUIRES(cerastes::live(p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes::live(p))
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes::owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    i += 1;
  }
}
