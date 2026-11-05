#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Self-test for scripts/object_identity.py (session S7, decision 0051): a twin for the harness itself.

Each case is one translation unit of a fixture tree, written in a "before" and an "after" form; the tool compares
the two trees in one run and its per-TU verdicts are checked.  The must-pass cases are the edits the restyle makes
(whitespace, braces around a statement, `U` on an unsigned literal, brace initialization, one declarator per
declaration); the must-fail cases are the edits that change behaviour and that a weak comparison would miss.  Each
must-fail case names the pass that is expected to catch it:

  * `obj`  - the object bytes differ (and the IR does);
  * `ir`   - only the LLVM IR differs (at -O0 the machine code of `int y = x - 1U;` is the instruction of
             `x - 1`, the IR keeps `sub` against `sub nsw`): weakening the tool to objects only fails this test.

The tool's own guards have twins too: a header read from the wrong tree (the include-path rewrite and the
dependency check), a source file that names its header by absolute path, a tracked .cpp or a TU count the compile
database does not match, a header no TU reads that changes, the same tree exported twice into two directories.

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
HDR_MACRO = ("extern void fail(const char*, int);\n#define CHECK(c) ((c) ? (void)0 : fail(__FILE__, __LINE__))\n"
             "int f(int a)\n{\n%sCHECK(a > 0);\n    return a;\n}\n")
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
        self.assertIn("wrong place", err)

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
        self.assertIn("wrong place", err)

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
    IR = ('@.str = private constant [8 x i8] c"/r/src/a.cpp\\00"\n@.str.1 = private constant [6 x i8] c"x.cpp\\00"\n'
          "  call void @fail(ptr noundef @.str, i32 noundef 12)\n  call void @fail(ptr noundef @.str.1, i32 noundef 13)\n")

    def test_only_own_file_calls_are_normalised(self):
        out, n = oi.normalize_lines(self.IR, "/r")
        self.assertEqual(n, 1)
        self.assertIn("@.str, i32 noundef 0", out)
        self.assertIn("@.str.1, i32 noundef 13", out)


if __name__ == "__main__":
    unittest.main()
