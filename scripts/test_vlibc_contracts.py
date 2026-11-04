#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Self-test for scripts/vlibc_contracts.py (decision 0046 items 6 and 8).

Every rule of the reader has a twin that fails when the rule is weakened: a
tag without a declaration, a tag over another function's declaration, a
contract without a tag, a duplicate name, a wrong prefix, a body, two
declarators, a tag not directly above, a contract hidden in a comment (which
must not count) and one in a multi-line declaration (which must).  `check` is
exercised passing and failing (a weakened contract), and `names` for a stable,
sorted, not-rewritten-when-unchanged output.

    python3 scripts/test_vlibc_contracts.py
"""

from __future__ import annotations

import contextlib
import io
import os
import re
import shutil
import sys
import tempfile
import time
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import vlibc_contracts as vc  # noqa: E402

ROOT = HERE.parent
VLIBC_DIR = Path(os.environ.get("CERASTES_VLIBC_INCLUDE_DIR",
                                str(Path.home() / "Documents/cerastes/vlibc/include/vlibc")))
COMMITTED = ROOT / "tests/vlibc/trusted_contracts.txt"

HEADER = """\
// SPDX-License-Identifier: BSL-1.0 OR Apache-2.0 WITH LLVM-exception
#ifndef VLIBC_X_H
#define VLIBC_X_H
#include_next <x.h>
#include <cerastes.h>

// cerastes:trusted-contract vlibc: abort never returns (C23 7.24.4.1)
void abort(void) CERASTES_ENSURES(false);

// cerastes:trusted-contract vlibc: exit never returns (C23 7.24.4.4)
void exit(int status) CERASTES_ENSURES(false);

#endif  // VLIBC_X_H
"""


def run(argv: list[str]) -> tuple[int, str, str]:
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        try:
            code = vc.main(argv)
        except SystemExit as e:           # argparse
            code = int(e.code) if e.code is not None else 0
    return code, out.getvalue(), err.getvalue()


class Base(unittest.TestCase):
    def setUp(self) -> None:
        base = ROOT / "build" / "m4.5d1"
        base.mkdir(parents=True, exist_ok=True)
        self.tmp = Path(tempfile.mkdtemp(prefix="vlibc_contracts_", dir=base))
        self.addCleanup(shutil.rmtree, self.tmp, ignore_errors=True)
        self.inc = self.tmp / "include" / "vlibc"
        self.inc.mkdir(parents=True)

    def header(self, text: str, name: str = "x.h") -> Path:
        p = self.inc / name
        p.write_text(text, encoding="utf-8")
        return p

    def list_(self) -> tuple[int, str, str]:
        return run(["list", "--include-dir", str(self.inc)])

    def assertFails(self, text: str, *fragments: str, name: str = "x.h") -> str:
        self.header(text, name)
        code, out, err = self.list_()
        self.assertEqual(code, 1, f"expected exit 1, stdout={out!r} stderr={err!r}")
        self.assertEqual(out, "", "a failing list must print no listing")
        for f in fragments:
            self.assertIn(f, err)
        return err


class RealHeader(unittest.TestCase):
    @unittest.skipUnless(VLIBC_DIR.is_dir(), "vlibc is not checked out next to cerastes")
    def test_real_vlibc_lists_the_six_names(self) -> None:
        code, out, err = run(["list", "--include-dir", str(VLIBC_DIR)])
        self.assertEqual((code, err), (0, ""))
        self.assertEqual([ln.split(" | ")[0] for ln in out.splitlines()],
                         ["_Exit", "abort", "calloc", "exit", "free", "malloc"])

    @unittest.skipUnless(VLIBC_DIR.is_dir(), "vlibc is not checked out next to cerastes")
    def test_real_vlibc_matches_the_committed_list(self) -> None:
        code, out, err = run(["check", "--include-dir", str(VLIBC_DIR), "--list", str(COMMITTED)])
        self.assertEqual((code, err), (0, ""))

    def test_committed_list_has_the_six_names_and_a_header(self) -> None:
        lines = COMMITTED.read_text().splitlines()
        self.assertTrue(lines[0].startswith("#"))
        body = [ln for ln in lines if ln and not ln.startswith("#")]
        self.assertEqual([ln.split(" | ")[0] for ln in body],
                         ["_Exit", "abort", "calloc", "exit", "free", "malloc"])
        for ln in body:
            self.assertEqual(len(ln.split(" | ")), 4, ln)


class Rules(Base):
    def test_good_header_listing_is_exact(self) -> None:
        self.header(HEADER)
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))
        self.assertEqual(out.splitlines(), [
            "abort | x.h | void abort(void) CERASTES_ENSURES(false); | never returns (C23 7.24.4.1)",
            "exit | x.h | void exit(int status) CERASTES_ENSURES(false); | never returns (C23 7.24.4.4)",
        ])

    def test_listing_is_sorted_by_name_across_headers(self) -> None:
        self.header(HEADER.replace("abort", "zzz"), "a.h")
        self.header(HEADER.replace("abort", "bbb").replace("exit", "yyy"), "b.h")
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))
        self.assertEqual([ln.split(" | ")[:2] for ln in out.splitlines()],
                         [["bbb", "b.h"], ["exit", "a.h"], ["yyy", "b.h"], ["zzz", "a.h"]])

    def test_tag_without_a_declaration(self) -> None:
        err = self.assertFails("// cerastes:trusted-contract vlibc: abort never returns\n\n"
                               "void abort(void) CERASTES_ENSURES(false);\n",
                               "x.h:1:", "not directly above a declaration")
        # the declaration is now untagged too
        self.assertIn("x.h:3:", err)
        self.assertIn("a contract without a cerastes:trusted-contract tag", err)

    def test_tag_at_the_end_of_the_file(self) -> None:
        self.assertFails("void f(void);\n// cerastes:trusted-contract vlibc: abort never returns\n",
                         "x.h:2:", "not directly above a declaration")

    def test_tag_over_another_function(self) -> None:
        self.assertFails("// cerastes:trusted-contract vlibc: abort never returns\n"
                         "void exit(int s) CERASTES_ENSURES(false);\n",
                         "x.h:2:", "names `abort` but the declaration below it declares `exit`")

    def test_tag_over_a_non_function(self) -> None:
        self.assertFails("// cerastes:trusted-contract vlibc: abort never returns\n"
                         "int abort CERASTES_ENSURES(false);\n",
                         "x.h:2:", "declares `(no function)`")

    def test_contract_without_a_tag(self) -> None:
        self.assertFails("void abort(void) CERASTES_ENSURES(false);\n",
                         "x.h:1:", "a contract without a cerastes:trusted-contract tag")

    def test_requires_decreases_owning_without_a_tag(self) -> None:
        for decl in ("void f(int *p) CERASTES_REQUIRES(p != NULL);",
                     "void f(int x) CERASTES_DECREASES(x);",
                     "CERASTES_OWNING void *f(unsigned n);",
                     "void f(int x) [[cerastes::ensures(x > 0)]];"):
            with self.subTest(decl=decl):
                self.assertFails(decl + "\n", "x.h:1:", "a contract without a cerastes:trusted-contract tag")

    def test_plain_declaration_without_a_contract_is_fine(self) -> None:
        self.header("void f(void);\nint g(int x);\n")
        code, out, err = self.list_()
        self.assertEqual((code, out, err), (0, "", ""))

    def test_contract_after_a_plain_declaration_on_the_same_line(self) -> None:
        self.assertFails("void a(void); void b(void) CERASTES_ENSURES(false);\n",
                         "x.h:1:", "a contract without a cerastes:trusted-contract tag")

    def test_duplicate_name_in_one_header(self) -> None:
        self.assertFails(HEADER.replace("exit", "abort"), "x.h:10:", "`abort` is tagged twice (first at")

    def test_duplicate_name_across_headers(self) -> None:
        self.header(HEADER, "a.h")
        self.header(HEADER, "b.h")
        code, out, err = self.list_()
        self.assertEqual(code, 1)
        self.assertIn("is tagged twice", err)
        self.assertIn("b.h:", err)

    def test_wrong_prefix(self) -> None:
        for prefix in ("libc:", "vlibm:", "VLIBC:", "vlibc;", "vlibc:abort"):
            with self.subTest(prefix=prefix):
                tag = f"// cerastes:trusted-contract {prefix} abort never returns\n"
                if prefix == "vlibc:abort":
                    tag = "// cerastes:trusted-contract vlibc:abort never returns\n"
                self.assertFails(tag + "void abort(void) CERASTES_ENSURES(false);\n",
                                 "x.h:1:", "must be exactly `vlibc:`")

    def test_missing_prefix_and_missing_name(self) -> None:
        self.assertFails("// cerastes:trusted-contract\nvoid abort(void) CERASTES_ENSURES(false);\n",
                         "x.h:1:", "malformed tag")
        self.assertFails("// cerastes:trusted-contract vlibc:\nvoid abort(void) CERASTES_ENSURES(false);\n",
                         "x.h:1:", "malformed tag")

    def test_missing_reason(self) -> None:
        self.assertFails("// cerastes:trusted-contract vlibc: abort\nvoid abort(void) CERASTES_ENSURES(false);\n",
                         "x.h:1:", "has no reason text")

    def test_multi_line_declaration(self) -> None:
        self.header("// cerastes:trusted-contract vlibc: copy copies n bytes (C23 7.26.2.1)\n"
                    "char *copy(char *dst,\n"
                    "           const char *src,\n"
                    "           unsigned long __n)\n"
                    "    CERASTES_REQUIRES(__n > 0)\n"
                    "    CERASTES_ENSURES(true);\n")
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))
        self.assertEqual(out, "copy | x.h | char *copy(char *dst, const char *src, unsigned long __n) "
                              "CERASTES_REQUIRES(__n > 0) CERASTES_ENSURES(true); "
                              "| copies n bytes (C23 7.26.2.1)\n")

    def test_multi_line_declaration_with_the_contract_on_a_later_line_needs_the_tag(self) -> None:
        self.assertFails("char *copy(char *dst,\n"
                         "           unsigned long n)\n"
                         "    CERASTES_ENSURES(true);\n", "x.h:1:", "a contract without")

    def test_leading_contract_group_does_not_hide_the_name(self) -> None:
        self.header("// cerastes:trusted-contract vlibc: f x\n"
                    "[[nodiscard]] CERASTES_OWNING char *f(unsigned __n) CERASTES_REQUIRES(__n > 0);\n")
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))
        self.assertTrue(out.startswith("f | x.h | [[nodiscard]] CERASTES_OWNING char *f"))

    def test_tag_not_directly_above(self) -> None:
        for between in ("\n", "// a comment\n", "/* c */\n", "#define X 1\n"):
            with self.subTest(between=between):
                self.assertFails("// cerastes:trusted-contract vlibc: abort never returns\n" + between +
                                 "void abort(void) CERASTES_ENSURES(false);\n",
                                 "x.h:1:", "not directly above a declaration")

    def test_tag_inside_a_declaration(self) -> None:
        self.assertFails("void abort(void)\n"
                         "// cerastes:trusted-contract vlibc: abort never returns\n"
                         "    CERASTES_ENSURES(false);\n",
                         "x.h:2:", "is inside a declaration")

    def test_two_tags_in_a_row(self) -> None:
        self.assertFails("// cerastes:trusted-contract vlibc: abort never returns\n"
                         "// cerastes:trusted-contract vlibc: exit never returns\n"
                         "void exit(int s) CERASTES_ENSURES(false);\n",
                         "x.h:1:", "not directly above a declaration")

    def test_tagged_declaration_with_a_body(self) -> None:
        self.assertFails("// cerastes:trusted-contract vlibc: f x\n"
                         "void f(void) CERASTES_ENSURES(true) { }\n",
                         "x.h:2:", "has a body")

    def test_untagged_inline_body_with_a_contract_is_a_contract_without_a_tag(self) -> None:
        self.assertFails("inline int g(int x) CERASTES_ENSURES(true) { return x; }\n",
                         "x.h:1:", "a contract without a cerastes:trusted-contract tag")

    def test_two_declarators_under_one_tag(self) -> None:
        self.assertFails("// cerastes:trusted-contract vlibc: a x\n"
                         "void a(void) CERASTES_ENSURES(false), b(void);\n",
                         "x.h:2:", "more than one declarator")

    def test_tagged_declaration_without_a_contract(self) -> None:
        self.assertFails("// cerastes:trusted-contract vlibc: a x\nvoid a(void);\n",
                         "x.h:2:", "carries no contract")

    def test_unterminated_declaration(self) -> None:
        self.assertFails("// cerastes:trusted-contract vlibc: a x\nvoid a(void) CERASTES_ENSURES(false)\n",
                         "does not end")

    def test_unbalanced_close_brace(self) -> None:
        self.assertFails("}\n", "x.h:1:", "unbalanced")


class NotDeclarations(Base):
    def test_contract_in_comments_does_not_count(self) -> None:
        self.header("// void f(void) CERASTES_ENSURES(false);\n"
                    "/* void g(void) CERASTES_ENSURES(false); */\n"
                    "/*\n void h(void) CERASTES_ENSURES(false);\n*/\n"
                    "void k(void); // CERASTES_ENSURES(false)\n")
        self.assertEqual(self.list_(), (0, "", ""))

    def test_contract_in_a_preprocessor_line_does_not_count(self) -> None:
        # (a `#define` is refused in a shim header, below; `#error` and `#include` lines are allowed and blanked)
        self.header('#error void f(void) CERASTES_ENSURES(false);\n'
                    "#error void g(void) \\\n    CERASTES_ENSURES(false);\n"
                    '  #  include "void h(void) CERASTES_REQUIRES(1)"\n'
                    "void k(void);\n")
        self.assertEqual(self.list_(), (0, "", ""))

    def test_tag_in_a_comment_mention_is_not_a_tag(self) -> None:
        # vlibc's own header comment mentions the tag mid-line
        self.header("// Every declaration carries a `cerastes:trusted-contract` tag.\n"
                    "void k(void);\n")
        self.assertEqual(self.list_(), (0, "", ""))

    def test_block_comment_tag_is_an_error(self) -> None:
        self.assertFails("/* cerastes:trusted-contract vlibc: abort x */\n"
                         "void abort(void) CERASTES_ENSURES(false);\n",
                         "x.h:1:", "must be a `//` comment alone on its line")

    def test_tag_text_after_code_is_not_a_tag(self) -> None:
        # so the declaration below is untagged and fails
        self.assertFails("int x; // cerastes:trusted-contract vlibc: abort never returns\n"
                         "void abort(void) CERASTES_ENSURES(false);\n",
                         "x.h:2:", "a contract without a cerastes:trusted-contract tag")

    def test_extern_c_block_is_transparent(self) -> None:
        self.header('#ifdef __cplusplus\nextern "C" {\n#endif\n'
                    "// cerastes:trusted-contract vlibc: abort never returns\n"
                    "void abort(void) CERASTES_ENSURES(false);\n"
                    "#ifdef __cplusplus\n}\n#endif\n")
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))
        self.assertTrue(out.startswith("abort | x.h |"))

    def test_string_with_comment_markers_is_code(self) -> None:
        self.header('const char *s = "// not a comment /* nor this";\n'
                    "// cerastes:trusted-contract vlibc: abort never returns\n"
                    "void abort(void) CERASTES_ENSURES(false);\n")
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))
        self.assertTrue(out.startswith("abort | x.h |"))

    def test_a_subdirectory_of_the_shim_directory_is_an_error(self) -> None:
        # decision 0046 item 3: the library class is this directory and its headers are the ones read
        self.header(HEADER)
        (self.inc / "sub").mkdir()
        (self.inc / "sub" / "y.h").write_text("void zzz(void) CERASTES_ENSURES(false);\n")
        code, out, err = self.list_()
        self.assertEqual((code, out), (1, ""))
        self.assertIn("a subdirectory of the shim directory", err)
        self.assertIn("sub:0:", err)

    def test_an_empty_subdirectory_is_an_error_too(self) -> None:
        self.header(HEADER)
        (self.inc / "empty").mkdir()
        code, out, err = self.list_()
        self.assertEqual(code, 1)
        self.assertIn("a subdirectory of the shim directory", err)

    def test_a_file_that_is_not_a_header_is_an_error(self) -> None:
        # twin: notes.txt next to the header would be harmless to ordinary compilers, but a file named like a C++
        # header (`array`) shadows it for the front end
        self.header(HEADER)
        for name in ("notes.txt", "array", "stdlib.hpp"):
            with self.subTest(name=name):
                f = self.inc / name
                f.write_text("x\n")
                code, out, err = self.list_()
                self.assertEqual((code, out), (1, ""))
                self.assertIn("not a *.h header", err)
                f.unlink()
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))

    def test_a_header_elsewhere_under_the_include_directory_is_an_error(self) -> None:
        self.header(HEADER)
        for name, make in (("stray.h", lambda p: p.write_text("int f(void);\n")),
                           ("other", lambda p: p.mkdir())):
            with self.subTest(name=name):
                p = self.inc.parent / name
                make(p)
                code, out, err = self.list_()
                self.assertEqual((code, out), (1, ""))
                self.assertIn(f"under {self.inc.resolve().parent} other than vlibc/", err)
                self.assertIn(name, err)
                if p.is_dir():
                    p.rmdir()
                else:
                    p.unlink()
        code, out, err = self.list_()   # twin: with nothing else there it passes
        self.assertEqual((code, err), (0, ""))

    def test_the_check_subcommand_and_names_refuse_them_too(self) -> None:
        self.header(HEADER)
        (self.inc.parent / "stray.h").write_text("int f(void);\n")
        committed = self.tmp / "c.txt"
        committed.write_text("")
        self.assertEqual(run(["check", "--include-dir", str(self.inc), "--list", str(committed)])[0], 1)
        out = self.tmp / "n.inc"
        self.assertEqual(run(["names", "--include-dir", str(self.inc), "--out", str(out)])[0], 1)
        self.assertFalse(out.exists())

    def test_only_h_files_of_the_directory_are_read(self) -> None:
        self.header(HEADER)
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))
        self.assertEqual([ln.split(" | ")[0] for ln in out.splitlines()], ["abort", "exit"])

    def test_empty_or_missing_directory_is_an_error(self) -> None:
        code, out, err = self.list_()
        self.assertEqual(code, 1)
        self.assertIn("no *.h files", err)
        code, out, err = run(["list", "--include-dir", str(self.tmp / "nope")])
        self.assertEqual(code, 1)
        self.assertIn("not a directory", err)

    def test_every_error_is_reported_not_only_the_first(self) -> None:
        err = self.assertFails("void a(void) CERASTES_ENSURES(false);\n"
                               "void b(void) CERASTES_ENSURES(false);\n", "x.h:1:", "x.h:2:")
        self.assertEqual(err.count("a contract without"), 2)


class Directives(Base):
    """Decision 0046 item 3 (audit round 2, S1): a shim header's only `#define` is its own include guard, and it has no
    `#undef`, `#pragma`, `#line`, linemarker, `_Pragma` or digraph/trigraph `#`.  Each refusal has the guard-only header
    as its twin (it passes), so dropping a rule fails here."""

    GUARDED = ("#ifndef VLIBC_X_H\n#define VLIBC_X_H\n#include <cerastes.h>\n"
               "// cerastes:trusted-contract vlibc: abort never returns\n"
               "void abort(void) CERASTES_ENSURES(false);\n#endif\n")

    def test_the_guard_alone_passes(self) -> None:
        self.header(self.GUARDED)
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))
        self.assertTrue(out.startswith("abort | x.h |"))

    def test_a_define_other_than_the_guard_is_an_error(self) -> None:
        for what in ("#define false 1\n", "#define VLIBC_X_H\n", "#define NULL ((void *) 0)\n",
                     "#define F(x) x\n", "#  define  false  1\n", "#define G \\\n    1\n"):
            with self.subTest(what=what):
                err = self.assertFails(self.GUARDED.replace("#include", what + "#include", 1),
                                       "other than its own include guard")
                self.assertIn("x.h:3:", err)

    def test_the_guard_with_a_value_or_without_its_ifndef_is_an_error(self) -> None:
        self.assertFails(self.GUARDED.replace("#define VLIBC_X_H\n", "#define VLIBC_X_H 1\n"),
                         "x.h:2:", "other than its own include guard")
        self.assertFails(self.GUARDED.replace("#ifndef VLIBC_X_H\n", ""), "x.h:1:",
                         "other than its own include guard")
        self.assertFails(self.GUARDED.replace("#ifndef VLIBC_X_H\n", "#ifndef VLIBC_Y_H\n"), "x.h:2:",
                         "other than its own include guard")
        self.assertFails(self.GUARDED.replace("VLIBC_X_H", "X_H"), "other than its own include guard")
        # a second define of the guard is not the guard
        self.assertFails(self.GUARDED.replace("#include <cerastes.h>\n", "#define VLIBC_X_H\n"),
                         "x.h:3:", "other than its own include guard")

    def test_a_second_guard_pair_is_not_the_guard(self) -> None:
        self.assertFails(self.GUARDED.replace("#include <cerastes.h>\n",
                                              "#ifndef VLIBC_X_H\n#define VLIBC_X_H\n#include <cerastes.h>\n", 1),
                         "x.h:4:", "other than its own include guard")

    def test_a_guard_define_with_a_line_continuation_is_still_the_guard(self) -> None:
        # twin of the refusals: the continuation is joined, so the define is still `#define VLIBC_X_H` alone
        self.header(self.GUARDED.replace("#define VLIBC_X_H\n", "#define VLIBC_X_H \\\n\n"))
        self.assertEqual(self.list_()[0], 0)

    def test_a_directive_on_the_last_line_without_a_newline_is_read(self) -> None:
        self.assertFails(self.GUARDED + "#define false 1", "x.h:7:", "other than its own include guard")
        self.assertFails(self.GUARDED + "#pragma once", "x.h:7:", "a `#pragma`")

    def test_an_undef_is_an_error(self) -> None:
        for what in ("#undef false\n", "#  undef NULL\n"):
            with self.subTest(what=what):
                self.assertFails(self.GUARDED.replace("#include", what + "#include", 1), "x.h:3:", "an `#undef`")

    def test_a_pragma_is_an_error(self) -> None:
        for what in ("#pragma once\n", "#pragma GCC system_header\n", "# pragma weak abort\n"):
            with self.subTest(what=what):
                self.assertFails(self.GUARDED.replace("#include", what + "#include", 1), "x.h:3:", "a `#pragma`")

    def test_the_pragma_operator_is_an_error(self) -> None:
        for what in ('_Pragma("once")\n', "int x = 0; __pragma(weak)\n"):
            with self.subTest(what=what):
                self.assertFails(self.GUARDED.replace("void abort", what + "void abort", 1),
                                 "a `_Pragma` operator")

    def test_a_line_directive_or_linemarker_is_an_error(self) -> None:
        for what in ("#line 10\n", '#line 10 "other.h"\n', '# 12 "other.h"\n', "#  7\n"):
            with self.subTest(what=what):
                self.assertFails(self.GUARDED.replace("#include", what + "#include", 1), "x.h:3:",
                                 "a `#line` or linemarker")

    def test_digraph_and_trigraph_hashes_are_an_error(self) -> None:
        for what in ("%:define false 1\n", "??=define false 1\n"):
            with self.subTest(what=what):
                self.assertFails(self.GUARDED.replace("#include", what + "#include", 1), "x.h:3:",
                                 "digraph or trigraph")

    def test_a_directive_inside_a_comment_is_not_a_directive(self) -> None:
        # twin of the refusals: text that only looks like a directive is a comment or a string
        self.header(self.GUARDED.replace("#include", "// #define false 1\n/* #pragma once */\n#include", 1))
        self.assertEqual(self.list_()[0], 0)

    def test_conditionals_and_includes_are_allowed(self) -> None:
        self.header("#ifndef VLIBC_X_H\n#define VLIBC_X_H\n#include_next <stdlib.h>\n#include <cerastes.h>\n"
                    '#ifdef __cplusplus\nextern "C" {\n#endif\n'
                    "// cerastes:trusted-contract vlibc: abort never returns\n"
                    "void abort(void) CERASTES_ENSURES(false);\n"
                    "#ifdef __cplusplus\n}\n#endif\n#endif\n")
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))

    def test_check_and_names_refuse_them_too(self) -> None:
        self.header(self.GUARDED.replace("#include", "#define false 1\n#include", 1))
        committed = self.tmp / "c.txt"
        committed.write_text("")
        self.assertEqual(run(["check", "--include-dir", str(self.inc), "--list", str(committed)])[0], 1)
        out = self.tmp / "n.inc"
        self.assertEqual(run(["names", "--include-dir", str(self.inc), "--out", str(out)])[0], 1)
        self.assertFalse(out.exists())


class Literals(Base):
    """Audit round 3, note 5: C23 digit separators and raw strings could make the reader lose its place (a `'` after a
    number starts a "character literal" that swallows a quote and hides a comment opener); they are refused, and so is
    an unterminated literal.  Ordinary character and string literals, with quotes inside, are the twins."""

    GUARDED = Directives.GUARDED

    def with_line(self, line: str) -> str:
        return self.GUARDED.replace("#include <cerastes.h>\n", "#include <cerastes.h>\n" + line + "\n", 1)

    def test_the_audits_trick_header_is_refused(self) -> None:
        # the header of audit round 3: the separator desyncs the reader so `#define false 1` and a comment opener hide
        self.header("#ifndef VLIBC_STDLIB_H\n#define VLIBC_STDLIB_H\n\n#include_next <stdlib.h>\n\n#include <cerastes.h>\n\n"
                    "enum { VLIBC_K = 1'2 + sizeof \"a'/*\" };\n#define false 1\nenum { VLIBC_J = sizeof \"*/\" };\n\n"
                    "// cerastes:trusted-contract vlibc: abort never returns (C23 7.24.4.1)\n"
                    "void abort(void) CERASTES_ENSURES(false);\n\n#endif  // VLIBC_STDLIB_H\n")
        code, out, err = self.list_()
        self.assertEqual((code, out), (1, ""))
        self.assertIn("x.h:8:", err)
        self.assertIn("a C23 digit separator", err)

    def test_a_digit_separator_is_an_error(self) -> None:
        for lit in ("enum { A = 1'000 };", "enum { B = 0x1'F };", "enum { C = 1'0 + 2 };", "int x'y;"):
            with self.subTest(lit=lit):
                self.assertFails(self.with_line(lit), "x.h:4:", "a C23 digit separator")

    def test_ordinary_character_literals_are_fine(self) -> None:
        for lit in ("enum { A = 'a' };", "enum { B = '\\'' };", "enum { C = '\"' };", "enum { D = L'a' };",
                    "enum { E = u8'a' };", "enum { F = u'a' };", "enum { G = U'a' };", "enum { H = ('a') };",
                    "enum { I = 1 + 'a' };", 'const char *s = "it\'s a \'string\'";', "enum { J = '\\\\' };"):
            with self.subTest(lit=lit):
                self.header(self.with_line(lit))
                code, out, err = self.list_()
                self.assertEqual((code, err), (0, ""))

    def test_a_raw_string_literal_is_an_error(self) -> None:
        for lit in ('const char *s = R"(a)";', 'const char *s = u8R"(a)";', 'const char *s = LR"x(a)x";'):
            with self.subTest(lit=lit):
                self.assertFails(self.with_line(lit), "x.h:4:", "a raw string literal")

    def test_an_identifier_ending_in_r_before_a_string_is_not_a_raw_string(self) -> None:
        self.header(self.with_line('const char *s = bar "x";'))
        self.assertEqual(self.list_()[0], 0)

    def test_an_unterminated_literal_is_an_error(self) -> None:
        for lit in ('const char *s = "abc;', "enum { A = 'a };"):
            with self.subTest(lit=lit):
                self.assertFails(self.with_line(lit), "x.h:4:", "an unterminated string or character literal")


class CommentsInTrusted(Base):
    """Audit round 2, note 11: a comment inside a trusted declaration is refused (a `)` in one would end the reading of
    the contract's identifiers); a comment before or after the declaration is fine."""

    TAG = "// cerastes:trusted-contract vlibc: f x\n"

    def test_a_comment_inside_a_declaration_is_an_error(self) -> None:
        for decl in ("void f(int a, /* the status */ int b) CERASTES_ENSURES(false);\n",
                     "void f(int a) CERASTES_ENSURES(false /* ) */);\n",
                     "void f(int a) // note\n    CERASTES_ENSURES(false);\n",
                     "void f(int a)\n    /* two\n       lines */\n    CERASTES_ENSURES(false);\n",
                     "void f(int a) CERASTES_ENSURES(false) /* before the semicolon */;\n"):
            with self.subTest(decl=decl):
                err = self.assertFails(self.TAG + decl, "a comment inside the trusted declaration of `f`")
                self.assertIn("x.h:2:", err)

    def test_comments_around_a_declaration_are_fine(self) -> None:
        # twins: before the tag, between the tag's declaration and the next, after the semicolon, and a comment
        # inside an untagged declaration without a contract
        self.header("/* a block comment */ // and a line one\n" + self.TAG +
                    "  /* before it on its line */ void f(int a) CERASTES_ENSURES(false); // after the semicolon\n"
                    "/* between */\nvoid k(int a /* plain */, int b);\n")
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))
        self.assertTrue(out.startswith("f | x.h | void f(int a) CERASTES_ENSURES(false); | x"))

    def test_the_real_header_comment_style_passes(self) -> None:
        # vlibc's own header has comments before the tag and after the declarations
        self.header(HEADER.replace("void exit(int status) CERASTES_ENSURES(false);",
                                   "void exit(int status) CERASTES_ENSURES(false);  // trailing"))
        self.assertEqual(self.list_()[0], 0)


class ReservedIdentifiers(Base):
    """Decision 0047 item 1: every identifier in a contract argument is reserved or NULL, false, true."""

    MALLOC = ("// cerastes:trusted-contract vlibc: malloc returns NULL or a new allocation (C23 7.24.3.6)\n"
              "void *malloc(size_t __size) CERASTES_REF_TO_UNINIT CERASTES_ENSURES(__r, __r == NULL || "
              "cerastes_owns_block(__r, __size));\n")
    CALLOC = ("// cerastes:trusted-contract vlibc: calloc returns NULL or a new zeroed allocation (C23 7.24.3.2)\n"
              "void *calloc(size_t __nmemb, size_t __size) CERASTES_ENSURES(__r, __r == NULL || "
              "(cerastes_owns_block(__r, __nmemb * __size) && cerastes_zeroed(__r)));\n")
    FREE = ("// cerastes:trusted-contract vlibc: free ends an allocation (C23 7.24.3.3)\n"
            "void free(void *__ptr CERASTES_REF_TO_UNINIT) CERASTES_REQUIRES(__ptr == NULL || "
            "cerastes_owns_block(__ptr));\n")

    def ok(self, text: str) -> str:
        self.header(text)
        code, out, err = self.list_()
        self.assertEqual((code, err), (0, ""))
        return out

    def test_the_three_malloc_family_declarations_pass_and_list_exactly(self) -> None:
        out = self.ok(self.MALLOC + self.CALLOC + self.FREE)
        self.assertEqual([ln.split(" | ")[0] for ln in out.splitlines()], ["calloc", "free", "malloc"])
        self.assertIn("free | x.h | void free(void *__ptr CERASTES_REF_TO_UNINIT) "
                      "CERASTES_REQUIRES(__ptr == NULL || cerastes_owns_block(__ptr)); | ends an allocation "
                      "(C23 7.24.3.3)", out)
        self.assertIn("malloc | x.h | void *malloc(size_t __size) CERASTES_REF_TO_UNINIT CERASTES_ENSURES(__r, "
                      "__r == NULL || cerastes_owns_block(__r, __size)); | returns NULL or a new allocation "
                      "(C23 7.24.3.6)", out)

    def test_a_bare_marker_after_the_declarator_does_not_hide_the_name(self) -> None:
        # the marker sits between the parameter list and the contract: the name is still `malloc`
        out = self.ok(self.MALLOC)
        self.assertTrue(out.startswith("malloc | x.h |"))

    def test_a_bare_marker_after_a_parameters_declarator_id_does_not_hide_the_name(self) -> None:
        out = self.ok(self.FREE)
        self.assertTrue(out.startswith("free | x.h |"))

    def test_a_bare_marker_before_the_declaration_does_not_hide_the_name(self) -> None:
        out = self.ok("// cerastes:trusted-contract vlibc: f x\n"
                      "CERASTES_OWNING int *f(int *__ptr) CERASTES_REQUIRES(__ptr == NULL);\n")
        self.assertTrue(out.startswith("f | x.h | CERASTES_OWNING int *f("))

    def test_a_marker_only_declaration_is_a_trusted_declaration_and_needs_its_tag(self) -> None:
        # a bare marker is a CERASTES_ name: the declaration counts as carrying a contract (decision 0047 item 3)
        out = self.ok("// cerastes:trusted-contract vlibc: free x\n"
                      "int free(int *__ptr CERASTES_REF_TO_UNINIT);\n")
        self.assertTrue(out.startswith("free | x.h | int free(int *__ptr CERASTES_REF_TO_UNINIT);"))
        self.assertFails("int free(int *__ptr CERASTES_REF_TO_UNINIT);\n", "x.h:1:",
                         "a contract without a cerastes:trusted-contract tag")

    def test_a_non_reserved_parameter_name_is_an_error(self) -> None:
        err = self.assertFails(self.FREE.replace("__ptr", "ptr"), "x.h:2:", "the identifier `ptr`", "not reserved",
                               "decision 0047 item 1")
        self.assertEqual(err.count("the identifier `ptr`"), 2)       # the contract names it twice

    def test_a_non_reserved_result_binder_is_an_error(self) -> None:
        self.assertFails(self.MALLOC.replace("__r", "r"), "the identifier `r` in the contract of `malloc`")

    def test_a_non_reserved_function_in_a_contract_is_an_error(self) -> None:
        self.assertFails(self.FREE.replace("cerastes_owns_block", "owns_block"), "the identifier `owns_block`")
        self.assertFails(self.FREE.replace("cerastes_owns_block", "xcerastes_owns_block"),
                         "the identifier `xcerastes_owns_block`")

    def test_a_cxx_spelling_is_not_reserved(self) -> None:
        # `cerastes::owns_block` is the C++ spelling; a shim is the common subset, so it uses the C spelling
        self.assertFails(self.FREE.replace("cerastes_owns_block(__ptr)", "cerastes::owns_block(__ptr)"),
                         "the identifier `cerastes`", "the identifier `owns_block`")

    def test_a_single_underscore_is_not_reserved(self) -> None:
        self.assertFails(self.FREE.replace("__ptr", "_ptr"), "the identifier `_ptr`")
        self.assertFails(self.FREE.replace("__ptr", "_Ptr"), "the identifier `_Ptr`")

    def test_null_false_and_true_are_the_only_plain_identifiers(self) -> None:
        self.ok("// cerastes:trusted-contract vlibc: f x\n"
                "void f(int *__p) CERASTES_REQUIRES(__p == NULL || true) CERASTES_ENSURES(false);\n")
        for word in ("nullptr", "size", "True", "sizeof", "int"):
            with self.subTest(word=word):
                self.assertFails("// cerastes:trusted-contract vlibc: f x\n"
                                 f"void f(int *__p) CERASTES_REQUIRES({word} == __p);\n",
                                 f"the identifier `{word}`")

    def test_numbers_and_literals_are_not_identifiers(self) -> None:
        self.ok("// cerastes:trusted-contract vlibc: f x\n"
                "void f(unsigned long __n) CERASTES_REQUIRES(__n > 0u && __n < 0x7fffUL && __n != 1e3) "
                "CERASTES_ENSURES(__n == 'a' || __n == 1.5e+3);\n")

    def test_a_string_literal_is_skipped_and_the_text_after_it_is_read(self) -> None:
        self.ok('// cerastes:trusted-contract vlibc: f x\n'
                'void f(unsigned long __n) CERASTES_REQUIRES(__n > sizeof_not_read_in_a_string_literal_(""));\n'
                .replace("sizeof_not_read_in_a_string_literal_", "cerastes_f"))
        self.assertFails('// cerastes:trusted-contract vlibc: f x\n'
                         'void f(unsigned long __n) CERASTES_REQUIRES(cerastes_f("size") && size);\n',
                         "the identifier `size`")

    def test_the_attribute_spelling_is_read_too(self) -> None:
        self.ok("// cerastes:trusted-contract vlibc: f x\n"
                "void f(int *__p) [[cerastes::requires(__p == NULL)]];\n")
        self.assertFails("// cerastes:trusted-contract vlibc: f x\n"
                         "void f(void *p) [[cerastes::requires(p == NULL)]];\n", "the identifier `p`")

    def test_any_contract_macro_is_read_not_only_requires_and_ensures(self) -> None:
        for macro in ("CERASTES_REQUIRES", "CERASTES_ENSURES", "CERASTES_INVARIANT", "CERASTES_DECREASES",
                      "CERASTES_CHECK"):
            with self.subTest(macro=macro):
                self.assertFails("// cerastes:trusted-contract vlibc: f x\n"
                                 f"void f(int *__p) CERASTES_ENSURES(true) {macro}(q == 0);\n",
                                 "the identifier `q`")

    def test_a_non_reserved_identifier_in_a_nested_call_or_parenthesis_is_read(self) -> None:
        self.assertFails("// cerastes:trusted-contract vlibc: f x\n"
                         "void f(int *__p) CERASTES_REQUIRES(((cerastes_live(__p) && (n))));\n",
                         "the identifier `n`")

    def test_an_identifier_outside_the_contract_groups_is_not_checked(self) -> None:
        # parameter names, types and markers stay free: only the contract's arguments are read
        self.ok("// cerastes:trusted-contract vlibc: f x\n"
                "unsigned long f(const struct tm *p, size_t n) CERASTES_REF_TO_UNINIT CERASTES_ENSURES(false);\n")

    def test_the_line_of_the_error_is_the_line_of_the_identifier(self) -> None:
        err = self.assertFails("// cerastes:trusted-contract vlibc: f x\n"
                               "void f(int *__p,\n"
                               "       unsigned long __n)\n"
                               "    CERASTES_REQUIRES(__n > 0)\n"
                               "    CERASTES_ENSURES(bad > 0);\n", "the identifier `bad`")
        self.assertIn("x.h:5:", err)

    def test_check_and_names_refuse_it_too(self) -> None:
        self.header(self.FREE.replace("__ptr", "ptr"))
        committed = self.tmp / "c.txt"
        committed.write_text("# nothing\n")
        out = self.tmp / "n.inc"
        for argv in (["check", "--include-dir", str(self.inc), "--list", str(committed)],
                     ["names", "--include-dir", str(self.inc), "--out", str(out)]):
            with self.subTest(cmd=argv[0]):
                code, so, err = run(argv)
                self.assertEqual(code, 1)
                self.assertIn("the identifier `ptr`", err)
        self.assertFalse(out.exists())

    def test_a_declaration_that_fails_the_rule_is_not_listed_but_the_others_are_still_checked(self) -> None:
        err = self.assertFails(self.FREE.replace("__ptr", "ptr") + self.MALLOC.replace("__r", "r"),
                               "the identifier `ptr`", "the identifier `r`")
        self.assertNotIn("malloc | x.h", err)


class VoidPointerShapes(Base):
    """Decision 0047 item 2 (audit S5): a trusted declaration with `void *` in its signature is the (A) allocation or the (D)
    deallocation shape and nothing else, checked textually."""

    A_MALLOC = ("void *malloc(size_t __size) CERASTES_REF_TO_UNINIT "
                "CERASTES_ENSURES(__r, __r == NULL || cerastes_owns_block(__r, __size));")
    A_CALLOC = ("void *calloc(size_t __nmemb, size_t __size) CERASTES_ENSURES(__r, __r == NULL || "
                "(cerastes_owns_block(__r, __nmemb * __size) && cerastes_zeroed(__r)));")
    D_FREE = "void free(void *__ptr CERASTES_REF_TO_UNINIT) CERASTES_REQUIRES(__ptr == NULL || cerastes_owns_block(__ptr));"

    def listing(self, declaration, name="f"):
        self.header(f"// cerastes:trusted-contract vlibc: {name} x\n{declaration}\n")
        return self.list_()

    def assertShape(self, declaration, name="f"):
        code, out, err = self.listing(declaration, name)
        self.assertEqual(code, 1, (declaration, out))
        self.assertIn("x.h:2:", err)
        self.assertEqual(out, "")
        return err

    def test_the_three_real_declarations_pass(self):
        for name, decl in (("malloc", self.A_MALLOC), ("calloc", self.A_CALLOC), ("free", self.D_FREE)):
            with self.subTest(name=name):
                self.assertEqual(self.listing(decl, name)[::2], (0, ""))

    def test_another_allocation_of_the_two_shapes_passes(self):
        for name, decl in (("aligned", "void *aligned(size_t __a, size_t __size) CERASTES_REF_TO_UNINIT CERASTES_ENSURES(__r, __r == NULL || cerastes_owns_block(__r, __size));"),
                           ("one", "void *one(int __n) CERASTES_ENSURES(__r, __r == NULL || (cerastes_owns_block(__r, __n) && cerastes_zeroed(__r)));"),
                           ("cube", "void *cube(size_t __a, size_t __b, size_t __c) CERASTES_REF_TO_UNINIT CERASTES_ENSURES(__r, __r == NULL || cerastes_owns_block(__r, __a * __b * __c));")):
            with self.subTest(name=name):
                self.assertEqual(self.listing(decl, name)[::2], (0, ""))

    def test_no_void_pointer_no_shape(self):
        # a signature without `void *` is not read by this rule at all
        self.assertEqual(self.listing("char *f(char *__p, unsigned long __n) CERASTES_REQUIRES(__n > 0 && __p == NULL);")[::2], (0, ""))

    def test_an_allocation_needs_exactly_one_of_the_marker_and_zeroed(self):
        self.assertShape(self.A_MALLOC.replace(" CERASTES_REF_TO_UNINIT", ""), "malloc")
        self.assertShape(self.A_CALLOC.replace("(cerastes_owns_block(__r, __nmemb * __size) && cerastes_zeroed(__r))", "cerastes_owns_block(__r, __nmemb * __size)"), "calloc")
        both = self.A_CALLOC.replace("void *calloc(size_t __nmemb, size_t __size)", "void *calloc(size_t __nmemb, size_t __size) CERASTES_REF_TO_UNINIT")
        self.assertShape(both, "calloc")

    def test_an_allocation_has_one_ensures_of_the_form_and_no_requires(self):
        self.assertShape(self.A_MALLOC.replace("__r == NULL || ", ""), "malloc")
        self.assertShape(self.A_MALLOC.replace("__r == NULL || cerastes_owns_block(__r, __size)", "true"), "malloc")
        self.assertShape(self.A_MALLOC.replace("CERASTES_ENSURES", "CERASTES_REQUIRES(__size > 0) CERASTES_ENSURES"), "malloc")
        self.assertShape(self.A_MALLOC[:-1] + " CERASTES_ENSURES(__r == NULL);", "malloc")
        self.assertShape(self.A_MALLOC.replace("(__r, __r == NULL", "(__r, __r != NULL"), "malloc")

    def test_an_allocations_size_is_a_parameter_or_a_product_of_parameters(self):
        self.assertShape(self.A_MALLOC.replace("cerastes_owns_block(__r, __size)", "cerastes_owns_block(__r, 4)"), "malloc")
        self.assertShape(self.A_MALLOC.replace("cerastes_owns_block(__r, __size)", "cerastes_owns_block(__r, __other)"), "malloc")
        self.assertShape(self.A_MALLOC.replace("cerastes_owns_block(__r, __size)", "cerastes_owns_block(__r, __size + 1)"), "malloc")
        self.assertShape(self.A_MALLOC.replace("cerastes_owns_block(__r, __size)", "cerastes_owns_block(__r)"), "malloc")

    def test_an_allocations_parameters_are_integers(self):
        self.assertShape(self.A_MALLOC.replace("size_t __size", "double __size"), "malloc")
        self.assertShape(self.A_MALLOC.replace("size_t __size", "char *__size"), "malloc")
        self.assertShape(self.A_MALLOC.replace("size_t __size", "void *__size"), "malloc")
        self.assertShape(self.A_MALLOC.replace("(size_t __size)", "(void)"), "malloc")

    def test_an_allocation_returns_void_pointer(self):
        self.assertShape(self.A_MALLOC.replace("void *malloc", "const void *malloc"), "malloc")
        self.assertShape(self.A_MALLOC.replace("void *malloc", "void **malloc"), "malloc")

    def test_a_deallocation_has_the_marker_on_its_parameter_and_one_requires(self):
        self.assertShape(self.D_FREE.replace(" CERASTES_REF_TO_UNINIT", ""), "free")
        self.assertShape(self.D_FREE.replace("void *__ptr CERASTES_REF_TO_UNINIT)", "void *__ptr) CERASTES_REF_TO_UNINIT"), "free")
        self.assertShape(self.D_FREE.replace("__ptr == NULL || ", ""), "free")
        self.assertShape(self.D_FREE[:-1] + " CERASTES_ENSURES(true);", "free")
        self.assertShape(self.D_FREE.replace("void free", "int free"), "free")
        self.assertShape(self.D_FREE.replace("(void *__ptr", "(void *__ptr, int __n").replace("CERASTES_REF_TO_UNINIT)", "CERASTES_REF_TO_UNINIT)"), "free")

    def test_any_other_void_pointer_signature_is_refused(self):
        for decl in ("void *copy(void *__dst, const void *__src, size_t __n) CERASTES_ENSURES(__r, __r == __dst);",
                     "void f(const void *__p) CERASTES_REQUIRES(__p == NULL);",
                     "void *f(void) CERASTES_ENSURES(false);",
                     "int f(void *__p) CERASTES_REQUIRES(__p == NULL);",
                     "void f(void * __p CERASTES_REF_TO_UNINIT) CERASTES_REQUIRES(__p == NULL);",
                     "void f(void **__pp) CERASTES_REQUIRES(__pp == NULL);"):
            with self.subTest(decl=decl):
                name = re.search(r"([A-Za-z_]+)\(", decl.replace("void *", "", 1) if decl.startswith("void *") else decl).group(1)
                err = self.assertShape(decl, name)
                self.assertIn("neither (A)", err)

    def test_check_and_names_refuse_it_too(self):
        self.header("// cerastes:trusted-contract vlibc: f x\nvoid *f(void) CERASTES_ENSURES(false);\n")
        committed = self.tmp / "c.txt"
        committed.write_text("# nothing\n")
        out = self.tmp / "n.inc"
        for argv in (["check", "--include-dir", str(self.inc), "--list", str(committed)],
                     ["names", "--include-dir", str(self.inc), "--out", str(out)]):
            with self.subTest(cmd=argv[0]):
                code, so, err = run(argv)
                self.assertEqual(code, 1)
                self.assertIn("decision 0047 item 2", err)
        self.assertFalse(out.exists())

    def test_the_real_vlibc_header_has_the_shapes(self):
        if not VLIBC_DIR.is_dir():
            self.skipTest("vlibc is not checked out next to cerastes")
        self.assertEqual(run(["list", "--include-dir", str(VLIBC_DIR)])[::2], (0, ""))


class Check(Base):
    def setUp(self) -> None:
        super().setUp()
        self.header(HEADER)
        self.committed = self.tmp / "trusted_contracts.txt"
        code, out, err = self.list_()
        self.assertEqual(code, 0)
        self.committed.write_text("# a comment header\n# another\n" + out, encoding="utf-8")

    def check(self) -> tuple[int, str, str]:
        return run(["check", "--include-dir", str(self.inc), "--list", str(self.committed)])

    def test_check_passes_and_ignores_comments_and_blank_lines(self) -> None:
        self.assertEqual(self.check(), (0, "", ""))
        self.committed.write_text("\n# c\n\n" + self.committed.read_text() + "\n\n# tail\n")
        self.assertEqual(self.check(), (0, "", ""))

    def test_a_changed_contract_fails_with_a_diff_and_the_message(self) -> None:
        self.header(HEADER.replace("void abort(void) CERASTES_ENSURES(false);",
                                   "void abort(void) CERASTES_ENSURES(true);"))
        code, out, err = self.check()
        self.assertEqual(code, 1)
        self.assertIn("-abort | x.h | void abort(void) CERASTES_ENSURES(false);", err)
        self.assertIn("+abort | x.h | void abort(void) CERASTES_ENSURES(true);", err)
        self.assertIn("a trusted vlibc contract changed: review it (record 0030 item 5: the "
                      "soundness-auditor reads every trusted contract diff) and update "
                      + str(self.committed), err)

    def test_a_changed_reason_fails(self) -> None:
        self.header(HEADER.replace("never returns (C23 7.24.4.1)", "never returns (C23 7.24.4.2)"))
        self.assertEqual(self.check()[0], 1)

    def test_an_added_trusted_declaration_fails(self) -> None:
        self.header(HEADER.replace("#endif", "// cerastes:trusted-contract vlibc: quick_exit x\n"
                                             "void quick_exit(int s) CERASTES_ENSURES(false);\n#endif"))
        code, out, err = self.check()
        self.assertEqual(code, 1)
        self.assertIn("+quick_exit | x.h |", err)

    def test_a_removed_trusted_declaration_fails(self) -> None:
        self.header(HEADER.replace("// cerastes:trusted-contract vlibc: exit never returns (C23 7.24.4.4)\n"
                                   "void exit(int status) CERASTES_ENSURES(false);\n", ""))
        code, out, err = self.check()
        self.assertEqual(code, 1)
        self.assertIn("-exit | x.h |", err)

    def test_a_whitespace_only_change_of_the_declaration_passes(self) -> None:
        self.header(HEADER.replace("void exit(int status) CERASTES_ENSURES(false);",
                                   "void exit(int status)\n    CERASTES_ENSURES(false);"))
        self.assertEqual(self.check(), (0, "", ""))

    def test_a_header_error_fails_check(self) -> None:
        self.header(HEADER + "void z(void) CERASTES_ENSURES(false);\n")
        code, out, err = self.check()
        self.assertEqual(code, 1)
        self.assertIn("a contract without a cerastes:trusted-contract tag", err)

    def test_a_missing_committed_file_fails(self) -> None:
        code, out, err = run(["check", "--include-dir", str(self.inc), "--list", str(self.tmp / "none.txt")])
        self.assertEqual(code, 1)
        self.assertIn("cannot read the committed list", err)

    def test_an_empty_committed_file_fails(self) -> None:
        self.committed.write_text("# nothing\n")
        self.assertEqual(self.check()[0], 1)


class Names(Base):
    def test_output_is_sorted_and_exact(self) -> None:
        self.header(HEADER.replace("abort", "_Exit"))
        out = self.tmp / "gen" / "vlibc_names.inc"
        self.assertEqual(run(["names", "--include-dir", str(self.inc), "--out", str(out)])[0], 0)
        self.assertEqual(out.read_text(),
                         f"// Generated by scripts/vlibc_contracts.py from {self.inc}; do not edit.\n"
                         '{"_Exit", "x.h"},\n{"exit", "x.h"},\n')

    def test_unchanged_output_is_not_rewritten_and_changed_output_is(self) -> None:
        self.header(HEADER)
        out = self.tmp / "vlibc_names.inc"
        args = ["names", "--include-dir", str(self.inc), "--out", str(out)]
        self.assertEqual(run(args)[0], 0)
        old = (time.time() - 1000)
        os.utime(out, (old, old))
        before = out.stat().st_mtime_ns
        self.assertEqual(run(args)[0], 0)
        self.assertEqual(out.stat().st_mtime_ns, before, "an unchanged fragment must not be rewritten")
        # a second run produces the same bytes
        first = out.read_bytes()
        self.assertEqual(run(args)[0], 0)
        self.assertEqual(out.read_bytes(), first)
        # a new name rewrites it
        self.header(HEADER.replace("#endif", "// cerastes:trusted-contract vlibc: _Exit x\n"
                                             "void _Exit(int s) CERASTES_ENSURES(false);\n#endif"))
        self.assertEqual(run(args)[0], 0)
        self.assertNotEqual(out.stat().st_mtime_ns, before)
        self.assertEqual(out.read_text().splitlines()[1:],
                         ['{"_Exit", "x.h"},', '{"abort", "x.h"},', '{"exit", "x.h"},'])

    def test_a_header_error_writes_nothing_and_leaves_an_old_file(self) -> None:
        out = self.tmp / "vlibc_names.inc"
        out.write_text("old\n")
        self.header(HEADER + "void z(void) CERASTES_ENSURES(false);\n")
        code, _, err = run(["names", "--include-dir", str(self.inc), "--out", str(out)])
        self.assertEqual(code, 1)
        self.assertEqual(out.read_text(), "old\n")
        self.assertIn("a contract without", err)

    def test_each_name_carries_its_own_header_file_name(self) -> None:
        self.header(HEADER.replace("abort", "zzz"), "a.h")
        self.header(HEADER.replace("abort", "bbb").replace("exit", "yyy"), "b.h")
        out = self.tmp / "n.inc"
        self.assertEqual(run(["names", "--include-dir", str(self.inc), "--out", str(out)])[0], 0)
        self.assertEqual(out.read_text().splitlines()[1:],
                         ['{"bbb", "b.h"},', '{"exit", "a.h"},', '{"yyy", "b.h"},', '{"zzz", "a.h"},'])

    def test_only_tagged_names_are_reserved(self) -> None:
        self.header(HEADER + "void plain(void);\n")
        out = self.tmp / "n.inc"
        self.assertEqual(run(["names", "--include-dir", str(self.inc), "--out", str(out)])[0], 0)
        self.assertNotIn("plain", out.read_text())


if __name__ == "__main__":
    unittest.main(verbosity=1)
