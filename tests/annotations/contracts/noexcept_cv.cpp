// expect: golden
// expect: native-ok
// Pins: placement after cv, ref and noexcept qualifiers (decision 0006 item 2).
#include <cerastes.h>

struct s {
  int v;
  int a() const CERASTES_REQUIRES(v > 0) { return v; }
  int b() const noexcept CERASTES_REQUIRES(v > 0) { return v; }
  int c() & noexcept CERASTES_REQUIRES(v > 0) { return v; }
};
