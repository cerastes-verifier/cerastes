// Library fixture: a library routine whose parameter carries cerastes::not_invalidating.  vlibc writes none, and one in a
// library file would be read by nothing (decision 0058 item 2): rejected.
#ifndef LIB_NOT_INVALIDATING_H
#define LIB_NOT_INVALIDATING_H
#include <cerastes.h>

void lib_touch(int *p CERASTES_NOT_INVALIDATING);

#endif
