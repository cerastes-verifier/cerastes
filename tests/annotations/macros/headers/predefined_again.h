// A system-like header (`#pragma GCC system_header`) that undefines a predefined macro and defines it again with the
// compiler's own definition, as glibc 2.43's <stdint.h> does with __INT64_C and __UINT64_C (decision 0008 item 2, as
// amended by record 0031 item 9).  __CHAR_BIT__ is 8 on every platform cerastes supports.
#pragma GCC system_header
#undef __CHAR_BIT__
#define __CHAR_BIT__  8
