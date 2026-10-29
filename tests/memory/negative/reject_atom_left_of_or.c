// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/null_or_owns.c
// Pins: 0037 items 5 and 15: an atom as the left operand of || is rejected (Viper refuses acc under ||; write p == NULL || owns(*p); never lifted)
#include <cerastes.h>
#include <stddef.h>

int get(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) || p == NULL)
{
  return p == NULL;
}
