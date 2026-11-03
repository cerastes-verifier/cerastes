// Library fixture: a predefined macro redefined in a library header (no `#pragma GCC system_header`): the exemption of
// record 0031 item 9 covers system files, not library files, so this is annotation_macro_changed.
#ifndef LIB_MACRO_H
#define LIB_MACRO_H
#undef __CHAR_BIT__
#define __CHAR_BIT__ 8
#endif
