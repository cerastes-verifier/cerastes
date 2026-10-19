// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "selftest.h"

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "check.h"
#include "emit.h"
#include "lower.h"
#include "pipeline.h"
#include "report.h"
#include "silicon_output.h"
#include "vir.h"
#include "verify_config.h"

namespace cerastes {
namespace {

namespace fs = std::filesystem;

struct Checks {
  int total = 0;
  int failed = 0;
  void expect(bool ok, const std::string& what) {
    ++total;
    if (!ok) {
      ++failed;
      std::fprintf(stderr, "selftest: FAIL: %s\n", what.c_str());
    }
  }
  template <class A, class B>
  void eq(const A& got, const B& want, const std::string& what) {
    ++total;
    if (!(got == want)) {
      ++failed;
      std::fprintf(stderr, "selftest: FAIL: %s\n", what.c_str());
    }
  }
};

void touch(const fs::path& p) {
  fs::create_directories(p.parent_path());
  std::ofstream(p) << "// x\n";
}

// ---- 0011 item 4: system files by canonical path ---------------------------------------------------------

void test_paths(Checks& c) {
  std::error_code ec;
  fs::path tmp = fs::temp_directory_path(ec);
  std::string templ = (tmp / "cerastes-selftest-XXXXXX").string();
  std::vector<char> buf(templ.begin(), templ.end());
  buf.push_back('\0');
  if (::mkdtemp(buf.data()) == nullptr) {
    c.expect(false, "cannot create a temporary directory");
    return;
  }
  const fs::path base = canonical_path(buf.data());
  struct Cleanup {
    fs::path p;
    ~Cleanup() { std::error_code e; fs::remove_all(p, e); }
  } cleanup{base};

  touch(base / "sys/inc/a.h");
  touch(base / "sys/inc/sub/b.h");
  touch(base / "sys/inc-evil/d.h");
  touch(base / "sys/inc2/e.h");
  touch(base / "user/c.h");
  touch(base / "cer/include/cerastes.h");
  touch(base / "cer/include/other.h");
  fs::create_symlink("../sys/inc/a.h", base / "user/link.h");
  fs::create_directory_symlink("../sys/inc", base / "user/linkdir");
  fs::create_symlink("../cer/include/cerastes.h", base / "user/ch.h");
  fs::create_symlink("../cer/include/other.h", base / "user/oh.h");

  const std::string sys = (base / "sys/inc").string();
  const std::string header = (base / "cer/include/cerastes.h").string();
  const std::vector<std::string> dirs = system_directories({sys, "/usr/local/include", (base / "nowhere").string()}, {});
  c.eq(dirs, std::vector<std::string>{sys}, "system_directories drops /usr/local/include and a missing directory");
  auto sysfile = [&](const fs::path& p) { return is_system_file(dirs, header, p.string()); };

  c.expect(sysfile(base / "sys/inc/a.h"), "a header under a system directory is a system file");
  c.expect(sysfile(base / "sys/inc/sub/b.h"), "a header below a subdirectory of a system directory is a system file");
  c.expect(!sysfile(base / "sys/inc-evil/d.h"), "a sibling directory with the same prefix is not a system directory");
  c.expect(!sysfile(base / "sys/inc2/e.h"), "inc2 is not under inc");
  c.expect(!sysfile(base / "user/c.h"), "a user header is not a system file");
  c.expect(sysfile(base / "user/../sys/inc/a.h"), "`..` is resolved: user/../sys/inc/a.h is a system file");
  c.expect(!sysfile(base / "sys/inc/../../user/c.h"), "a path that starts in a system directory and leaves it by `..` is a user file");
  c.expect(!sysfile(base / "sys/inc/../inc-evil/d.h"), "sys/inc/../inc-evil/d.h is not under sys/inc");
  c.expect(sysfile(base / "user/link.h"), "a symbolic link to a system file is a system file (the physical path decides)");
  c.expect(sysfile(base / "user/linkdir/a.h"), "a file reached through a symbolic link to a system directory is a system file");
  c.expect(!sysfile(base / "does/not/exist.h"), "a file that does not exist is not a system file");
  c.expect(sysfile(base / "cer/include/cerastes.h"), "exactly include/cerastes.h is a system file");
  c.expect(sysfile(base / "cer/include/../include/cerastes.h"), "include/cerastes.h through `..` is a system file");
  c.expect(sysfile(base / "user/ch.h"), "a symbolic link to include/cerastes.h is that file");
  c.expect(!sysfile(base / "cer/include/other.h"), "the other files of cerastes's include directory are user files");
  c.expect(!sysfile(base / "user/oh.h"), "a symbolic link to another file of include/ is a user file");

  // A user -I directory that is also a system directory makes it a user directory.
  c.eq(system_directories({sys}, {sys}).size(), static_cast<size_t>(0), "a -I directory named as a system directory is a user directory");
  c.eq(system_directories({sys}, {(base / "user/linkdir").string()}).size(), static_cast<size_t>(0), "...also through a symbolic link");
  c.expect(!path_is_under("/a/bc", "/a/b"), "path_is_under compares whole components");
  c.expect(path_is_under("/a/b/c", "/a/b"), "path_is_under: below");
  c.expect(!path_is_under("/a/b", "/a/b"), "path_is_under: the directory itself is not below it");
  c.expect(path_is_under("/a", "/"), "path_is_under: below the root");
}

// ---- 0011 item 11: Silicon's output ----------------------------------------------------------------------

void test_silicon_output(Checks& c) {
  {
    SiliconVerdict v = parse_silicon_output("Silicon 1.1-SNAPSHOT (6b540aa4@(detached))\nSilicon finished verification successfully in 1.2s.\n", 0);
    c.expect(v.verified && v.errors.empty() && v.problem.empty(), "a success line with exit status 0 is verified");
  }
  {
    SiliconVerdict v = parse_silicon_output("Silicon finished verification successfully in 1.2s.\n", 1);
    c.expect(!v.verified && !v.problem.empty(), "a success line with exit status 1 is a problem");
  }
  {
    const char* out =
        "Silicon 1.1-SNAPSHOT (6b540aa4@(detached))\n"
        "Silicon found 1 error in 4.92s:\n"
        "  [0] Assert might fail. Assertion !(a + b < -1 || 1 < a + b) might not hold. (program.vpr@15.10--15.142)\n";
    SiliconVerdict v = parse_silicon_output(out, 1);
    c.expect(!v.verified && v.problem.empty() && v.errors.size() == 1, "one error is a verdict");
    if (v.errors.size() == 1) {
      const SiliconError& e = v.errors[0];
      c.eq(e.viper_id, std::string("assert.failed"), "kind of an assert");
      c.eq(e.reason, std::string("assertion.false"), "reason of an assert");
      c.eq(e.file, std::string("program.vpr"), "file of the position");
      c.eq(e.line, 15, "line of the position");
    }
  }
  {
    const char* out =
        "Silicon 1.1-SNAPSHOT (6b540aa4@(detached))\n"
        "Silicon found 4 errors in 4.92s:\n"
        "  [0] The precondition of method f_g__1 might not hold. Assertion a > 0 might not hold.\n"
        "      (program.vpr@3.4--3.20)\n"
        "  [1] Loop invariant x >= 0 might not hold on entry. Assertion x >= 0 might not hold. (program.vpr@9.1)\n"
        "  [2] Postcondition of f_f__0 might not hold. Assertion res > 0 might not hold. (program.vpr@4.3--4.9)\n"
        "  [3] Assert might fail. Divisor b might be zero. (program.vpr@7.5--7.12)\n";
    SiliconVerdict v = parse_silicon_output(out, 1);
    c.expect(v.problem.empty() && v.errors.size() == 4, "four errors, the first continued on a second line");
    if (v.errors.size() == 4) {
      c.eq(v.errors[0].viper_id, std::string("call.precondition"), "precondition kind");
      c.eq(v.errors[0].reason, std::string("assertion.false"), "a continued message keeps its reason");
      c.eq(v.errors[0].line, 3, "a continued message keeps its position");
      c.eq(v.errors[1].viper_id, std::string("invariant.not.established"), "invariant entry kind");
      c.eq(v.errors[1].line, 9, "a position without an end");
      c.eq(v.errors[2].viper_id, std::string("postcondition.violated"), "postcondition kind");
      c.eq(v.errors[3].reason, std::string("division.by.zero"), "division by zero reason");
    }
  }
  {
    SiliconVerdict v = parse_silicon_output(
        "Silicon 1.1-SNAPSHOT (6b540aa4@(detached))\nSilicon found 1 error in 1s:\n  [0] Assert might fail. An SMT query timed out after 5000ms. (program.vpr@2.1--2.9)\n", 1);
    c.expect(v.errors.size() == 1 && v.errors[0].reason == "timeout", "a solver timeout has reason timeout");
  }
  {
    SiliconVerdict v = parse_silicon_output(
        "Silicon 1.1-SNAPSHOT (6b540aa4@(detached))\nSilicon found 1 error in 1s:\n  [0] Parse error: something (program.vpr@2.1)\n", 1);
    c.expect(v.errors.size() == 1 && v.errors[0].viper_id == "unknown" && v.errors[0].reason == "unknown",
             "a message that is not a verification error is unknown");
  }
  {
    SiliconVerdict v = parse_silicon_output("Silicon 1.1-SNAPSHOT (x)\nSilicon found 2 errors in 1s:\n  [0] Assert might fail. Assertion x might not hold. (program.vpr@2.1)\n", 1);
    c.expect(!v.problem.empty(), "a count that does not match the entries is a problem");
  }
  {
    SiliconVerdict v = parse_silicon_output("Exception in thread \"main\" java.lang.OutOfMemoryError\n", 1);
    c.expect(!v.verified && !v.problem.empty(), "a crash is not a verdict");
  }
  {
    // Anything besides the banner, the verdict and its error list is not a verdict (decision 0011 item 11).
    const char* clean = "Silicon 1.1-SNAPSHOT (6b540aa4@(detached))\nSilicon finished verification successfully in 1.2s.\n";
    c.expect(parse_silicon_output(clean, 0).verified, "the banner and a success line are a verdict");
    c.expect(!parse_silicon_output(std::string("WARNING: something\n") + clean, 0).verified, "a stray line before the banner is not a verdict");
    c.expect(!parse_silicon_output(std::string(clean) + "Exception in thread main\n", 0).verified, "a stray line after the verdict is not a verdict");
    c.expect(!parse_silicon_output(std::string(clean) + "Silicon finished verification successfully in 1.2s.\n", 0).verified, "two verdicts are not a verdict");
    c.expect(!parse_silicon_output("Silicon finished verification successfully in 1.2s.\n", 0).verified, "a verdict without the banner is not a verdict");
    c.expect(!parse_silicon_output("Silicon 1.1-SNAPSHOT (x)\n", 0).verified, "a banner without a verdict is not a verdict");
  }
  {
    SiliconVerdict v = parse_silicon_output("", 0);
    c.expect(!v.verified && !v.problem.empty(), "no output is not a verdict");
  }
}

// ---- the position map and the mapping of errors ---------------------------------------------------------

void test_report(Checks& c) {
  const std::string map =
      "7\tnot.wellformed\tinternal\tt.c\t1\t3\tinternal check: well-formedness of the contract of `f`\n"
      "9\tpostcondition.violated\tinternal\tt.c\t1\t3\tinternal check: Silicon's own check of the postcondition of `f`\n"
      "15\tassert.failed\toverflow\tt.c\t6\t3\tsigned `int` addition may overflow\n"
      "20\tassert.failed\tpostcondition\tt.c\t12\t3\tthe postcondition may not hold\tt.c:10:this clause may not hold\n"
      "30\tcall.precondition\tprecondition\th.h\t4\t1\tthe precondition of `g` may not hold\th.h:2:the clause\n"
      "30\tassert.failed\tcheck\tt.c\t3\t1\tcheck may not hold\n";
  std::string problem;
  std::vector<MapRow> rows = parse_position_map(map, &problem);
  c.expect(problem.empty() && rows.size() == 6, "a well-formed position map parses");
  if (rows.size() == 6) {
    c.expect(rows[3].has_note && rows[3].note_file == "t.c" && rows[3].note_line == 10 && rows[3].note_text == "this clause may not hold",
             "a note is read");
    c.eq(rows[4].file, std::string("h.h"), "a header's file");
  }
  parse_position_map("7\tnot.wellformed\n", &problem);
  c.expect(!problem.empty(), "a short map line is a problem");

  auto err = [](const char* id, int line, const char* reason = "assertion.false") {
    SiliconError e;
    e.viper_id = id;
    e.reason = reason;
    e.file = "program.vpr";
    e.line = line;
    e.message = std::string(id) + " at " + std::to_string(line);
    return e;
  };
  {
    SiliconVerdict v;
    v.errors = {err("assert.failed", 20), err("call.precondition", 30), err("assert.failed", 15)};
    MappedVerdict m = map_errors(v, rows, "program.vpr");
    c.expect(m.failures.empty() && m.errors.size() == 3, "three errors map");
    if (m.errors.size() == 3) {
      // Sorted by file, line, category: h.h, t.c:6, t.c:12.
      c.eq(render_error(m.errors[0]), std::string("h.h:4: error: precondition: the precondition of `g` may not hold\nh.h:2: note: the clause\n"),
           "a precondition error and its note, sorted first by file");
      c.eq(render_error(m.errors[1]), std::string("t.c:6: error: overflow: signed `int` addition may overflow\n"), "an overflow error");
      c.eq(render_error(m.errors[2]), std::string("t.c:12: error: postcondition: the postcondition may not hold\nt.c:10: note: this clause may not hold\n"),
           "a postcondition error and its note");
    }
  }
  {
    SiliconVerdict v;
    v.errors = {err("assert.failed", 15), err("assert.failed", 15)};
    MappedVerdict m = map_errors(v, rows, "program.vpr");
    c.expect(m.failures.empty() && m.errors.size() == 1, "an error reported twice is printed once");
  }
  {
    SiliconVerdict v;
    v.errors = {err("postcondition.violated", 9)};
    MappedVerdict m = map_errors(v, rows, "program.vpr");
    c.expect(m.errors.empty() && m.failures.size() == 1, "an internal check that fails is a tool failure, not an error");
  }
  {
    SiliconVerdict v;
    v.errors = {err("assert.failed", 99)};
    MappedVerdict m = map_errors(v, rows, "program.vpr");
    c.expect(m.errors.empty() && m.failures.size() == 1, "an error the map does not explain is a tool failure");
  }
  {
    SiliconVerdict v;
    v.errors = {err("assert.failed", 15)};  // line 15 has an assert.failed entry; ask for another kind
    v.errors[0].viper_id = "call.precondition";
    MappedVerdict m = map_errors(v, rows, "program.vpr");
    c.expect(m.errors.empty() && m.failures.size() == 1, "an error of another kind at a mapped line is a tool failure");
  }
  {
    SiliconVerdict v;
    v.errors = {err("assert.failed", 15, "timeout")};
    MappedVerdict m = map_errors(v, rows, "program.vpr");
    c.expect(m.errors.empty() && m.failures.size() == 1, "a solver timeout is a tool failure");
  }
  {
    SiliconVerdict v;
    v.errors = {err("assert.failed", 15)};
    v.errors[0].file = "other.vpr";
    MappedVerdict m = map_errors(v, rows, "program.vpr");
    c.expect(m.errors.empty() && m.failures.size() == 1, "a position in another file is a tool failure");
  }
  {
    SiliconVerdict v;
    v.errors = {err("assert.failed", 15, "division.by.zero")};
    MappedVerdict m = map_errors(v, rows, "program.vpr");
    c.expect(m.errors.empty() && m.failures.size() == 1, "an assertion error whose reason is not assertion.false is a tool failure");
  }
  {
    SiliconVerdict v;
    v.errors = {err("assert.failed", 15, "insufficient.permission")};
    MappedVerdict m = map_errors(v, rows, "program.vpr");
    c.expect(m.errors.empty() && m.failures.size() == 1, "an insufficient-permission error is a tool failure");
  }
  {
    SiliconVerdict v;
    v.problem = "Silicon did not produce a verdict (exit status 7)";
    MappedVerdict m = map_errors(v, rows, "program.vpr");
    c.expect(m.errors.empty() && m.failures.size() == 1, "a verdict problem is a tool failure");
  }
}

// ---- a whole pipeline: VIR to Viper to Silicon to the verdict ------------------------------------------------

struct Built {
  std::string vpr;
  std::string map;
  bool ok = false;
};

// `int f(int a) requires 0 <= a < 100 ensures result == a + 1 { return a + 1; }` and, when `with_g`,
// `int g(int a) { return a + 1; }`, which may overflow.
Built build_program(bool with_g) {
  using namespace vir;
  Program p;
  Builder b(p);
  const IntType* it = p.int_type("int", 32, true);
  const Type T = Type::integer(it);
  auto loc = [](int line, int col = 3) { return SrcLoc{"t.c", line, col}; };

  Function* f = p.add_function("f", T, loc(1), loc(5));
  f->add_param("a", T, loc(1));
  const Expr* a0 = b.formal(*f, 0, loc(2));
  f->requires_.push_back({b.binary(BinaryOp::LAnd, b.binary(BinaryOp::Ge, a0, b.int_const(T, "0", loc(2)), loc(2)),
                                   b.binary(BinaryOp::Lt, b.formal(*f, 0, loc(2)), b.int_const(T, "100", loc(2)), loc(2)), loc(2)),
                          loc(2)});
  f->ensures_.push_back({b.binary(BinaryOp::Eq, b.result(*f, loc(3)),
                                  b.binary(BinaryOp::Add, b.formal(*f, 0, loc(3)), b.int_const(T, "1", loc(3)), loc(3)), loc(3)),
                         loc(3)});
  f->body = b.block({b.return_stmt(b.binary(BinaryOp::Add, b.local(f->param_var(0), loc(4)), b.int_const(T, "1", loc(4)), loc(4)), loc(4))},
                    loc(1));
  if (with_g) {
    Function* g = p.add_function("g", T, loc(7), loc(10));
    g->add_param("a", T, loc(7));
    g->body = b.block({b.return_stmt(b.binary(BinaryOp::Add, b.local(g->param_var(0), loc(9)), b.int_const(T, "1", loc(9)), loc(9)), loc(9))},
                      loc(7));
  }
  Built out;
  if (!check_program(p).empty()) return out;
  LowerResult lr = lower_program(p);
  if (!lr.errors.empty()) return out;
  viper::Emitted e = viper::emit(lr.program);
  out.vpr = e.vpr;
  out.map = e.map;
  out.ok = true;
  return out;
}

// `void f(int n) requires n >= 1 { int i = 0; while (i < n) decreases n - i {} }`: the measure never decreases.  Silicon's
// termination check must find it, so that a termination plugin that is switched off cannot make results vacuous
// (decision 0013 item 8).
Built build_nonterminating() {
  using namespace vir;
  Program p;
  Builder b(p);
  const IntType* it = p.int_type("int", 32, true);
  const IntType* lt = p.int_type("long", 64, true);
  const Type T = Type::integer(it);
  const Type L = Type::integer(lt);
  auto loc = [](int line, int col = 3) { return SrcLoc{"t.c", line, col}; };
  Function* f = p.add_function("f", Type::void_type(), loc(1), loc(8));
  f->add_param("n", T, loc(1));
  f->requires_.push_back({b.binary(BinaryOp::Ge, b.formal(*f, 0, loc(2)), b.int_const(T, "1", loc(2)), loc(2)), loc(2)});
  Var* i = f->new_local("i", T, loc(3));
  const Expr* gap = b.binary(BinaryOp::Sub, b.convert(L, b.local(f->param_var(0), loc(5)), false, loc(5)),
                             b.convert(L, b.local(i, loc(5)), false, loc(5)), loc(5));
  f->body = b.block({b.decl(i, b.int_const(T, "0", loc(3)), loc(3)),
                     b.while_stmt(b.binary(BinaryOp::Lt, b.local(i, loc(4)), b.local(f->param_var(0), loc(4)), loc(4)), {},
                                  {Clause{gap, loc(5)}}, b.block({}, loc(6)), loc(4))},
                    loc(1));
  Built out;
  if (!check_program(p).empty()) return out;
  LowerResult lr = lower_program(p);
  if (!lr.errors.empty()) return out;
  viper::Emitted e = viper::emit(lr.program);
  out.vpr = e.vpr;
  out.map = e.map;
  out.ok = true;
  return out;
}

void test_pipeline(Checks& c, const SiliconTools& tools) {
  PipelineOptions po;
  po.tools = tools;
  po.timeout_seconds = 300;
  po.primary_file = "t.c";
  po.functions = 1;
  {
    Built b = build_program(false);
    c.expect(b.ok, "the verified program builds");
    if (b.ok) {
      PipelineResult r = run_pipeline(po, b.vpr, b.map);
      c.eq(r.exit_code, 0, "a correct program verifies (exit status)");
      c.eq(r.out, std::string("t.c: verified (1 function; one translation unit: assumes that functions defined elsewhere satisfy the contracts declared here)\n"),
           "the verdict line");
      c.eq(r.err, std::string(""), "nothing on standard error");
      PipelineOptions timeout = po;
      timeout.timeout_seconds = 0;
      PipelineResult t = run_pipeline(timeout, b.vpr, b.map);
      c.eq(t.exit_code, 3, "a timeout of zero is a tool failure");
      c.expect(t.out.empty() && t.err.find("did not finish") != std::string::npos, "the timeout is named, and nothing is claimed");
      PipelineOptions broken = po;
      broken.tools.java = "/nonexistent/java";
      PipelineResult m = run_pipeline(broken, b.vpr, b.map);
      c.eq(m.exit_code, 3, "a java that cannot be run is a tool failure");
    }
  }
  {
    Built b = build_program(true);
    c.expect(b.ok, "the failing program builds");
    if (b.ok) {
      po.functions = 2;
      PipelineResult r = run_pipeline(po, b.vpr, b.map);
      c.eq(r.exit_code, 1, "g's overflow is a verification error (exit status)");
      c.eq(r.out, std::string("t.c:9: error: overflow: signed `int` addition may overflow\n"), "the error is at g's line");
      c.eq(r.err, std::string(""), "nothing on standard error");
    }
  }
  {
    Built b = build_nonterminating();
    c.expect(b.ok, "the non-terminating program builds");
    if (b.ok) {
      po.functions = 1;
      PipelineResult r = run_pipeline(po, b.vpr, b.map);
      c.eq(r.exit_code, 1, "a loop whose measure does not decrease is a verification error (exit status)");
      c.expect(r.out.rfind("t.c:5: error: termination: ", 0) == 0, "the termination error is at the measure's line: " + r.out);
      c.eq(r.err, std::string(""), "nothing on standard error");
    }
  }
}

}  // namespace

int run_selftest(const SiliconTools& tools) {
  Checks c;
  test_paths(c);
  test_silicon_output(c);
  test_report(c);
  test_pipeline(c, tools);
  if (c.failed == 0) {
    std::printf("selftest: %d checks passed\n", c.total);
    return 0;
  }
  std::printf("selftest: %d of %d checks FAILED\n", c.failed, c.total);
  return 1;
}

}  // namespace cerastes
