// Library fixture: a tagged contract on a name vlibc does not reserve.  Trust needs a reserved name (decision 0046
// item 4), so the declaration is rejected `annotation_reserved_library`: a library file cannot make any name trusted.
#ifndef LIB_NONRESERVED_H
#define LIB_NONRESERVED_H
#include <cerastes.h>

// cerastes:trusted-contract vlibc: lib_triple triples its argument (fixture)
int lib_triple(int x) CERASTES_REQUIRES(-100 <= x && x <= 100) CERASTES_ENSURES(r, r == 3 * x);

#endif
