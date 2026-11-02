// expect: rejected cerastes-unsupported_declaration @ L8
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2, audit S3: a struct under #pragma pack is packed like __attribute__((packed)): rejected at the struct (a member's address may be misaligned: M8)
#include <cerastes.h>

#pragma pack(push, 1)
struct S {  // <- here
  char c;
  int x;
};
#pragma pack(pop)

int ident(int a)
{
  return a;
}
