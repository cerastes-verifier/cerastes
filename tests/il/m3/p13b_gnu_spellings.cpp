// expect: golden
// mode: c++20
// M3 probe P13b: GNU and std spellings of the same attributes, and which of them EDG recognizes.
__attribute__((noreturn)) void g1(int);
void g2(int) __attribute__((noreturn));
[[gnu::noreturn]] void g3(int);
[[noreturn]] void g4(int);
__attribute__((unused)) static int g5(int a) { return a; }
__attribute__((deprecated)) int g6(int);
__attribute__((nodiscard)) int g7(int);
int g8(int) __attribute__((const)) __attribute__((nothrow));
alignas(8) int al1;
int g9(int a) __attribute__((aligned(16)));
void g10(int a, int b __attribute__((unused)));
