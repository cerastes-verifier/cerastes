// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Front-end driver: registers the cerastes annotations with EDG, refuses the
// options cerastes does not support, strips its own options, defines
// __CERASTES__, then runs EDG in process.  EDG calls back_end()
// (src/backend/back_end.cpp) when parsing succeeds.
//
// Every refusal is ONE line on stderr, `cerastes-fe: error: <message>`, and exit
// status 2, before EDG runs (tests/README.md, "Refused options").  The options
// refused here, and why:
//   --remove_unneeded_entities  the walker needs the unpruned IL (decision 0002)
//   -U / -D / --undefine_macro / --define_macro of __CERASTES__
//                               cerastes defines it itself (decision 0008)
//   --gcc --no_gcc --g++ --no_g++ --gnu_version
//                               GNU emulation (decision 0007)
//   --c --c89 --c99 --no_c99 --c11 --c17 --c18 --old_c -K
//                               C before C23 (decision 0007)
//   --create_pch --use_pch --pch --pch_dir
//                               a precompiled header skips the reading of its files
//   --output_mode=sarif         back-end diagnostics cannot be emitted as SARIF yet
// EDG accepts an unambiguous abbreviation of a long option, so a refused option
// is recognised under every spelling EDG would resolve to it.  The language
// modes are checked again on the state itself in back_end() (run_state.cpp).
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

#include "options.h"
#include "register_attributes.h"
#include "run_state.h"

// EDG's entry point, renamed from main by EDG_MAIN (host_envir.h).
extern int edg_main(int argc, char** argv);

namespace {

// Every long option EDG describes (see edg_options.inc).
constexpr const char* kEdgOptions[] = {
#include "edg_options.inc"
};

// Resolves `keyword` (an option name without the leading `--` or a value) the
// way EDG's look_up_option_description does: an exact match, else the only
// option that starts with it.  Returns nullptr when there is none or several.
const char* resolve_option(std::string_view keyword) {
  if (keyword.empty()) return nullptr;
  const char* only = nullptr;
  int matches = 0;
  for (const char* name : kEdgOptions) {
    if (keyword == name) return name;
    if (std::string_view(name).substr(0, keyword.size()) == keyword) {
      only = name;
      ++matches;
    }
  }
  return matches == 1 ? only : nullptr;
}

bool is_one_of(std::string_view name, std::initializer_list<std::string_view> names) {
  for (std::string_view n : names)
    if (name == n) return true;
  return false;
}

[[noreturn]] void refuse(const char* message) {
  cerastes::print_refusal(message);
  std::exit(2);
}

// Options that take a value, as `--name=value` or `--name value`.
bool takes_value(std::string_view name) {
  return is_one_of(name, {"undefine_macro", "define_macro", "gnu_version", "output_mode"});
}

void refuse_macro(std::string_view value, std::string_view option) {
  // `-D__CERASTES__`, `-D__CERASTES__=0`, `-U__CERASTES__`: the name is what
  // is before an `=`.
  std::string_view name = value.substr(0, value.find('='));
  if (name == "__CERASTES__") {
    std::string msg(option);
    msg += " of __CERASTES__ is not supported: cerastes defines __CERASTES__ itself (decision 0008)";
    refuse(msg.c_str());
  }
}

// Checks one command-line argument; `next` is the following argument (or
// nullptr) for options whose value is a separate word.  Returns true if the
// following word was consumed as this option's value.
bool check_argument(const char* arg, const char* next) {
  std::string_view a(arg);
  if (a.size() >= 2 && a[0] == '-' && a[1] == '-') {
    std::string_view body = a.substr(2);
    size_t eq = body.find('=');
    std::string_view keyword = body.substr(0, eq);
    const char* name = resolve_option(keyword);
    if (name == nullptr) return false;
    std::string_view n(name);
    bool has_value = eq != std::string_view::npos;
    std::string_view value = has_value ? body.substr(eq + 1) : (next ? std::string_view(next) : "");
    bool consumes_next = takes_value(n) && !has_value && next != nullptr;
    if (n == "remove_unneeded_entities") {
      refuse("--remove_unneeded_entities is not supported: cerastes needs the unpruned IL "
             "(unused templates and entities)");
    }
    if (n == "undefine_macro" || n == "define_macro") {
      refuse_macro(value, n == "undefine_macro" ? "--undefine_macro" : "--define_macro");
    }
    if (is_one_of(n, {"gcc", "no_gcc", "g++", "no_g++", "gnu_version"})) {
      refuse("GNU emulation (--gcc, --g++, --gnu_version) is not supported: cerastes reads "
             "C++ in Clang emulation mode (decision 0007)");
    }
    if (is_one_of(n, {"c", "c89", "c99", "no_c99", "c11", "c17", "c18", "old_c"})) {
      refuse("C before C23 is not supported: use --c23 (--c selects an older default; older C lets identifier-list "
             "declarators bind a contract's names to globals; decision 0007)");
    }
    if (is_one_of(n, {"create_pch", "use_pch", "pch", "pch_dir"})) {
      refuse("precompiled headers (--create_pch, --use_pch, --pch, --pch_dir) are not supported: "
             "a precompiled header skips the front end's reading of the files it holds, so the "
             "checks of the annotations in them (and of __CERASTES__) could not be made");
    }
    if (n == "output_mode" && value == "sarif") {
      refuse("--output_mode=sarif is not supported yet: cerastes diagnostics cannot be written "
             "as SARIF");
    }
    return consumes_next;
  }
  if (a.size() >= 2 && a[0] == '-') {
    // Short options: -Uname, -U name, -Dname, -D name, -K (old C).
    char letter = a[1];
    if (letter == 'U' || letter == 'D') {
      std::string_view value = a.size() > 2 ? a.substr(2) : (next ? std::string_view(next) : "");
      refuse_macro(value, letter == 'U' ? "-U" : "-D");
      return a.size() == 2 && next != nullptr;
    }
    if (letter == 'K') {
      refuse("C before C23 is not supported: use --c23 (--c selects an older default; older C lets identifier-list "
             "declarators bind a contract's names to globals; decision 0007)");
    }
    // Other short options with a separate value (-I dir, -o file, ...): do not
    // mistake the value for an option.
    if (a.size() == 2 && is_one_of(a.substr(1), {"I", "e", "L", "X", "o", "D", "U"})) return next != nullptr;
  }
  return false;
}

}  // namespace

int main(int argc, char** argv) {
  std::vector<char*> args{argv[0]};
  // cerastes defines __CERASTES__ itself, so that a direct run of this binary
  // cannot silently drop every contract (decision 0008).  It comes before the
  // user's options.
  static char define_cerastes[] = "-D__CERASTES__=1";
  args.push_back(define_cerastes);
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--cerastes_walk") == 0) {
      cerastes::options().walk = true;
    } else if (std::strcmp(argv[i], "--cerastes_walk_selftest") == 0) {
      cerastes::options().walk = cerastes::options().walk_selftest = true;
    } else if (std::strcmp(argv[i], "--cerastes_checks_selftest") == 0) {
      cerastes::options().checks_selftest = true;
    } else {
      bool consumed = check_argument(argv[i], i + 1 < argc ? argv[i + 1] : nullptr);
      args.push_back(argv[i]);
      if (consumed) args.push_back(argv[++i]);
    }
  }
  args.push_back(nullptr);
  cerastes::register_cerastes_annotations();
  std::atexit(cerastes::guard_back_end_ran);
  return edg_main(static_cast<int>(args.size()) - 1, args.data());
}
