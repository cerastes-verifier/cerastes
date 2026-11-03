// Library fixture: the twin of lib_trusted.h without the tag.  A library contract with no tag is an internal
// error (exit 3): vlibc_contracts.py refuses such a header before it can be pinned (decision 0046 items 4 and 8).
#ifndef LIB_UNTAGGED_H
#define LIB_UNTAGGED_H
#include <cerastes.h>

int lib_untagged(int x) CERASTES_REQUIRES(-100 <= x && x <= 100) CERASTES_ENSURES(r, r == x);

#endif
