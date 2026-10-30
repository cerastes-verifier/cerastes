// expect: error overflow @ L13
// mode: c23, c++20
// twin: tests/memory/verifast/global_points_to_syntax.c
// Pins: 0041 item 8 and the port of VeriFast's global_points_to_syntax (VeriFast ran with -disable_overflow_check): without a bound on global1 in the requires the sum global1 + 5 may overflow (reduced to one global: dropping the bound of the full port would fail twice, in the contract's well-formedness and in the body)
#include <cerastes.h>

static int global1 = -10;

void modify_primitive_globals_old_syntax(void)
    CERASTES_REQUIRES(cerastes_owns(global1))
    CERASTES_ENSURES(cerastes_owns(global1))
{
  global1 = global1 + 5;
}

int main(void)
{
  return global1 - global1;
}
