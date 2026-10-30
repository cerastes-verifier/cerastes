// expect: verified
// expect: native-ok
// mode: c23, c++20
// Port of the scalar half of VeriFast's examples/globals.c (38 lines; ~/Documents/verifast at 0fb63506) to cerastes (decision 0041, M4.5b2a).
// Adaptation: `requires x |-> 7` / `ensures x |-> 8` become `cerastes_owns(x) && x == 7` / `cerastes_owns(x) && x == 8`; main: `open_module()` and the `module` ghost are gone
// (main receives x from program_start, 0041 item 4), `assert(x0 == 0)` is the zero-initialization check as a CERASTES_CHECK, and the check after m() reads x.
// Left out: `struct counter *c` with `c->f` (a pointer global to a struct: M4.5c; its `malloc`, `free` and the final `leak`: M4.5d2), and `if (ctr == 0) abort();`.
// Line map (this file : VeriFast's):  11 : 3   13 : 11   17 : 15   18 : 16   19 : 18   21 : 20   23 : 25   24 : 26   25 : 27   26 : 32   28 : 34
#include <cerastes.h>

static int x;

void m(void)
    CERASTES_REQUIRES(cerastes_owns(x) && x == 7)
    CERASTES_ENSURES(cerastes_owns(x) && x == 8)
{
  int y = x;
  x = y + 1;
}

int main(void)
{
  int x0 = x;
  CERASTES_CHECK(x0 == 0);
  x = 7;
  m();
  int x1 = x;
  CERASTES_CHECK(x1 == 8);
  return x0 + x1 - 8;
}
