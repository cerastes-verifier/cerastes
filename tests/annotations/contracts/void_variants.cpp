// expect: golden
// expect: native-ok
// Pins: ensures(p) on functions returning void, spelled directly and through
// an alias. Twin of the ensures_binder_void and ensures_binder_typedef_void
// negatives (a result name is an error for a void function).
#include <cerastes.h>

using unit = void;

void clear(int x) CERASTES_ENSURES(x > 0);

unit reset(int x) CERASTES_ENSURES(x > 0);
