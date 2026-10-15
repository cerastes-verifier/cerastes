// expect: frontend-warning attribute_namespace_unrecognized @ L57
// mode: c++20
//
// Smoke test: the cerastes front end (cerastes-fe, with prototype instantiations
// in the IL and the cerastes back end behind it) accepts the libc++ headers
// listed in libcxx_headers.cpp, and a program that instantiates libc++'s
// vectorized algorithms.  The only diagnostic is the warning at the line named
// above: no other diagnostic may appear, which is how this kind of test shows
// that the front end ran to the end.  It parses because of EDG bugs 0005 to
// 0007 (internal/edg_bugs; see libcxx_headers.cpp).
//
// Which calls below reach libc++'s vectorized code (__find_vectorized and
// __mismatch_vectorized, which use <__algorithm/simd_utils.h>), checked in
// --il_display output (their instantiations appear only for these) and by reading
// <__algorithm/find.h> and <__algorithm/mismatch.h> of the SDK:
//   - std::find and std::ranges::find on a 2-byte or 8-byte integer element
//     (vector<short>, vector<long long>, and char16_t through the iterators of
//     u16string): __find_vectorized.  On macOS find on a 1-byte element uses
//     memchr and on a 4-byte one (wchar_t's size) wmemchr, so vector<char> and
//     vector<int> do not reach it;
//   - std::mismatch on an integral element (vector<char>, vector<int>):
//     __mismatch_vectorized;
//   - std::equal, std::count, std::string::find and std::u16string::find do
//     not vectorize and are only here as ordinary uses.

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <numeric>
#include <optional>
#include <random>
#include <ranges>
#include <regex>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>


[[unreg_ns::x]] int unregistered;  // the one expected diagnostic

int use() {
  std::vector<char> vc(100, 'a');
  std::vector<int> vi(100, 1);
  std::vector<short> vs(100, 1);
  std::vector<long long> vl(100, 1);
  std::u16string u(100, u'x');
  std::string s(100, 'b');
  int r = 0;
  // Reach __find_vectorized.
  r += std::find(vs.begin(), vs.end(), 7) == vs.end();
  r += std::find(vl.begin(), vl.end(), 7) == vl.end();
  r += std::find(u.begin(), u.end(), u'q') == u.end();
  r += std::ranges::find(vs, 7) == vs.end();
  // Reach __mismatch_vectorized.
  r += std::mismatch(vc.begin(), vc.end(), vc.begin()).first == vc.end();
  r += std::mismatch(vi.begin(), vi.end(), vi.begin()).first == vi.end();
  // Do not vectorize (memchr, wmemchr, memcmp, a loop).
  r += std::find(vc.begin(), vc.end(), 'z') == vc.end();
  r += std::find(vi.begin(), vi.end(), 7) == vi.end();
  r += std::ranges::find(vc, 'z') == vc.end();
  r += std::ranges::find(vi, 7) == vi.end();
  r += s.find('z') == std::string::npos;
  r += s.find("zz") == std::string::npos;
  r += u.find(u'q') == std::u16string::npos;
  r += std::equal(vi.begin(), vi.end(), vi.begin());
  r += std::count(vi.begin(), vi.end(), 1);
  return r;
}
