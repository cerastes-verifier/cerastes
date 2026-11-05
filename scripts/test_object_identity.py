#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Self-test for scripts/object_identity.py (session S7, decision 0051): a twin for the harness itself.

25 compile cases and 18 token cases, plus the guard tests (31 tests in all).

Compile cases.  Each is one translation unit of a fixture tree, written in a "before" and an "after" form; the tool
compares the two trees in one run and its per-TU verdicts are checked.
  * Must pass (the edits the restyle makes): whitespace; braces around statements; `U` on a `std::size_t` compare
    and on a shift count; brace initialization (scalar and aggregate); split declarators; `T* p`; a macro that embeds
    `__FILE__`/`__LINE__` of EDG's `assertion_failed` with a blank line above ("identical modulo lines": the objects
    differ by the line immediate and are not compared, the case asserts that); a library template instantiated for
    another literal type (counted noise, not a failure under libc++).
  * Must fail, and which pass catches it: `obj` (object bytes and IR differ): a changed literal, `x - 1 < 0` with
    `1U`, `vector v(3, 0)` against `{3, 0}`, an `initializer_list` constructor taken by brace-init, an `explicit`
    constructor only direct-list-initialization may pick (`W(long)` against `explicit W(int)`), an `else` rebound,
    a backslash-continued comment, `log_line(__FILE__, __LINE__)` and an `assertion_failed` call's other argument
    (the line normalisation is for `assertion_failed`'s line argument only), a non-own-file line;
    `ir` only (machine code identical): `int y = x - 1U;` (`sub` against `sub nsw`), so an objects-only tool fails;
    `ext` only (-femit-all-decls IR): an unused inline function changed or removed, so a tool without that pass fails;
    a header edit seen only when the header is read from the side's own tree.

Token cases (`--tokens none|braces --no-compile`): a comment and whitespace change and a splice added inside a
directive pass under both modes; inserted braces (a statement, an else chain, nested ifs) pass under `braces` and
fail under `none`; an inserted scope, an unclosed brace and braces with another change fail under both; a dropped token
in a `static_assert`, a changed token in an `#if 0` and in an inactive branch, a token moved into or before a directive,
`= delete` dropped, an access specifier, `> >` against `>>` (the lexer does not split `>>`), a swallowed-comment line, a
changed CMakeLists.txt and an added file fail under both.

Guards (each exits 2 where it must): a source naming its header by the working tree's absolute path; a header of
the other side's export; a header outside the root and the toolchain (another checkout), found by absolute path and
by an explicit -I; system headers allowed; an unlisted src .cpp; wrong TU-count constants; a changed unread header,
CMakeLists.txt, cmake/ file or `.in` file (allowed only by --allow-uncovered, include/cerastes.h by default); `--tu`;
a compile failure is not a pass; the same tree exported twice into two directories; the token pass beside the
compile passes; `--no-compile` needs a token mode; the command rewrite, the line normaliser, the tokenizer and the
dependency-file parser on their own.

The compiler is $CXX, `c++` or `clang++`, else $CERASTES_CLANG_CXX or build/env.sh's CERASTES_CLANG_CXX
(check.sh does not source env.sh, and on Linux `c++` is GCC and clang is only clang++-21): the first that is
clang.  No clang at all is a failure, not a skip, so CI cannot pass without running the cases.  No EDG build
is needed.
"""

import contextlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import object_identity as oi  # noqa: E402


def env_sh_clang():
    """CERASTES_CLANG_CXX as bootstrap.sh wrote it into build/env.sh, or None."""
    env = Path(__file__).resolve().parent.parent / "build" / "env.sh"
    try:
        for line in env.read_text().splitlines():
            if line.startswith("export CERASTES_CLANG_CXX="):
                return line.split("=", 1)[1].strip().strip('"') or None
    except OSError:
        pass
    return None


def find_clang():
    # $CXX, `c++` and `clang++` first: on macOS /usr/bin/c++ is the xcrun shim that adds the SDK, which the
    # Command Line Tools' clang++ named by env.sh lacks; on Linux `c++` is GCC and only clang++-21 exists.
    for c in (os.environ.get("CXX"), "c++", "clang++", os.environ.get("CERASTES_CLANG_CXX"), env_sh_clang()):
        if not c:
            continue
        exe = shutil.which(c)
        if exe and oi.compiler_is_clang(exe)[0]:
            return exe
    return None


CXX = find_clang()


def uses_libcxx():
    """The fixture's `std::string(1, c)` instantiates a different trait under libc++ only."""
    if CXX is None:
        return False
    r = subprocess.run([CXX, "-x", "c++", "-std=c++20", "-E", "-dM", "-include", "string", "-"], input="", text=True,
                       capture_output=True)
    return "_LIBCPP_VERSION" in r.stdout


LIBCXX = uses_libcxx()

# name: (before, after, expectation).  expectation: "same", "lines" (identical modulo __LINE__ of a macro use in the
# file itself), or the pass that must catch the difference, "obj" or "ir".
CASES = {}


def case(name, before, after, expect):
    CASES[name] = (before, after, expect)


# ---- must pass: what the restyle does
case("ws", "int f(int a){return a+1;}\n",
     "int f(int a)\n{\n    return a + 1;\n}\n\n\n", "same")
case("braces", "int f(int a) {\n  if (a) return 1;\n  for (int i = 0; i < a; ++i) a += i;\n  return 2;\n}\n",
     "int f(int a)\n{\n    if (a)\n    {\n        return 1;\n    }\n    for (int i = 0; i < a; ++i)\n    {\n"
     "        a += i;\n    }\n    return 2;\n}\n", "same")
case("size_t_U", "#include <cstddef>\nint f(std::size_t n) { return n == 0 ? 1 : 2; }\n",
     "#include <cstddef>\nint f(std::size_t n) { return n == 0U ? 1 : 2; }\n", "same")
case("shift_U", "unsigned f(unsigned a) { return a << 3; }\n", "unsigned f(unsigned a) { return a << 3U; }\n", "same")
case("brace_init", "int f() { int x = 5; return x; }\n", "int f() { int x {5}; return x; }\n", "same")
case("brace_init_agg", "struct P { int a; int b; };\nint f() { P p = {1, 2}; return p.a + p.b; }\n",
     "struct P { int a; int b; };\nint f() { P p {1, 2}; return p.a + p.b; }\n", "same")
case("declarators", "int f() { int a = 1, b = 2; return a + b; }\n",
     "int f() { int a = 1; int b = 2; return a + b; }\n", "same")
case("pointer_style", "int f(int *p, int &r) { return *p + r; }\n", "int f(int* p, int& r) { return *p + r; }\n",
     "same")
# a literal that changes type instantiates a different library template that nothing uses (the real case: `name.compare(0, n, p)`
# becoming `0U` instantiates libc++'s `__can_be_converted_to_string_view_v<char, traits, unsigned>`; the minimal
# form is `std::string(1, c)` against `std::string(1U, c)`): visible only
# with -femit-all-decls, a name and no behaviour, counted but not a failure
case("std_instantiation_name", "#include <string>\nstd::string f(char v) { return std::string(1, v); }\n",
     "#include <string>\nstd::string f(char v) { return std::string(1U, v); }\n", "noise")
# a macro that embeds __FILE__ and __LINE__ (as EDG's check_assertion does): a blank line above moves the line
ASSERTION_FAILED = ("extern void assertion_failed(const char*, int, const char*, const char*, const char*);\n"
                    "#define CHECK(c) ((c) ? (void)0 : assertion_failed(__FILE__, __LINE__, __func__, %s, nullptr))\n"
                    "int f(int a)\n{\n%sCHECK(a > 0);\n    return a;\n}\n")
HDR_MACRO = ASSERTION_FAILED % ("nullptr", "%s")
case("line_macro", HDR_MACRO % "", HDR_MACRO % "\n\n    // a comment\n", "lines")

# ---- must fail: what changes behaviour
case("literal", "int f() { return 41; }\n", "int f() { return 42; }\n", "obj")
case("sub_unsigned_compare", "bool f(int x) { return x - 1 < 0; }\n", "bool f(int x) { return x - 1U < 0; }\n", "obj")
case("sub_unsigned_alone", "int f(int x) { int y = x - 1; return y; }\n", "int f(int x) { int y = x - 1U; return y; }\n",
     "ir")
case("vector_braces", "#include <vector>\nint f() { std::vector<int> v(3, 0); return (int)v.size(); }\n",
     "#include <vector>\nint f() { std::vector<int> v{3, 0}; return (int)v.size(); }\n", "obj")
INIT_LIST = ("#include <initializer_list>\nstruct W {\n    W(int) : r(1) {}\n"
             "    explicit W(std::initializer_list<int>) : r(2) {}\n    int r;\n};\n")
case("explicit_ctor", INIT_LIST + "int f() { W w = 5; return w.r; }\n", INIT_LIST + "int f() { W w {5}; return w.r; }\n",
     "obj")
case("else_rebound", "int f(int a, int b) {\n  if (a)\n    if (b) return 1;\n    else return 2;\n  return 3;\n}\n",
     "int f(int a, int b)\n{\n    if (a)\n    {\n        if (b)\n        {\n            return 1;\n        }\n"
     "    }\n    else\n    {\n        return 2;\n    }\n    return 3;\n}\n", "obj")
case("comment_continuation", "int f() {\n  int x = 1; // set x \\\n  x = 2;\n  return x;\n}\n",
     "int f()\n{\n    int x = 1; // set x\n    x = 2;\n    return x;\n}\n", "obj")
# header-only code that no TU calls is compiled only under -femit-all-decls (an inline function nobody uses): a changed
# or removed one must be seen there
case("unused_inline_changed", "inline int unused() { return 1; }\nint f() { return 0; }\n",
     "inline int unused() { return 2; }\nint f() { return 0; }\n", "ext")
case("unused_inline_removed", "inline int unused() { return 1; }\nint f() { return 0; }\n",
     "int f() { return 0; }\n", "ext")
# the line argument of a call that does not name the file itself is data: its change must still be seen
case("line_not_own_file", "extern void fail(const char*, int);\nint f(int a) {\n  if (a < 0) fail(\"x.cpp\", __LINE__);\n"
     "  return a;\n}\n",
     "extern void fail(const char*, int);\n\nint f(int a) {\n  if (a < 0) fail(\"x.cpp\", __LINE__);\n  return a;\n}\n",
     "obj")
case("line_macro_and_literal", HDR_MACRO % "", (HDR_MACRO % "\n").replace("return a;", "return a + 1;"), "obj")
# the normalisation is for EDG's `assertion_failed` only: another callee that is given __FILE__ and __LINE__ must see
# its line change, and so must another argument of an `assertion_failed` call
LOG = ("extern void log_line(const char*, int);\n#define LOG() log_line(__FILE__, __LINE__)\n"
       "int f(int a)\n{\n%sLOG();\n    return a;\n}\n")
case("line_other_callee", LOG % "", LOG % "\n\n    // a comment\n", "obj")
MSG = ASSERTION_FAILED % ('"%s"', "%s")
case("line_other_argument", MSG % ("a", ""), MSG % ("b", "\n\n    // a comment\n"), "obj")
# an explicit constructor that only direct-list-initialization may pick (copy-initialization skips it)
EXPL = "struct W {\n    W(long) : r(1) {}\n    explicit W(int) : r(2) {}\n    int r;\n};\n"
case("explicit_only", EXPL + "int f() { W w = 5; return w.r; }\n", EXPL + "int f() { W w {5}; return w.r; }\n", "obj")
# the header edit is seen only if the header is read from the side's own tree (root/src/hdr.h holds the old text)
case("header_edit", '#include "hdr.h"\nint f() { return hv(); }\n', '#include "hdr.h"\nint f() { return hv(); }\n', "obj")

HEADERS = {"hdr.h": ("inline int hv() { return 1; }\n", "inline int hv() { return 2; }\n")}

N_FILES = len(CASES)


def write(p, text):
    p = Path(p)
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(text)


class Fixture:
    """root/ (the repository as the compile database knows it, with the OLD header), v1/ and v2/ (the sides)."""

    def __init__(self, tmp, cases=None, headers=None, extra_cmd=None):
        self.tmp = Path(tmp)
        self.root = self.tmp / "root"
        self.v1, self.v2 = self.tmp / "v1", self.tmp / "v2"
        self.scratch = self.tmp / "scratch"
        self.cc = self.tmp / "compile_commands.json"
        cases = CASES if cases is None else cases
        headers = HEADERS if headers is None else headers
        entries = []
        for name, (b, a, _) in cases.items():
            for side, text in ((self.v1, b), (self.v2, a)):
                write(side / "src" / f"{name}.cpp", text)
            self.entry(entries, name, extra_cmd)
        for h, (b, a) in headers.items():
            write(self.v1 / "src" / h, b)
            write(self.v2 / "src" / h, a)
            write(self.root / "src" / h, b)
        (self.root / "include").mkdir(parents=True, exist_ok=True)
        (self.v1 / "include").mkdir(exist_ok=True)
        (self.v2 / "include").mkdir(exist_ok=True)
        self.cc.write_text(json.dumps(entries))

    def entry(self, entries, name, extra_cmd=None):
        r = str(self.root.resolve()) if self.root.exists() else str(self.root)
        entries.append({
            "directory": str(self.root),
            "command": f"{CXX} -std=c++20 -I{self.root.resolve()}/src -I{self.root.resolve()}/include "
                       f"{extra_cmd or ''} -o x.o -c {self.root.resolve()}/src/{name}.cpp",
            "file": f"{self.root.resolve()}/src/{name}.cpp",
            "output": f"{r}/CMakeFiles/fx.dir/{name}.o",
        })

    def run(self, before=None, after=None, extra=(), n=None, files=None):
        out, err = io.StringIO(), io.StringIO()
        js = self.tmp / "result.json"
        argv = ["--before", str(before or self.v1), "--after", str(after or self.v2), "--root", str(self.root),
                "--compile-commands", str(self.cc), "--scratch", str(self.scratch), "--expected-tus",
                str(N_FILES if n is None else n), "--expected-files", str(N_FILES if files is None else files),
                "--json", str(js), "-j", str(min(8, os.cpu_count() or 4)), *extra]
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = oi.main(argv)
        res = {}
        if js.exists():
            res = {e["tu"]: e for e in json.loads(js.read_text())["tus"]}
        return rc, res, out.getvalue(), err.getvalue()


class CompilerFound(unittest.TestCase):
    def test_a_clang_is_found(self):
        self.assertIsNotNone(CXX, "no clang++ found ($CXX, c++, clang++, $CERASTES_CLANG_CXX, build/env.sh): "
                                  "run scripts/bootstrap.sh")


@unittest.skipIf(CXX is None, "no clang++ found ($CXX, c++, clang++, $CERASTES_CLANG_CXX, build/env.sh)")
class ObjectIdentityCases(unittest.TestCase):
    """One run of the tool over every case's before/after trees."""

    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.fx = Fixture(cls.tmp.name)
        cls.rc, cls.res, cls.out, cls.err = cls.fx.run()

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def verdict(self, name):
        return self.res[f"fx/src/{name}.cpp"]

    def test_run_found_every_case(self):
        self.assertEqual(len(self.res), N_FILES, self.out + self.err)
        # header_edit is a failing case: the run as a whole reports a difference (1), not an invalid proof (2)
        self.assertEqual(self.rc, 1, self.out + self.err)

    def test_every_case(self):
        for name, (_, _, expect) in CASES.items():
            if name == "header_edit":
                continue
            with self.subTest(case=name):
                e = self.verdict(name)
                if expect == "same":
                    self.assertEqual(e["verdict"], "identical", e)
                elif expect == "noise":
                    self.assertEqual(e["verdict"], "identical", e)
                    if LIBCXX:
                        self.assertGreaterEqual(e["ext_noise"], 1, "the -femit-all-decls pass should have seen the "
                                                f"instantiation of the other type: {e}")
                elif expect == "ext":
                    # the plain IR and the object cannot see it, only -femit-all-decls
                    self.assertEqual(e["verdict"], "DIFFERS", e)
                    self.assertTrue(e["ir"], e)
                    self.assertTrue(e["obj"], e)
                    self.assertFalse(e["ir_ext"], e)
                elif expect == "lines":
                    self.assertEqual(e["verdict"], "identical-modulo-lines", e)
                    self.assertGreaterEqual(e["line_sites"], 1)
                    # the objects differ by the line immediates: they are NOT compared, and the report says so
                    self.assertFalse(e["obj_compared"], e)
                    self.assertFalse(e["obj"], e)
                else:
                    self.assertEqual(e["verdict"], "DIFFERS", e)
                    # which pass catches it: the object bytes, or only the IR
                    self.assertFalse(e["ir"], f"{name}: the IR comparison must catch it: {e}")
                    if expect == "obj":
                        self.assertFalse(e["obj"], f"{name}: the object bytes are expected to differ too: {e}")
                    else:
                        self.assertTrue(e["obj"], f"{name}: expected identical machine code, only the IR differs: {e}")

    def test_header_edit_needs_the_sides_own_header(self):
        # hdr.h is 1 in root/src and in v1, 2 in v2: a tool that read root/src/hdr.h for both would agree
        e = self.verdict("header_edit")
        self.assertEqual(e["verdict"], "DIFFERS", e)
        self.assertFalse(e["ir"])

    def test_ir_only_cases_exist(self):
        # the point of the IR pass: weakening the tool to objects only loses these
        self.assertTrue(any(x[2] == "ir" for x in CASES.values()))


@unittest.skipIf(CXX is None, "no clang++ found")
class ObjectIdentityGuards(unittest.TestCase):
    def fixture(self, cases, headers=None, **kw):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        return Fixture(tmp.name, cases, headers if headers is not None else {}, **kw)

    def test_same_tree_exported_twice_is_identical(self):
        fx = self.fixture({k: CASES[k] for k in ("ws", "brace_init", "line_macro", "header_edit")}, HEADERS)
        copy = Path(fx.tmp) / "v1copy"
        shutil.copytree(fx.v1, copy)
        rc, res, out, err = fx.run(before=fx.v1, after=copy, n=4, files=4)
        self.assertEqual(rc, 0, out + err)
        self.assertEqual({e["verdict"] for e in res.values()}, {"identical"})
        # two exports in two directories: their trees are different paths
        self.assertNotEqual(next(fx.scratch.glob("before-*")), next(fx.scratch.glob("after-*")))

    def test_absolute_include_of_the_working_tree_is_caught(self):
        # the source names its header by absolute path: no rewrite of the command can redirect it, only the
        # dependency check sees that both sides would read root/src/hdr.h
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        root = Path(tmp.name).resolve() / "root"
        text = f'#include "{root}/src/hdr.h"\nint f() {{ return hv(); }}\n'
        cases = {"abs": (text, text, "same")}
        fx = Fixture(tmp.name, cases, HEADERS)
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 2, out + err)
        self.assertIn("allow-list", err)

    def test_header_of_the_other_side_is_caught(self):
        # before names a header that exists only in the AFTER export (found through the scratch directory)
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        fx = Fixture(tmp.name, {"t": ("int f() { return 1; }\n", "int f() { return 1; }\n", "same")}, {})
        other = Path(tmp.name).resolve() / "scratch" / "after-v2" / "tree" / "src" / "o.h"
        write(fx.v1 / "src" / "t.cpp", f'#include "{other}"\nint f() {{ return 1; }}\n')
        write(fx.v2 / "src" / "t.cpp", "int f() { return 1; }\n")
        write(fx.v2 / "src" / "o.h", "// only in the after tree\n")
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 2, out + err)
        self.assertIn("allow-list", err)

    def test_unlisted_cpp_is_refused(self):
        fx = self.fixture({"ws": CASES["ws"]})
        write(fx.v1 / "src" / "extra.cpp", "int g() { return 0; }\n")
        write(fx.v2 / "src" / "extra.cpp", "int g() { return 1; }\n")
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 2, out + err)
        self.assertIn("extra.cpp", err)

    def test_tu_count_constant_is_enforced(self):
        fx = self.fixture({"ws": CASES["ws"]})
        for kw in ({"n": 2}, {"files": 2}):
            rc, res, out, err = fx.run(**{"n": 1, "files": 1, **kw})
            self.assertEqual(rc, 2, out + err)
            self.assertIn("expected", err)
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 0, out + err)

    def test_unread_header_that_changes_is_refused_unless_allowed(self):
        fx = self.fixture({"ws": CASES["ws"]})
        write(fx.v1 / "src" / "unread.h", "inline int u() { return 1; }\n")
        write(fx.v2 / "src" / "unread.h", "inline int u() { return 2; }\n")
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 2, out + err)
        self.assertIn("unread.h", err)
        rc, res, out, err = fx.run(n=1, files=1, extra=["--allow-uncovered", "src/unread.h"])
        self.assertEqual(rc, 0, out + err)
        self.assertIn("unread.h (CHANGED)", out)
        # an unread header that does not change is only listed
        write(fx.v2 / "src" / "unread.h", "inline int u() { return 1; }\n")
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 0, out + err)
        self.assertIn("unread.h (unchanged)", out)

    def test_include_cerastes_h_is_allowed_to_differ_by_default(self):
        fx = self.fixture({"ws": CASES["ws"]})
        write(fx.v1 / "include" / "cerastes.h", "// a\n")
        write(fx.v2 / "include" / "cerastes.h", "// b\n")
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 0, out + err)
        self.assertIn("include/cerastes.h (CHANGED)", out)

    def test_tu_selection(self):
        fx = self.fixture({k: CASES[k] for k in ("ws", "literal")})
        rc, res, out, err = fx.run(n=2, files=2, extra=["--tu", "src/ws.cpp"])
        self.assertEqual(rc, 0, out + err)
        self.assertEqual(list(res), ["fx/src/ws.cpp"])
        rc, res, out, err = fx.run(n=2, files=2, extra=["--tu", "src/nosuch.cpp"])
        self.assertEqual(rc, 2, out + err)

    def test_compile_failure_is_a_failure_not_a_pass(self):
        fx = self.fixture({"broken": ("int f() { return 1; }\n", "int f() { return 1 }\n", "x")})
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 1, out + err)
        self.assertEqual(res["fx/src/broken.cpp"]["verdict"], "COMPILE-FAILED")


class RewriteArgs(unittest.TestCase):
    def test_rewrites_src_and_include_only(self):
        root, tree = "/r/repo", "/r/repo/build/s7/before-x/tree"
        args = [CXX or "c++", "-I/r/repo/src/backend", "-isystem", "/r/repo/include", "-I/r/repo/third_party/edg/src",
                "-I/r/repo/build/cmake/src/backend", "-I/r/repo/srcfoo", "-o", "x.o", "-c", "/r/repo/src/a.cpp", "-MD",
                "-MF", "x.d", '-DCERASTES_SOURCE_INCLUDE_DIR="/r/repo/include"']
        out = oi.rewrite_args(args, root, tree)
        self.assertEqual(out, [args[0], f"-I{tree}/src/backend", "-isystem", f"{tree}/include",
                               "-I/r/repo/third_party/edg/src", "-I/r/repo/build/cmake/src/backend", "-I/r/repo/srcfoo",
                               f"{tree}/src/a.cpp", '-DCERASTES_SOURCE_INCLUDE_DIR="/r/repo/include"'])

    def test_a_leftover_reference_is_an_invalid_proof(self):
        # the check after the rewrite: an unrewritten command is refused, a rewritten one passes
        root = "/r/repo"
        with self.assertRaises(oi.ProofInvalid):
            oi.assert_no_root_refs(["c++", "-I/r/repo/src/vir"], root)
        with self.assertRaises(oi.ProofInvalid):
            oi.assert_no_root_refs(["c++", "-DX=/r/repo/include/y"], root)
        with self.assertRaises(oi.ProofInvalid):
            oi.assert_no_root_refs(["c++", "-c", "/r/repo/src/a.cpp"], root)
        oi.assert_no_root_refs(["c++", "-I/r/repo/build/cmake/src", "-I/r/repo/third_party/edg/src",
                                '-DCERASTES_SOURCE_INCLUDE_DIR="/r/repo/include"', "-I/r/repo/srcfoo"], root)


class DepFile(unittest.TestCase):
    def test_parse(self):
        text = "x.o: /a/b.cpp \\\n /c/d\\ e.h /f.h\n"
        self.assertEqual(oi.parse_depfile(text), ["/a/b.cpp", "/c/d e.h", "/f.h"])


class LineNormalisation(unittest.TestCase):
    AF = "@_Z16assertion_failedPKciS0_S0_S0_"
    IR = ('@.str = private constant [8 x i8] c"/r/src/a.cpp\\00"\n@.str.1 = private constant [6 x i8] c"x.cpp\\00"\n'
          f"  call void {AF}(ptr noundef @.str, i32 noundef 12, ptr noundef null)\n"
          f"  call void {AF}(ptr noundef @.str.1, i32 noundef 13, ptr noundef null)\n"
          "  call void @_Z8log_linePKci(ptr noundef @.str, i32 noundef 14)\n"
          f"  invoke void {AF}(ptr noundef @.str, i32 noundef 15, ptr noundef null)\n")

    def test_only_own_file_assertion_failed_calls_are_normalised(self):
        out, n = oi.normalize_lines(self.IR, "/r")
        self.assertEqual(n, 2)
        self.assertIn(f"call void {self.AF}(ptr noundef @.str, i32 noundef 0, ptr noundef null)", out)
        self.assertIn("@.str.1, i32 noundef 13", out)      # not the file of the tree
        self.assertIn("@_Z8log_linePKci(ptr noundef @.str, i32 noundef 14)", out)    # not assertion_failed
        self.assertIn("invoke void " + self.AF + "(ptr noundef @.str, i32 noundef 0", out)


@unittest.skipIf(CXX is None, "no clang++ found")
class ObjectIdentityAllowList(unittest.TestCase):
    """Check 2 is an allow-list, and check 3 covers every input of the build, not only headers."""

    def fixture(self, cases, headers=None, **kw):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        return Fixture(tmp.name, cases, headers if headers is not None else {}, **kw)

    def test_header_outside_the_root_and_the_toolchain_is_caught(self):
        # another checkout: not under the root (the worktree sits inside the main checkout), not a toolchain
        # directory; found by absolute path, so no rewrite of the command can redirect it
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        other = Path(tmp.name).resolve() / "other_checkout" / "src"
        write(other / "o.h", "inline int ov() { return 1; }\n")
        text = f'#include "{other}/o.h"\nint f() {{ return ov(); }}\n'
        fx = Fixture(tmp.name, {"t": (text, text, "same")}, {})
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 2, out + err)
        self.assertIn("allow-list", err)
        self.assertIn("other_checkout", err)

    def test_explicit_include_directory_outside_the_root_is_not_toolchain(self):
        # a directory the command itself names with -I is the project's, so it is not allowed because it is a
        # search directory
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        other = Path(tmp.name).resolve() / "other_checkout" / "inc"
        write(other / "o.h", "inline int ov() { return 1; }\n")
        text = '#include "o.h"\nint f() { return ov(); }\n'
        fx = Fixture(tmp.name, {"t": (text, text, "same")}, {}, extra_cmd=f"-I{other}")
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 2, out + err)
        self.assertIn("allow-list", err)

    def test_system_headers_are_allowed(self):
        fx = self.fixture({"s": ("#include <vector>\nint f() { return 1; }\n", "#include <vector>\nint f() { return 1; }\n",
                                 "same")})
        rc, res, out, err = fx.run(n=1, files=1)
        self.assertEqual(rc, 0, out + err)

    def test_cmake_and_cmakelists_are_inputs(self):
        for rel in ("cmake/Gen.cmake", "CMakeLists.txt", "src/sub/CMakeLists.txt", "src/driver/tool_paths.h.in"):
            with self.subTest(file=rel):
                fx = self.fixture({"ws": CASES["ws"]})
                write(fx.v1 / rel, "a\n")
                write(fx.v2 / rel, "b\n")
                rc, res, out, err = fx.run(n=1, files=1)
                self.assertEqual(rc, 2, out + err)
                self.assertIn(os.path.basename(rel), err)
                # unchanged, they are only listed
                write(fx.v2 / rel, "a\n")
                rc, res, out, err = fx.run(n=1, files=1)
                self.assertEqual(rc, 0, out + err)
                self.assertIn(f"{rel} (unchanged)", out)

    def test_changed_cmake_file_can_be_allowed(self):
        fx = self.fixture({"ws": CASES["ws"]})
        write(fx.v1 / "CMakeLists.txt", "a\n")
        write(fx.v2 / "CMakeLists.txt", "b\n")
        rc, res, out, err = fx.run(n=1, files=1, extra=["--allow-uncovered", "CMakeLists.txt"])
        self.assertEqual(rc, 0, out + err)


# token-pass cases: name -> (file, before, after, status under `none`, status under `braces`); a before of None is
# a file that exists on the after side only.  "same" / "braces" (only inserted braces) / "DIFFERS".
TOKEN_CASES = {
    "comment_and_whitespace": ("a.cpp", "int f(int a) { return a + 1; } // one\n",
                               "/* two */\nint f(int a)\n{\n    return a +\n        1;\n}\n", "same", "same"),
    "splice_in_a_directive": ("b.h", "#define M(a) ((a) + 1)\n", "#define M(a) \\\n    ((a) + 1)\n", "same", "same"),
    "braces_around_statements": ("c.cpp", "int f(int a) { if (a) return 1; else return 2; }\n",
                                 "int f(int a)\n{\n    if (a)\n    {\n        return 1;\n    }\n    else\n    {\n"
                                 "        return 2;\n    }\n}\n", "DIFFERS", "braces"),
    "braces_nested": ("d.cpp", "void g(int a, int b) { if (a) if (b) h(); else k(); }\n",
                      "void g(int a, int b) { if (a) { if (b) { h(); } else { k(); } } }\n", "DIFFERS", "braces"),
    "scope_inserted": ("e.cpp", "void g() { int x = 1; use(x); }\n", "void g() { { int x = 1; } use(x); }\n",
                       "DIFFERS", "DIFFERS"),
    "brace_not_closed": ("f.cpp", "void g(int a) { if (a) h(); }\n", "void g(int a) { if (a) { h(); }\n",
                         "DIFFERS", "DIFFERS"),
    "braces_and_a_change": ("g.cpp", "int f(int a) { if (a) return 1; return 2; }\n",
                            "int f(int a) { if (a) { return 1; } return 3; }\n", "DIFFERS", "DIFFERS"),
    "static_assert_token_dropped": ("h.h", "static_assert(sizeof(int) == 4 && sizeof(long) == 8);\n",
                                    "static_assert(sizeof(int) == 4);\n", "DIFFERS", "DIFFERS"),
    "if0_branch_changed": ("i.cpp", "#if 0\nint a = 1;\n#endif\n", "#if 0\nint a = 2;\n#endif\n", "DIFFERS",
                           "DIFFERS"),
    "inactive_platform_branch": ("j.cpp", "#ifdef NO_SUCH_MACRO\nint a = f(1);\n#endif\n",
                                 "#ifdef NO_SUCH_MACRO\nint a = f(1U);\n#endif\n", "DIFFERS", "DIFFERS"),
    "token_moved_across_a_directive": ("k.h", "#define A 1\nint x;\n", "#define A 1 int\nx;\n", "DIFFERS", "DIFFERS"),
    "token_moved_before_a_directive": ("l.h", "#if X\nint a;\n#endif\n", "int\n#if X\na;\n#endif\n", "DIFFERS",
                                       "DIFFERS"),
    "deleted_member": ("m.h", "struct S { S(const S&) = delete; int x; };\n", "struct S { S(const S&); int x; };\n",
                       "DIFFERS", "DIFFERS"),
    "access_specifier": ("n.h", "class C { public: int x; };\n", "class C { private: int x; };\n", "DIFFERS", "DIFFERS"),
    "right_shift_not_split": ("o.h", "A<B<int> > x;\n", "A<B<int>> x;\n", "DIFFERS", "DIFFERS"),
    "comment_continuation": ("p.cpp", "void g() {\n  int x = 1; // set x \\\n  x = 2;\n}\n",
                             "void g()\n{\n    int x = 1; // set x\n    x = 2;\n}\n", "DIFFERS", "DIFFERS"),
    "cmake_file_changed": ("CMakeLists.txt", "add_library(a a.cpp)\n", "add_library(a b.cpp)\n", "DIFFERS", "DIFFERS"),
    "file_added": ("q.h", None, "int q;\n", "DIFFERS", "DIFFERS"),
}


class TokenPass(unittest.TestCase):
    """--tokens none|braces over fixture trees, with --no-compile (no compiler, no database)."""

    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        t = Path(cls.tmp.name)
        for name, (f, b, a, _, _) in TOKEN_CASES.items():
            if b is not None:
                write(t / "v1" / "src" / f, b)
            write(t / "v2" / "src" / f, a)
        cls.root = t / "root"
        cls.root.mkdir()
        cls.results = {}
        for mode in ("none", "braces"):
            cls.results[mode] = cls.run_tokens(t, mode)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    @classmethod
    def run_tokens(cls, t, mode, extra=()):
        js = t / f"tok-{mode}.json"
        out, err = io.StringIO(), io.StringIO()
        argv = ["--before", str(t / "v1"), "--after", str(t / "v2"), "--root", str(cls.root), "--scratch",
                str(t / f"scratch-{mode}"), "--tokens", mode, "--no-compile", "--json", str(js), *extra]
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = oi.main(argv)
        res = json.loads(js.read_text())["tokens"] if js.exists() else {}
        return rc, res, out.getvalue(), err.getvalue()

    def test_every_case(self):
        for mode, idx in (("none", 3), ("braces", 4)):
            rc, res, out, err = self.results[mode]
            self.assertEqual(rc, 1, out + err)       # some case differs in each mode
            for name, spec in TOKEN_CASES.items():
                with self.subTest(case=name, mode=mode):
                    self.assertEqual(res["src/" + spec[0] if spec[0] != "CMakeLists.txt" else spec[0]]["status"]
                                     if False else res[("src/" + spec[0])]["status"], spec[idx], out)

    def test_the_report_names_each_differing_file(self):
        out = self.results["none"][2]
        self.assertIn("TOKENS DIFFER  src/h.h", out)
        self.assertIn("tokens (none):", out)

    def test_pure_mode_passes_when_only_clean_cases_exist(self):
        t = Path(self.tmp.name) / "clean"
        for name in ("comment_and_whitespace", "splice_in_a_directive"):
            f, b, a, _, _ = TOKEN_CASES[name]
            write(t / "v1" / "src" / f, b)
            write(t / "v2" / "src" / f, a)
        rc, res, out, err = self.run_tokens(t, "none")
        self.assertEqual(rc, 0, out + err)
        self.assertEqual({r["status"] for r in res.values()}, {"same"})

    def test_braces_mode_alone_passes_and_none_fails(self):
        t = Path(self.tmp.name) / "braced"
        f, b, a, _, _ = TOKEN_CASES["braces_around_statements"]
        write(t / "v1" / "src" / f, b)
        write(t / "v2" / "src" / f, a)
        self.assertEqual(self.run_tokens(t, "braces")[0], 0)
        self.assertEqual(self.run_tokens(t, "none")[0], 1)

    def test_no_compile_needs_a_token_mode(self):
        t = Path(self.tmp.name) / "clean"
        rc, res, out, err = self.run_tokens(t, "off")
        self.assertEqual(rc, 2, out + err)

    def test_token_pass_with_the_compile_passes(self):
        # identical object code but inserted braces: rc 0 with `braces`, 1 with `none`
        if CXX is None:
            self.skipTest("no clang++")
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        fx = Fixture(tmp.name, {k: CASES[k] for k in ("braces", "ws")}, {})
        rc, res, out, err = fx.run(n=2, files=2, extra=["--tokens", "braces"])
        self.assertEqual(rc, 0, out + err)
        rc, res, out, err = fx.run(n=2, files=2, extra=["--tokens", "none"])
        self.assertEqual(rc, 1, out + err)
        self.assertEqual({e["verdict"] for e in res.values()}, {"identical"}, "the compile passes see no difference")
        self.assertIn("TOKENS DIFFER", out)

    def test_tokenizer(self):
        self.assertEqual(oi.tokenize("a >> b; // c\n/* d */ e\\\nf"), ["a", ">>", "b", ";", "ef"])
        self.assertEqual(oi.tokenize("#define X 1 \\\n + 2\nint y;"),
                         ["#", "define", "X", "1", "+", "2", oi.EOD, "int", "y", ";"])
        self.assertEqual(oi.tokenize('x = R"(a\\nb)"; y'), ["x", "=", 'R"(a\\nb)"', ";", "y"])


class CaseCount(unittest.TestCase):
    def test_the_docstring_counts_the_cases(self):
        n = len(CASES) + len(TOKEN_CASES)
        self.assertIn(f"{len(CASES)} compile cases and {len(TOKEN_CASES)} token cases", __doc__, f"{n} cases in all")
        loader = unittest.defaultTestLoader.loadTestsFromModule(sys.modules[__name__])
        self.assertIn(f"({loader.countTestCases()} tests in all)", __doc__)


if __name__ == "__main__":
    unittest.main()
