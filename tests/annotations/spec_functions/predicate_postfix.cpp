// expect: golden
// expect: native-ok
// Pins: predicate and pure written after the parameter list in the spelled-out
// form (decision 0006 item 2 allows both placements); natively the same
// function without the attribute.
#include <cerastes.h>

#ifdef __CERASTES__
inline bool nonneg(int x) [[cerastes::predicate]];
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
