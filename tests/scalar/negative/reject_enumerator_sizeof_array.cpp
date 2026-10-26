// expect: rejected cerastes-unsupported_type @ L7
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: encode_named_enumerator returned at :1053 for a top-level sizeof; in C the same shape is reject_enum_list_sizeof_array.c (0018 item 10)
#include <cerastes.h>

enum {
  SZ = sizeof(char[4])
};

int size(void)
{
  return SZ;
}
