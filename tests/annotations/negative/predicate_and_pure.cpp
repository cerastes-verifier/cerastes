// expect: frontend-error cerastes-predicate_and_pure @ L9
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_postfix.cpp
// Mutation of the twin: both predicate and pure on one function.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

#ifdef __CERASTES__
inline bool nonneg(int x) [[cerastes::predicate]] [[cerastes::pure]];
inline int twice(int x) [[cerastes::pure]];
#else
inline bool nonneg(int x);
inline int twice(int x);
#endif

inline bool nonneg(int x)
{
  return x >= 0;
}

inline int twice(int x)
{
  return x + x;
}
