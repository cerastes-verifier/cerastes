// Library fixture: the twin of lib_trusted.h without the tag, on another reserved name.  A library contract with no tag
// is an internal error (exit 3): vlibc_contracts.py refuses such a header before it can be pinned (decision 0046
// items 4 and 9).
#ifndef LIB_UNTAGGED_H
#define LIB_UNTAGGED_H
#include <cerastes.h>

#ifdef __cplusplus
extern "C" {
#endif
void _Exit(int status) CERASTES_ENSURES(false);
#ifdef __cplusplus
}
#endif

#endif
