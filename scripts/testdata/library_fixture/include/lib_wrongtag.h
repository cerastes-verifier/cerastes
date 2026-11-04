// Library fixture: a tag that names another function than the declaration below it is an internal error (exit 3),
// as a missing tag is (decision 0046 item 4: the tag names this routine).
#ifndef LIB_WRONGTAG_H
#define LIB_WRONGTAG_H
#include <cerastes.h>

#ifdef __cplusplus
extern "C" {
#endif
// cerastes:trusted-contract vlibc: exit never returns (fixture)
void _Exit(int status) CERASTES_ENSURES(false);
#ifdef __cplusplus
}
#endif

#endif
