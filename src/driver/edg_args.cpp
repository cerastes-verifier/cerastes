// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "edg_args.h"

#include <initializer_list>

namespace cerastes::args {
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

// Options that take a value, as `--name=value` or `--name value`.
bool takes_value(std::string_view name) {
  return is_one_of(name, {"undefine_macro", "define_macro", "gnu_version", "output_mode"});
}

// The message for a macro option, kept alive for the life of the process.
const char* keep(std::string s) {
  static std::string last;
  last = std::move(s);
  return last.c_str();
}

const char* macro_message(std::string_view value, std::string_view option) {
  std::string m = macro_refusal(value, option, /*verify_mode=*/false);
  return m.empty() ? nullptr : keep(std::move(m));
}

constexpr const char kOldC[] =
    "C before C23 is not supported: use --c23 (--c selects an older default; older C lets identifier-list "
    "declarators bind a contract's names to globals; decision 0007)";

}  // namespace

namespace {

bool is_identifier(std::string_view n) {
  if (n.empty() || (n[0] >= '0' && n[0] <= '9')) return false;
  for (char ch : n)
    if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_')) return false;
  return true;
}

// A reserved identifier (C23 7.1.3, C++ [lex.name]): two underscores anywhere, or an underscore and a capital.
// Every predefined macro is one (`__cplusplus`, `__STDC_VERSION__`, `__has_cpp_attribute`, `__CERASTES__`, ...).
bool is_reserved_identifier(std::string_view n) {
  if (n.size() >= 2 && n[0] == '_' && ((n[1] >= 'A' && n[1] <= 'Z') || n[1] == '_')) return true;
  return n.find("__") != std::string_view::npos;
}

// The names the annotation machinery reads (decisions 0008 item 2, 0011 item 2): changing any of them could
// make contracts vanish, or change what they mean, without a diagnostic.
bool is_cerastes_name(std::string_view n) {
  static constexpr std::string_view names[] = {"cerastes", "requires", "ensures", "invariant", "check",
                                               "fold",     "unfold",   "predicate", "pure",     "old",
                                               "owns",     "reads",    "forall",    "owns_range", "CERASTES_H"};
  for (std::string_view x : names)
    if (n == x) return true;
  return n.substr(0, 9) == "CERASTES_";
}

}  // namespace

std::string macro_refusal(std::string_view value, std::string_view option, bool verify_mode) {
  // `-D__CERASTES__`, `-D__CERASTES__=0`, `-U__CERASTES__`, `-D'name(x)=...'`: the name is what is before an
  // `=` or a `(`.  EDG trims blanks around the name (`-D" __CERASTES__"` defines it), so compare the trimmed name.
  std::string_view name = value.substr(0, value.find_first_of("=("));
  while (!name.empty() && (name.front() == ' ' || name.front() == '\t')) name.remove_prefix(1);
  while (!name.empty() && (name.back() == ' ' || name.back() == '\t')) name.remove_suffix(1);
  const bool undefine = option == "-U" || option == "--undefine_macro";
  std::string msg(option);
  if (name == "__CERASTES__") {
    msg += " of __CERASTES__ is not supported: cerastes defines __CERASTES__ itself (decision 0008)";
    return msg;
  }
  if (verify_mode && undefine) {
    msg += " of ";
    msg += name;
    msg += " is not supported: `cerastes verify` takes no -U (undefining a predefined macro such as __cplusplus "
           "made <cerastes.h> drop every contract; decision 0011 item 2)";
    return msg;
  }
  if (verify_mode && !is_identifier(name)) {
    msg += " needs a macro name made of letters, digits and underscores, not \"";
    msg += name;
    msg += "\" (decision 0011 item 2)";
    return msg;
  }
  if (name == "defined" || is_reserved_identifier(name)) {
    msg += " of ";
    msg += name;
    msg += " is not supported: it is a reserved identifier (a predefined macro, or a name the implementation owns), "
           "and changing it can silently change what the program or <cerastes.h> means (decisions 0008 item 2, "
           "0011 item 2)";
    return msg;
  }
  if (is_cerastes_name(name)) {
    msg += " of ";
    msg += name;
    msg += " is not supported: <cerastes.h> and the annotation attributes read this name, so changing it could "
           "make contracts vanish or change meaning without a diagnostic (decisions 0008 item 2, 0011 item 2)";
    return msg;
  }
  return "";
}

FeCheck check_fe_argument(const char* arg, const char* next) {
  FeCheck r;
  std::string_view a(arg);
  if (a.size() >= 2 && a[0] == '-' && a[1] == '-') {
    std::string_view body = a.substr(2);
    size_t eq = body.find('=');
    std::string_view keyword = body.substr(0, eq);
    const char* name = resolve_option(keyword);
    if (name == nullptr) return r;
    std::string_view n(name);
    bool has_value = eq != std::string_view::npos;
    std::string_view value = has_value ? body.substr(eq + 1) : (next ? std::string_view(next) : "");
    r.consumes_next = takes_value(n) && !has_value && next != nullptr;
    if (n == "remove_unneeded_entities") {
      r.message = "--remove_unneeded_entities is not supported: cerastes needs the unpruned IL "
                  "(unused templates and entities)";
    } else if (n == "undefine_macro" || n == "define_macro") {
      r.message = macro_message(value, n == "undefine_macro" ? "--undefine_macro" : "--define_macro");
    } else if (is_one_of(n, {"gcc", "no_gcc", "g++", "no_g++", "gnu_version"})) {
      r.message = "GNU emulation (--gcc, --g++, --gnu_version) is not supported: cerastes reads "
                  "C++ in Clang emulation mode (decision 0007)";
    } else if (is_one_of(n, {"c", "c89", "c99", "no_c99", "c11", "c17", "c18", "old_c"})) {
      r.message = kOldC;
    } else if (is_one_of(n, {"create_pch", "use_pch", "pch", "pch_dir"})) {
      r.message = "precompiled headers (--create_pch, --use_pch, --pch, --pch_dir) are not supported: "
                  "a precompiled header skips the front end's reading of the files it holds, so the "
                  "checks of the annotations in them (and of __CERASTES__) could not be made";
    } else if (n == "output_mode" && value == "sarif") {
      r.message = "--output_mode=sarif is not supported yet: cerastes diagnostics cannot be written "
                  "as SARIF";
    } else if (n == "trigraphs") {
      r.message = "--trigraphs is not supported: trigraph replacement rewrites the text of the source "
                  "(\"\?\?/\" is a backslash) after the checks that read it (the scan for a change of "
                  "__CERASTES__) have looked at it (decision 0011)";
    }
    return r;
  }
  if (a.size() >= 2 && a[0] == '-') {
    // Short options: -Uname, -U name, -Dname, -D name, -K (old C).
    char letter = a[1];
    if (letter == 'U' || letter == 'D') {
      std::string_view value = a.size() > 2 ? a.substr(2) : (next ? std::string_view(next) : "");
      r.message = macro_message(value, letter == 'U' ? "-U" : "-D");
      r.consumes_next = a.size() == 2 && next != nullptr;
      return r;
    }
    if (letter == 'K') {
      r.message = kOldC;
      return r;
    }
    // Other short options with a separate value (-I dir, -o file, ...): do not
    // mistake the value for an option.
    if (a.size() == 2 && is_one_of(a.substr(1), {"I", "e", "L", "X", "o", "D", "U"})) r.consumes_next = next != nullptr;
  }
  return r;
}

}  // namespace cerastes::args
