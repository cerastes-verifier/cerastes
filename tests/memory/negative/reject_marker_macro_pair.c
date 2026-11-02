// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 4 (audit S4): two declarations of f from one macro expansion have one position, so cerastes cannot tell their marker apart: rejected
#include <cerastes.h>

#define DECL_AND_DEF void f(int *p CERASTES_MUST_INIT); void f(int *p CERASTES_MUST_INIT) { *p = 0; }
DECL_AND_DEF  // <- here
