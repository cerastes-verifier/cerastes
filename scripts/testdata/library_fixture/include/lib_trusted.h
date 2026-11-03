// Library fixture: a bodyless declaration whose contract is written in a library file is trusted, with the tag
// above it as the reason (decision 0046 item 4).
#ifndef LIB_TRUSTED_H
#define LIB_TRUSTED_H
#include <cerastes.h>

// cerastes:trusted-contract vlibc: lib_triple triples its argument (fixture)
int lib_triple(int x) CERASTES_REQUIRES(-100 <= x && x <= 100) CERASTES_ENSURES(r, r == 3 * x);

#endif
