// expect: error conversion @ L14
// mode: c23, c++20
// twin: tests/memory/struct_init_char.c
// Pins: 0043 item 6, audit S7: 300 does not fit an unsigned char member: conversion at the element, under the folded aggregate too (EDG's #69 is suppressed in verify mode)
#include <cerastes.h>

struct B {
  unsigned char c;
  int y;
};

int narrow(void)
{
  struct B b = {300, 1};  // <- here
  return b.y;
}
