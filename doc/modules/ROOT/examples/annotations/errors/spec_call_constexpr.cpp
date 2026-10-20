// expect: frontend-error expr_not_constant @ L11
// A constexpr pure function is not evaluated at compile time: the front end refuses it where a constant is required.
// tag::bad[]
#include <cerastes.h>

CERASTES_PURE constexpr int scale(int x)
{
  return x + 1;
}

int g_table[scale(3)]; // <- error
// end::bad[]
