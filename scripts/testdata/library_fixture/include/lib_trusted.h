// Library fixture: a bodyless declaration of a reserved name (`abort`) whose contract is written in a library file is
// trusted, with the tag above it as the reason (decision 0046 item 4).  It is declared here, in a unit that does not
// include <stdlib.h>, with C linkage written (a reserved name needs it), and its tag differs from vlibc's so that the
// Viper shows which declaration the reason came from.
#ifndef LIB_TRUSTED_H
#define LIB_TRUSTED_H
#include <cerastes.h>

#ifdef __cplusplus
extern "C" {
#endif
// cerastes:trusted-contract vlibc: abort never returns (fixture)
void abort(void) CERASTES_ENSURES(false);
#ifdef __cplusplus
}
#endif

#endif
