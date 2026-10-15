// expect: golden
// expect: native-ok
// Pins: contracts on functions whose return type is a pointer or reference to
// function, written with an alias so the declarator is not nested. Twin of
// the nested_declarator_* negatives, where the same function is declared
// with the nested declarator `int (*pick(int x))(double d)`.
#include <cerastes.h>

using handler = int (*)(double);
using handler_ref = int (&)(double);

handler pick(int x) CERASTES_REQUIRES(x > 0);

handler make(int x) CERASTES_ENSURES(r, r != nullptr);

handler_ref refer(int x) CERASTES_REQUIRES(x > 0);
