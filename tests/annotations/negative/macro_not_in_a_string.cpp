// expect: frontend-error cerastes-annotation_macro_changed @ L25
// expect: native-ok
// twin: tests/annotations/macros/unrelated_macros.cpp
// Mutation of the twin: `#define cerastes 1` is a real directive, not the text of a string literal.
// Pins: plan R4c: annotation_macro_changed for a real directive that the string-literal case only quotes
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0) CERASTES_ENSURES(r, r <= x)
{
  return x / 2;
}

// Near misses of the protected names.
#define MY_REQUIRES 1
#define CERASTE_H 2
#define CERASTES 3
#define cerastes_x 4
#define Cerastes 5
#define has_cpp_attribute 6
#undef MY_REQUIRES

// Text that only looks like a change.
// #define requires 1
/* #undef cerastes */
#define cerastes 1
const char* g_text2 = "#undef CERASTES_H";
#define TEXT3 "#define requires 1"
const char* g_text3 = TEXT3;

// Inactive groups.
#if 0
#define requires 1
#endif
#if 0
#undef CERASTES_REQUIRES
#endif
#if 0
#define __has_cpp_attribute(x) 0
#endif
#ifdef NEVER_DEFINED_NAME
#undef cerastes
#else
#define ACTIVE_OK 1
#endif
