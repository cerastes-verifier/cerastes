// expect: rejected cerastes-unsupported_expression @ L10
// c-only: the address of a label (&&L) is a GNU extension
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 1: the address of a label is rejected, also when it is only converted to an integer
#include <cerastes.h>

int address(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  long addr = (long)&&done;
  if (addr == 0) {
    return -1;
  }
done:
  return x;
}
