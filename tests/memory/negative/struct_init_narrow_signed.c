// expect: error conversion @ L14
// mode: c23, c++20
// twin: tests/memory/struct_init_char.c
// Pins: 0043 item 6, audit S7: 300 does not fit a signed char member: conversion at the element
#include <cerastes.h>

struct SB {
  signed char c;
  int y;
};

int narrow(void)
{
  struct SB b = {300, 1};  // <- here
  return b.y;
}
