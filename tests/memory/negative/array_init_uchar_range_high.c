// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/memory/array_init_uchar_range.c
// Pins: 0059 item 3 (audit round 2, item 5c): 300 does not fit an unsigned char: conversion at the element (the declaration's line)
// Pre-registered from audit round 2 of record 0059 (item 5c)
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 1)
{
  unsigned char a[2] = {300, 1};
  return a[1];
}
