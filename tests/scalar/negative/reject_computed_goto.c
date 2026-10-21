// expect: rejected cerastes-unsupported_type @ L12
// expect: rejected cerastes-unsupported_statement @ L14
// c-only: the address of a label and a computed goto are GNU extensions (no C++ equivalent)
// twin: tests/scalar/goto_error_exit.c
// Pins: 0015 item 1: a computed goto and the address of a label are rejected (GNU)
// The pointer variable is an unsupported type (reported at its declaration); the goto *p is its own diagnostic.
#include <cerastes.h>

int computed(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  void *target = &&done;
  if (x > 5) {
    goto *target;
  }
done:
  return x;
}
