// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "silicon_output.h"

#include <regex>
#include <sstream>

namespace cerastes {
namespace {

// (message prefix, Viper error id, harness category): `_VIPER_ERRORS` in scripts/run_tests.py, in order.  A `.`
// of the Python patterns (re.DOTALL) is `[\s\S]` here: ECMAScript's `.` stops at a newline.
struct ErrorKind {
  const char* prefix;
  const char* id;
  const char* category;
};
const ErrorKind kErrorKinds[] = {
    {R"(Assignment might fail\.)", "assignment.failed", "assignment"},
    {R"(Method call might fail\.)", "call.failed", "call"},
    {R"(Contract might not be well-formed\.)", "not.wellformed", "wellformedness"},
    {R"(The precondition of method \S+ might not hold\.)", "call.precondition", "precondition"},
    {R"(Precondition of function \S+ might not hold\.)", "application.precondition", "precondition"},
    {R"(Exhale might fail\.)", "exhale.failed", "exhale"},
    {R"(Inhale might fail\.)", "inhale.failed", "inhale"},
    {R"(Conditional statement might fail\.)", "if.failed", "if"},
    {R"(While statement might fail\.)", "while.failed", "while"},
    {R"(Assert might fail\.)", "assert.failed", "assert"},
    {R"(Function (?:\S+ )?might not terminate\.)", "termination.failed", "termination"},
    {R"(Method might not terminate\.)", "termination.failed", "termination"},
    {R"(Loop might not terminate\.)", "termination.failed", "termination"},
    {R"(Postcondition of \S+ might not hold\.)", "postcondition.violated", "postcondition"},
    {R"(Folding [\s\S]+? might fail\.)", "fold.failed", "fold"},
    {R"(Unfolding [\s\S]+? might fail\.)", "unfold.failed", "unfold"},
    {R"(Packaging wand might fail\.)", "package.failed", "package"},
    {R"(Applying wand might fail\.)", "apply.failed", "apply"},
    {R"(Loop invariant [\s\S]+? might not be preserved\.)", "invariant.not.preserved", "invariant-preserved"},
    {R"(Loop invariant [\s\S]+? might not hold on entry\.)", "invariant.not.established", "invariant-entry"},
    {R"(Function might not be well-formed\.)", "function.not.wellformed", "wellformedness"},
    {R"(Predicate might not be well-formed\.)", "predicate.not.wellformed", "wellformedness"},
    {R"(Magic wand might not be well-formed\.)", "wand.not.wellformed", "wellformedness"},
    {R"(Referencing a wand might fail\.)", "letwand.failed", "letwand"},
    {R"(Quasihavoc might fail\.)", "quasihavoc.failed", "quasihavoc"},
    {R"(Quasihavocall might fail\.)", "quasihavocall.failed", "quasihavoc"},
    {R"(Applying heuristics failed\.)", "heuristics.failed", "heuristics"},
    {R"(An internal error occurred\.)", "internal", "internal"},
};

// `_VIPER_REASONS`.
struct ReasonKind {
  const char* id;
  const char* pattern;
};
const ReasonKind kReasons[] = {
    {"assertion.false", R"(Assertion [\s\S]+ might not hold\.)"},
    {"receiver.null", R"(Receiver of [\s\S]+ might be null\.)"},
    {"division.by.zero", R"(Divisor [\s\S]+ might be zero\.)"},
    {"negative.permission", R"(Fraction [\s\S]+ might be negative\.)"},
    {"permission.not.positive", R"(Fraction [\s\S]+ might not be positive\.)"},
    {"insufficient.permission", R"(There might be insufficient permission to access [\s\S]+)"},
    {"invalid.perm.multiplication", R"(Permission multiplication might not be possible[\s\S]*)"},
    {"wand.not.found", R"(Magic wand instance not found\.)"},
    {"qp.not.injective", R"(Quantified resource [\s\S]+ might not be injective\.)"},
    {"quasihavocall.not.injective", R"(Quasihavocall statement [\s\S]+ might not be injective\.)"},
    {"labelled.state.not.reached", R"(Did not reach labelled state [\s\S]+)"},
    {"seq.index.negative", R"(Index [\s\S]+ into [\s\S]+ might be negative\.)"},
    {"seq.index.length", R"(Index [\s\S]+ into [\s\S]+ might exceed sequence length\.)"},
    {"map.key.contains", R"(Map [\s\S]+ might not contain an entry at key [\s\S]+)"},
    {"epsilon.as.param", R"(The parameter [\s\S]+ might be an epsilon permission[\s\S]*)"},
    {"feature.unsupported", R"([\s\S]+ is not supported\.[\s\S]*)"},
    {"unexpected.node", R"([\s\S]+ occurred unexpectedly\.[\s\S]*)"},
    // silver's termination plugin (TerminationErrors.scala): three messages share the id tuple.false.
    {"tuple.false", R"(Termination measure might not decrease or might not be bounded\.)"},
    {"tuple.false", R"(Termination measure might not decrease\.)"},
    {"tuple.false", R"(Termination measure might not bounded\.)"},
    {"tuple.condition.false", R"(Required tuple condition might not hold\.)"},
    {"termination.condition.false", R"(Required termination condition might not hold\.)"},
    {"tuple.false", R"(Termination measure might not (?:decrease or might not be bounded|decrease|bounded)\.)"},
    {"tuple.condition.false", R"(Required tuple condition might not hold\.)"},
    {"termination.condition.false", R"(Required termination condition might not hold\.)"},
};

struct CompiledKind {
  std::regex re;
  const ErrorKind* kind;
};
const std::vector<CompiledKind>& error_regexes() {
  static const std::vector<CompiledKind> v = [] {
    std::vector<CompiledKind> out;
    for (const ErrorKind& k : kErrorKinds)
      out.push_back({std::regex(std::string(k.prefix) + R"(\s+([\s\S]*))"), &k});
    return out;
  }();
  return v;
}
const std::vector<std::pair<const ReasonKind*, std::regex>>& reason_regexes() {
  static const std::vector<std::pair<const ReasonKind*, std::regex>> v = [] {
    std::vector<std::pair<const ReasonKind*, std::regex>> out;
    for (const ReasonKind& r : kReasons) out.emplace_back(&r, std::regex(r.pattern));
    return out;
  }();
  return v;
}

std::string strip(const std::string& s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \t\r\n");
  return s.substr(a, b - a + 1);
}

std::string one_line(const std::string& s) {
  std::istringstream in(s);
  std::string word, out;
  while (in >> word) {
    if (!out.empty()) out += ' ';
    out += word;
  }
  return out;
}

}  // namespace

SiliconError classify_silicon_message(const std::string& entry) {
  SiliconError e;
  e.viper_id = e.category = e.reason = "unknown";
  e.message = one_line(entry);
  std::string message = entry;
  static const std::regex position(R"(\s*\(([^()@]+)@(\d+)\.(\d+)(?:--(\d+)\.(\d+))?\)\s*$)");
  std::smatch pm;
  if (std::regex_search(message, pm, position)) {
    e.file = pm[1].str();
    e.line = std::atoi(pm[2].str().c_str());
    message = message.substr(0, static_cast<size_t>(pm.position(0)));
  }
  message = strip(message);
  for (const CompiledKind& k : error_regexes()) {
    std::smatch m;
    if (!std::regex_match(message, m, k.re)) continue;
    e.viper_id = k.kind->id;
    e.category = k.kind->category;
    const std::string rest = m[1].str();
    static const std::regex timed_out(R"(\s*An SMT query timed out after \d+ms\.$)");
    if (std::regex_search(rest, timed_out)) {
      e.reason = "timeout";
      return e;
    }
    for (const auto& [reason, re] : reason_regexes()) {
      if (std::regex_match(rest, re)) {
        e.reason = reason->id;
        return e;
      }
    }
    return e;  // reason "unknown"
  }
  return e;  // viper_id and category "unknown"
}

SiliconVerdict parse_silicon_output(const std::string& output, int exit_code) {
  std::vector<std::string> entries;
  static const std::regex entry_re(R"(^\s*\[\s*(\d+)\] (.*)$)");
  static const std::regex banner_re(R"(^Silicon \S+( \(.*\))?\s*$)");
  static const std::regex success_line_re(R"(^Silicon finished verification successfully\b.*$)");
  static const std::regex found_line_re(R"(^Silicon found (\d+) errors? in .*:\s*$)");
  std::istringstream in(output);
  std::string raw;
  std::string stray;  // the first line that is neither part of a verdict nor of an error message
  int banners = 0, successes = 0, founds = 0;
  std::string found_count;
  while (std::getline(in, raw)) {
    if (!raw.empty() && raw.back() == '\r') raw.pop_back();
    std::smatch m;
    if (std::regex_match(raw, m, entry_re)) {
      entries.push_back(m[2].str());
    } else if (strip(raw).empty()) {
      continue;
    } else if (std::regex_match(raw, banner_re) && !std::regex_match(raw, success_line_re) && !std::regex_match(raw, found_line_re)) {
      ++banners;
    } else if (std::regex_match(raw, success_line_re)) {
      ++successes;
    } else if (std::regex_match(raw, m, found_line_re)) {
      ++founds;
      found_count = m[1].str();
    } else if (!entries.empty()) {
      entries.back() += "\n" + strip(raw);  // the rest of a multi-line message
    } else if (stray.empty()) {
      stray = raw;
    }
  }
  SiliconVerdict v;
  for (const std::string& e : entries) v.errors.push_back(classify_silicon_message(e));

  // A verdict is accepted only if the output is the banner, one verdict line and its error list, nothing else
  // (decision 0011 item 11): a warning, a stack trace or a second verdict means something else happened.
  if (successes + founds == 0) {
    v.problem = "Silicon did not produce a verdict (exit status " + std::to_string(exit_code) + ")";
    return v;
  }
  if (!stray.empty() || banners != 1 || successes + founds != 1) {
    v.problem = "Silicon's output is not a banner and one verdict" +
                (stray.empty() ? std::string() : " (unexpected line: " + stray.substr(0, 120) + ")") + " (exit status " +
                std::to_string(exit_code) + ")";
    return v;
  }
  if (successes == 1 && entries.empty() && exit_code == 0) {
    v.verified = true;
    return v;
  }
  if (founds != 1 || exit_code != 1) {
    v.problem = "Silicon did not produce a verdict (exit status " + std::to_string(exit_code) + ")";
  } else if (std::atoi(found_count.c_str()) != static_cast<int>(entries.size())) {
    v.problem = "Silicon reported " + found_count + " errors but " + std::to_string(entries.size()) + " were parsed";
  }
  return v;
}

}  // namespace cerastes
