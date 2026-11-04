// expect: verified
// expect: native-ok
// Pins: 0047 items 1, 2: std::malloc, std::calloc and std::free through <cstdlib> are vlibc's trusted contracts (C linkage, the typed view of the static_cast)
// Pre-registered from record 0047 items 1, 2 (std::malloc is ::malloc): verifies
#include <cstdlib>
#include <cerastes.h>

int roundtrip()
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  int *p = static_cast<int *>(std::malloc(sizeof(int)));
  if (p == nullptr) {
    return -1;
  }
  *p = 7;
  int v = *p;
  std::free(p);
  return v;
}

int zeroed()
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = static_cast<int *>(std::calloc(1, sizeof(int)));
  if (p == nullptr) {
    return -1;
  }
  int v = *p;
  std::free(p);
  return v;
}
