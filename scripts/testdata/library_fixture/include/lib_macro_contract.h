// Library fixture: a macro defined in a library header over an identifier of a trusted contract.  `LIB_FALSE` would turn
// the contract's meaning into whatever the macro says, so it is refused as a user's macro over vlibc's `false` is
// (decision 0046 item 4, audit round 2, S1): library-file directives count in the macro scan, apart from the header's
// own include guard.  lib_trusted.h is the twin without the macro.
#ifndef LIB_MACRO_CONTRACT_H
#define LIB_MACRO_CONTRACT_H
#include <cerastes.h>

#define LIB_FALSE 0

#ifdef __cplusplus
extern "C" {
#endif
// cerastes:trusted-contract vlibc: abort never returns (fixture)
void abort(void) CERASTES_ENSURES(LIB_FALSE != 0);
#ifdef __cplusplus
}
#endif

#endif
