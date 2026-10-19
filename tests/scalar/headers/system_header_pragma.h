#include <cerastes.h>
#pragma GCC system_header
// The GCC spelling: EDG ignores #pragma clang system_header (warning #161), so only this one makes the rest of
// the file a system header.

static inline int after_pragma_h(int x)
{
  return x;
}
