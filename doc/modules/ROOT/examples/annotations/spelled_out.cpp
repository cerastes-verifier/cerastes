// expect: golden
// expect: native-ok
// The attribute spelling that the macros expand to, guarded for ordinary compilers.
#include <cerastes.h>

// tag::attr[]
#ifdef __CERASTES__
int twice(int x) [[cerastes::requires(x >= 0)]] [[cerastes::ensures(r, r == x + x)]];
#else
int twice(int x);
#endif
// end::attr[]
