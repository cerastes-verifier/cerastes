// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 13 (question 295, 309): a structure holding exactly 7 array elements (the limit, the constant vir::kMaxByValueArrayElements) is accepted as a by-value parameter and argument (7 Viper formals) (twin negative/struct_array_limit_wrong.c)
// Pre-registered from record 0063 item 13 (Part E), before any run: verifies (time it first: rule 3's 60 s line)
#include <cerastes.h>
#include <stddef.h>

struct B {
  int a[7];
};

int last(struct B b)
    CERASTES_REQUIRES(b.a[6] == 0)
    CERASTES_ENSURES(r, r == 0)
{
  return b.a[6];
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct B b = {0};
  int v = last(b);
  CERASTES_CHECK(v == 0);
  return v;
}
