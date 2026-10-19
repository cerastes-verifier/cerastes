// expect: frontend-warning integer_overflow @ L10
// expect: frontend-warning integer_overflow @ L14
// expect: frontend-warning unrecognized_pragma @ L15
// expect: frontend-warning integer_overflow @ L16
// expect: frontend-warning unrecognized_gcc_pragma @ L17
// expect: frontend-warning integer_overflow @ L18
// M3 probe P3d: which diagnostic pragmas EDG honours (diag_suppress, diag_default) and which it does not
// (the clang and GCC ones).
#include <limits.h>
int a1() { return INT_MAX + 1; }
#pragma diag_suppress 61
int a2() { return INT_MAX + 1; }
#pragma diag_default 61
int a3() { return INT_MAX + 1; }
#pragma clang diagnostic ignored "-Winteger-overflow"
int a4() { return INT_MAX + 1; }
#pragma GCC diagnostic ignored "-Woverflow"
int a5() { return INT_MAX + 1; }
