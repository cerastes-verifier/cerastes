// expect: golden
// Contrast probe: constant expressions written in a function template's body
// are operations of that body, and an enumerator's initializer, written
// elsewhere, is not.  The body of `fold` contains non-dependent constant
// expressions (3 + 4, 1 << 4, 0u - 1u, 2 * 5, 100 / 3, -1) and a dependent
// one (sizeof(T) * 2), and refers to the enumerator EA, whose initializer
// `1 << 4` is declared outside any routine.  Every written operation must
// appear as an `op` line under `fold`, with the position where it is written
// in the body (EDG may have folded some of them; the golden records which
// still show as operations, and the il_display check says whether the others
// were folded away or lost).  Nothing from the initializer of EA (an
// `eok_shiftl` at the enumerator's position, line 15) may appear under any
// routine.

enum E { EA = 1 << 4 };

template <typename T>
T fold(T v) {
  int a = 3 + 4;
  int b = 1 << 4;
  unsigned c = 0u - 1u;
  int d = 2 * 5;
  int e = 100 / 3;
  int f = -1;
  unsigned long g = sizeof(T) * 2;
  return v + EA + a + b + (int)c + d + e + f + (int)g;
}
