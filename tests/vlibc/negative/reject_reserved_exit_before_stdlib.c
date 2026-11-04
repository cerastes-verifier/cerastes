// expect: rejected cerastes-annotation_reserved_library @ L8
// c-only: before <stdlib.h> a user declaration in C++ has C++ linkage, which conflicts with the header's extern "C" (EDG error 337, as quick_exit's); the same order is rejected after the header only when the contract differs (EDG's must-match)
// twin: tests/vlibc/exit_redeclared_after.c
// Pins: 0046 item 6 (questions 115, 116): the same identical contract written BEFORE the header is rejected: only an identical redeclaration after vlibc's is discarded
// Here the user's declaration is the first one, so its contract is the user's own.
#include <cerastes.h>

void exit(int status) CERASTES_ENSURES(false);
#include <stdlib.h>

int nonneg(int x)
    CERASTES_REQUIRES(-10 <= x && x <= 10)
{
  return x;
}
