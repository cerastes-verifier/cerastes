// expect: frontend-ok
// mode: c++20
//
// Smoke test: the pinned EDG front end parses the libc++ container, string,
// algorithm, stream and utility headers of the system SDK (stock cpfe, native
// macOS configuration).  Before EDG bugs 0005, 0006 and 0007 (internal/edg_bugs)
// were fixed, <algorithm>, <string>, <vector>, <iostream> and most others
// failed:
//   0005: the macOS clang bases predefined __BLOCKS__ (and the blocks syntax
//         was not parsed);
//   0006: they lacked __GCC_DESTRUCTIVE_SIZE and __GCC_CONSTRUCTIVE_SIZE
//         (<new> and everything that includes it);
//   0007: GNU_VECTOR_TYPES_ALLOWED was off, so libc++'s ext_vector_type alias
//         in <__algorithm/simd_utils.h> was a scalar.
// tests/edg-smoke/libcxx_headers_cerastes_fe.cpp runs the same headers through
// the cerastes front end.

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

int main() {
  std::vector<int> v(3, 1);
  std::string s("abc");
  return std::find(v.begin(), v.end(), 2) == v.end() && s.find('b') != std::string::npos ? 0 : 1;
}
