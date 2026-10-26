// expect: rejected cerastes-unsupported_type @ L9
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6 (cut 3 candidate; Q32's 'other known instances'): a non-type template argument is evaluated by the front end alone
#include <cerastes.h>
#include <array>

int arr(void)
{
  return (int)sizeof(std::array<char, 4>);
}
