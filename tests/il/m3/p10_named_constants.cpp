// expect: frontend-warning integer_truncated @ L19
// mode: c++20
// M3 probe P10: const globals, enumerators, constexpr variables and constexpr function calls in bodies.
const int kConst = 40;
constexpr int kCx = kConst + 2;
enum E { A = 1, B = A + 5, C = 300 };
enum class SE : signed char { X = 1 };
constexpr int sq(int x) { return x * x; }
constexpr int big(int x) { return x + 2147483647; }
int g_nonconst = 7;
int uses(int x) {
  int a = kConst + x;
  int b = kCx * 2;
  int c = B + x;
  int d = sq(3) + x;
  int e = sq(x);
  int f = big(0) + x;
  int g = g_nonconst + x;
  signed char sc = C;
  return a + b + c + d + e + f + g + sc + (int)SE::X;
}
