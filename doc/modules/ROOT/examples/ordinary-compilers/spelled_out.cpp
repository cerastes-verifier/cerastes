// expect: golden
// expect: native-ok
// The expansion under the CERASTES define, written out, next to what an ordinary compiler sees.
#include <cerastes.h>

// tag::expansion[]
#ifdef __CERASTES__
int twice(int x) [[cerastes::requires(x >= 0)]];
#else
int twice(int x);
#endif
// end::expansion[]
