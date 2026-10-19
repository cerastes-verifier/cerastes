// expect: frontend-error cerastes-spec_call_in_code @ L11
// A constexpr pure function used where the language evaluates it at compile time is ordinary code.
// tag::bad[]
#include <cerastes.h>

CERASTES_PURE constexpr int scale(int x)
{
  return x + 1;
}

int g_table[scale(3)]; // <- error
// end::bad[]
