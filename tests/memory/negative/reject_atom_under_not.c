// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/null_or_owns.c
// Pins: 0037 items 5 and 15: an atom under ! is rejected (Viper has no negated permission; never lifted)
#include <cerastes.h>
#include <stddef.h>

int get(int *p)
    CERASTES_REQUIRES(!cerastes_owns(*p))
{
  return p == NULL;
}
