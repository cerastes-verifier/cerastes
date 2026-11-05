#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Prove that a refactor of cerastes's tool code (src/, include/) changed no behaviour (session S7).

    scripts/object_identity.py --before <rev|dir> [--after <rev|dir>] [-j N] [--tu <path> ...]
                               [--tokens {off,none,braces}] [--no-compile]

Each side is exported into build/s7/<side>-<label>/tree/: src/, include/, cmake/ and every CMakeLists.txt (a rev by
`git archive`; a directory by copying them; the default `--after`, the working tree, by copying the files git lists
in it).  Every cerastes translation unit's command is taken from build/cmake/compile_commands.json (both sides use
the working tree's database and generated headers), with the repository root rewritten to the exported tree for
paths under src/ and include/ only, and compiled for both sides with the same compiler, three times with `-g0`
(what ships: no -DNDEBUG; nothing in src/ calls `assert` and no TU reads <assert.h>, measured):

  * an object: the build's flags, `-MD -MF <dep> -o <side>/<tu>.o`;
  * LLVM IR of the build (`-S -emit-llvm`, its -O0).  At -O0 machine code can hide a semantic change (`x - 1`
    becoming `x - 1U` on an `int` is the same instruction, `sub nsw` against `sub` in the IR); the IR keeps it;
  * the same IR with `-femit-all-decls`, which also emits inline functions nobody calls.

A TU is identical when its IR is identical, its all-decls IR is identical (see compare_extended for the one
leniency) and its object is byte-identical.  When only the object bytes differ the `objdump -d -r -s` text is
compared and the TU is reported as "identical by objdump".  EDG's assertion macros embed `__LINE__` of their use in
cerastes's own files (`assertion_failed(__FILE__, __LINE__, ...)`, 2 sites in encode.cpp): a reformat moves them.
Only the line argument of calls to `assertion_failed` whose first argument is a string naming a file of the tree is
normalised; such a TU is reported "identical modulo N line immediates" and its objects are NOT compared byte for
byte (they differ by the immediates; the summary names those TUs).

`--tokens none|braces` adds a token pass over every file of both exports, whatever the compile passes see: the token
streams of .cpp, .h, .hpp, .inc and .def files are compared, ignoring comments, whitespace and backslash-newline
splices, keeping directive boundaries (a directive is one logical line: a token moving into or out of one is a
difference) and not splitting `>>`.  `none` allows no difference; `braces` allows only inserted `{` and `}` tokens
(balanced; each `{` after `)`, `else` or `do`, each `}` after `;` or `}`).  Any other file (CMakeLists.txt,
cmake/*, *.in) must be byte-identical.  It finds token edits in code no build compiles (an inactive `#if` branch,
the other platform), compile-time-only code that gives the same IR (`static_assert`, `= delete`, `explicit`,
access specifiers, uninstantiated templates) and include/cerastes.h.  `--no-compile` runs it alone.

This is a proof, so it refuses to pass vacuously (exit status 2, "proof invalid"):
  1. after rewriting, no argument names <root>/src or <root>/include (but the one `-DCERASTES_SOURCE_INCLUDE_DIR`
     string, a path the tool reads at run time, which stays so that both sides carry the same string);
  2. the dependency file of every compile is read and each dependency must be on an allow-list: the side's own
     exported tree, third_party/edg, build/ (the generated headers) outside the scratch directory, or the
     toolchain (the compiler's implicit system search directories and its SDK root).  Anything else (the working
     tree's src/, the other side, the main checkout) is invalid, so both sides cannot agree vacuously;
  3. every file of either export that is neither a compiled .cpp nor a header some TU reads (CMakeLists.txt, cmake/*,
     *.in, an unread header) is listed; one that differs between the sides (or exists on one only) and is not on
     --allow-uncovered (default: include/cerastes.h, which only verified programs include and which the suite's
     .vpr/.map identity covers instead) is an error;
  4. every src/**/*.cpp of both sides appears in compile_commands.json and the number of cerastes TUs equals the
     expected constants (EXPECTED_TU_ENTRIES, EXPECTED_TU_FILES; a new TU makes the tool fail until they are
     updated with it).

Exit status: 0 every selected TU identical (and the token pass, if asked, clean); 1 a difference or a compile
failure; 2 the proof is not valid.
"""

import argparse
import concurrent.futures
import difflib
import filecmp
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent

# The cerastes TUs of build/cmake/compile_commands.json (entries under <root>/src/): 30 entries, 29 files
# (src/driver/edg_args.cpp is compiled into both the cerastes and the cerastes-fe target).
EXPECTED_TU_ENTRIES = 30
EXPECTED_TU_FILES = 29

# The define whose value is a path of the repository (read by the tool at run time, not a header read): it is not
# rewritten, so that both sides carry the same string.
KEEP_DEFINES = ("-DCERASTES_SOURCE_INCLUDE_DIR=",)

# Dropped from the recorded command: the output, the compile-only flag, and any dependency output.
DROP_WITH_ARG = ("-o", "-MF", "-MT", "-MQ")
DROP_PLAIN = ("-c", "-MD", "-MMD", "-M", "-MM", "-MG", "-MP")

TOKEN_SUFFIXES = (".cpp", ".h", ".hpp", ".inc", ".def")
DEFAULT_ALLOW_UNCOVERED = ("include/cerastes.h",)

WORKTREE = "<working tree>"


class ProofInvalid(Exception):
    """The comparison cannot be trusted (exit status 2)."""


# ---------------------------------------------------------------- the compile database


class TU:
    def __init__(self, entry, root):
        self.file = os.path.abspath(entry["file"])
        self.rel = os.path.relpath(self.file, root)
        self.directory = entry.get("directory") or root
        if "arguments" in entry:
            self.args = list(entry["arguments"])
        else:
            self.args = shlex.split(entry["command"])
        out = entry.get("output", "")
        m = re.search(r"CMakeFiles/([^/]+)\.dir/", out)
        self.target = m.group(1) if m else "tu"
        self.key = f"{self.target}/{self.rel}"
        self.stem = self.key.replace("/", "__")


def load_tus(cc_path, root):
    """The cerastes TUs: the entries whose file is under <root>/src/."""
    entries = json.loads(Path(cc_path).read_text())
    prefix = os.path.join(os.path.realpath(root), "src") + os.sep
    tus = []
    for e in entries:
        f = os.path.realpath(e["file"])
        if f.startswith(prefix):
            e = dict(e)
            e["file"] = f
            tus.append(TU(e, os.path.realpath(root)))
    keys = [t.key for t in tus]
    if len(set(keys)) != len(keys):
        raise ProofInvalid("two compile_commands entries have the same target and file")
    return tus


# ---------------------------------------------------------------- command rewriting


def _root_pattern(root):
    return re.compile(re.escape(root) + r"/(src|include)(?=[/\"'\s]|$)")


def rewrite_args(args, root, tree):
    """The TU's command for one side: <root>/src and <root>/include become <tree>/src and <tree>/include, the
    output and dependency options are dropped.  Raises ProofInvalid when an argument still names the root's src
    or include afterwards (check 1)."""
    pat = _root_pattern(root)
    out = []
    i = 0
    while i < len(args):
        a = args[i]
        if a in DROP_WITH_ARG:
            i += 2
            continue
        if a in DROP_PLAIN:
            i += 1
            continue
        if a.startswith(KEEP_DEFINES):
            out.append(a)
        else:
            out.append(pat.sub(lambda m: f"{tree}/{m.group(1)}", a))
        i += 1
    assert_no_root_refs(out, root)
    return out


def assert_no_root_refs(args, root):
    """Check 1: no argument names <root>/src or <root>/include (but the keep-listed define)."""
    pat = _root_pattern(root)
    for a in args:
        if a.startswith(KEEP_DEFINES):
            continue
        if pat.search(a):
            raise ProofInvalid(f"an argument still names {root}/src or {root}/include after rewriting: {a}")


# ---------------------------------------------------------------- exporting a side


def _git(root, *a, **kw):
    return subprocess.run(["git", "-C", str(root), *a], check=True, capture_output=True, **kw)


EXTRA_TOPS = ("cmake",)
SKIP_DIRS = ("build", "third_party", ".git", ".claude", "node_modules", "tests", "doc")


def _is_input(path):
    """The files of an export: src/, include/, cmake/ and any CMakeLists.txt."""
    return path.split("/")[0] in ("src", "include") + EXTRA_TOPS or os.path.basename(path) == "CMakeLists.txt"


def export_side(spec, root, dest):
    """Export the inputs of `spec` (src/, include/, cmake/, CMakeLists.txt files) into dest (a fresh directory).
    Returns the label."""
    if dest.exists():
        shutil.rmtree(dest)
    dest.mkdir(parents=True)
    if spec == WORKTREE:
        # the tracked files as they are in the working tree, plus untracked and not ignored ones (a new header
        # must be seen: a missing one is a compile error, never a silent fallback)
        names = _git(root, "ls-files", "-z", "--cached", "--others", "--exclude-standard").stdout.split(b"\0")
        for n in names:
            if not n:
                continue
            n = n.decode()
            s = Path(root) / n
            if _is_input(n) and s.is_file():
                (dest / n).parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(s, dest / n)
        return "worktree"
    if os.path.isdir(spec):
        for top in ("src", "include") + EXTRA_TOPS:
            s = Path(spec) / top
            if s.is_dir():
                shutil.copytree(s, dest / top)
        for dirpath, dirnames, filenames in os.walk(spec):
            dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
            if "CMakeLists.txt" in filenames:
                rel = os.path.relpath(os.path.join(dirpath, "CMakeLists.txt"), spec)
                (dest / rel).parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(os.path.join(dirpath, "CMakeLists.txt"), dest / rel)
        return re.sub(r"[^A-Za-z0-9._-]", "_", os.path.basename(os.path.abspath(spec)))
    try:
        sha = _git(root, "rev-parse", "--verify", spec + "^{commit}").stdout.decode().strip()
    except subprocess.CalledProcessError:
        raise ProofInvalid(f"{spec!r} is neither a directory nor a revision")
    listed = _git(root, "ls-tree", "-r", "-z", "--name-only", sha).stdout.decode().split("\0")
    paths = [n for n in listed if n and _is_input(n)]
    arch = subprocess.run(["git", "-C", str(root), "archive", "--format=tar", sha, "--", *paths],
                          check=True, capture_output=True).stdout
    subprocess.run(["tar", "-x", "-C", str(dest)], input=arch, check=True)
    return re.sub(r"[^A-Za-z0-9._-]", "_", spec) + "-" + sha[:8]


# ---------------------------------------------------------------- compiling


def compiler_is_clang(cxx):
    try:
        v = subprocess.run([cxx, "--version"], capture_output=True, text=True).stdout
    except OSError:
        return False, ""
    return "clang" in v.lower(), v.splitlines()[0] if v else ""


def run_cmd(argv, cwd):
    t = time.time()
    r = subprocess.run(argv, cwd=cwd, capture_output=True, text=True, errors="replace")
    return r, time.time() - t


def path_flags(root, tree):
    """`__FILE__` of an exported source is mapped to the repository path, so that both sides spell it alike."""
    return [f"-fmacro-prefix-map={tree}={root}"]


def compile_job(side, tu, kind, base, out_dir, cwd):
    """kind is 'obj', 'ir' (the build's IR) or 'ira' (the same with -femit-all-decls).  Returns (ok, stderr, seconds).
    `base` is the rewritten command plus path_flags."""
    d = out_dir / tu.target
    d.mkdir(parents=True, exist_ok=True)
    dep = ["-g0", "-MD", "-MF", str(d / (tu.stem + "." + kind + ".d")), "-o"]
    if kind == "obj":
        argv = base + ["-c"] + dep + [str(d / (tu.stem + ".o"))]
    elif kind == "ir":
        argv = base + ["-S", "-emit-llvm"] + dep + [str(d / (tu.stem + ".ll"))]
    else:
        argv = base + ["-S", "-emit-llvm", "-femit-all-decls"] + dep + [str(d / (tu.stem + ".all.ll"))]
    r, dt = run_cmd(argv, cwd)
    return r.returncode == 0, r.stderr[-3000:], dt


def normalize_ir(text):
    return "".join(l for l in text.splitlines(keepends=True)
                   if not l.startswith("; ModuleID") and not l.startswith("source_filename"))


# ---------------------------------------------------------------- dependency files (check 2)


def parse_depfile(text):
    text = text.replace("\\\n", " ")
    _, _, rest = text.partition(": ")
    deps, cur, i = [], "", 0
    while i < len(rest):
        c = rest[i]
        if c == "\\" and i + 1 < len(rest) and rest[i + 1] in " #":
            cur += rest[i + 1]
            i += 2
            continue
        if c.isspace():
            if cur:
                deps.append(cur)
            cur = ""
        else:
            cur += c
        i += 1
    if cur:
        deps.append(cur)
    return deps


def search_dirs(args, tu_file):
    """The compiler's implicit system search directories for a command (`-E -v`), and the SDK roots they lie in.  The
    directories the command names itself (-I, -isystem, ...) are not among them: they are the project's, and are
    classified by where they are."""
    probe = [a for a in args if a != tu_file and not a.startswith("-D")]
    r = subprocess.run(probe + ["-E", "-v", "-x", "c++", "-"], input="", capture_output=True, text=True)
    lines, on = [], False
    for l in r.stderr.splitlines():
        if l.startswith("#include <...> search starts here"):
            on = True
        elif l.startswith("End of search list"):
            on = False
        elif on:
            lines.append(re.sub(r" \((framework directory|headermap)\)$", "", l.strip()))
    explicit = set()
    for i, a in enumerate(probe):
        for opt in ("-I", "-isystem", "-iquote", "-idirafter", "-F", "-iframework"):
            if a == opt and i + 1 < len(probe):
                explicit.add(os.path.realpath(probe[i + 1]))
            elif a.startswith(opt) and len(a) > len(opt):
                explicit.add(os.path.realpath(a[len(opt):]))
    out = set()
    for d in lines:
        rd = os.path.realpath(d)
        if rd in explicit:
            continue
        out.add(rd)
        m = re.match(r"(.*?\.sdk)(/|$)", rd)
        if m:
            out.add(m.group(1))
    if not out:
        raise ProofInvalid(f"cannot read the system search directories of {probe[0]} (-E -v printed none)")
    return sorted(out)


def check_deps(deps, tu_file_in_tree, root, tree, scratch, toolchain):
    """Classify one TU's dependencies against an allow-list.  Allowed: the side's own tree (these are returned as
    `read`, relative to the tree), <root>/third_party/edg, <root>/build outside the scratch directory (returned as
    `generated`), and the toolchain's directories (`toolchain`, realpaths).  Everything else is a violation: the
    working tree's src/ and include/, the other side, another checkout, any other file."""
    rroot = os.path.realpath(root)
    rtree = os.path.realpath(tree)
    rscratch = os.path.realpath(scratch)

    def under(p, d):
        return p == d or p.startswith(d + os.sep)

    read, viol, gen = set(), [], set()
    for d in deps:
        p = os.path.realpath(os.path.join(rroot, d) if not os.path.isabs(d) else d)
        if under(p, rtree):
            read.add(os.path.relpath(p, rtree))
        elif under(p, rscratch):
            viol.append(p)
        elif under(p, rroot):
            if under(p, os.path.join(rroot, "third_party", "edg")):
                continue
            if under(p, os.path.join(rroot, "build")):
                gen.add(p)
            else:
                viol.append(p)
        elif not any(under(p, t) for t in toolchain):
            viol.append(p)
    # the TU itself must be the exported file (clang may list other files, SDKSettings.json, before it)
    if os.path.relpath(os.path.realpath(tu_file_in_tree), rtree) not in read:
        viol.append(f"the exported {tu_file_in_tree} is not among the dependencies of its own compile")
    return read, viol, gen


# ---------------------------------------------------------------- the comparison


def find_objdump():
    for n in ("llvm-objdump",):
        p = shutil.which(n)
        if p:
            return [p]
    try:
        p = subprocess.run(["xcrun", "-f", "llvm-objdump"], capture_output=True, text=True).stdout.strip()
        if p:
            return [p]
    except OSError:
        pass
    for n in sorted(Path("/usr/bin").glob("llvm-objdump-*"), reverse=True):
        return [str(n)]
    p = shutil.which("objdump")
    return [p] if p else None


def objdump_text(objdump, obj):
    r = subprocess.run(objdump + ["-d", "-r", "-s", obj.name], cwd=obj.parent, capture_output=True, text=True,
                       errors="replace")
    # the first lines name the file; the directories differ between the sides
    return r.returncode, "\n".join(l for l in r.stdout.splitlines() if "file format" not in l)


def short_diff(a, b, name, n=12):
    d = list(difflib.unified_diff(a.splitlines(), b.splitlines(), "before/" + name, "after/" + name, n=1,
                                  lineterm=""))
    return d


def normalize_lines(ir, root):
    """EDG's assertion macros (`is_typeref_kind` with EXPENSIVE_CHECKING, `check_assertion`) pass `__FILE__` and
    `__LINE__` of their use to `assertion_failed`, so a use in a cerastes file embeds that file's line numbers:
    reformatting moves them.  The `i32` line argument of a call whose first argument is a string constant that holds
    a path under <root>/src or <root>/include (the exported file, mapped back by -fmacro-prefix-map) becomes 0, for
    calls of `assertion_failed` only (EDG's, third_party/edg/src/checking.h: `_Z16assertion_failedPKciS0_S0_S0_`):
    `log(__FILE__, __LINE__)`, `assertion_failed` called with another string, and the calls of EDG's own headers
    keep their numbers.  Returns (text, number of sites)."""
    own = set(m.group(1) for m in re.finditer(
        r'^(@\.str[.\d]*) = [^\n]*c"' + re.escape(root) + r'/(?:src|include)/[^"\\]*\\00"', ir, re.M))
    n = 0

    def sub(m):
        nonlocal n
        if m.group(2) in own:
            n += 1
            return m.group(1) + "0"
        return m.group(0)

    out = re.sub(r"(\b(?:call|invoke)\b[^\n]*?@(?:_Z\d+)?assertion_failed\w*\(ptr (?:noundef )?(@\.str[.\d]*), i32 (?:noundef )?)(\d+)", sub, ir)
    return out, n


STD_NAME = re.compile(r"^@_Z[A-Z]{0,4}St")


def _fix_line(l, attrs, strs):
    parts = re.split(r'(c"(?:[^"\\]|\\.)*")', l)
    for i in range(0, len(parts), 2):
        p = parts[i]
        p = re.sub(r" #(\d+)\b", lambda m: " #{" + attrs.get(m.group(1), "?") + "}", p)
        p = re.sub(r"@\.str(?:\.\d+)?\b", lambda m: "@str{" + strs.get(m.group(0), "?") + "}", p)
        p = re.sub(r"!\d+\b", "!#", p)
        parts[i] = p
    return "".join(parts)


def ir_entities(text):
    """The top-level entities of a module (functions, declarations, globals by name; type definitions), with
    attribute-group, metadata and string-constant numbers replaced by what they stand for, so that adding or removing
    an unrelated entity does not renumber the others.  Returns (kinds, bodies, misc).  Dropped, deliberately: blank
    lines, `;` comments, `attributes #N = ...` groups (resolved into their users), `source_filename`, numbered
    metadata definitions `!N = ...` (their numbers are erased in the users; the plain-IR pass, which is leniency-free,
    compares them) and string constants `@.str*` (compared by content in their users).  Everything else at top level
    that is not a function, declaration, global or type (target triple, datalayout, `module asm`, comdats, named
    metadata such as !llvm.module.flags) goes to `misc` and is compared."""
    lines = text.splitlines()
    attrs, strs = {}, {}
    for l in lines:
        m = re.match(r"attributes #(\d+) = (.*)$", l)
        if m:
            attrs[m.group(1)] = m.group(2)
        m = re.match(r'(@\.str(?:\.\d+)?) = .*? (c"(?:[^"\\]|\\.)*")', l)
        if m:
            strs[m.group(1)] = m.group(2)
    kinds, bodies, misc = {}, {}, []
    name_re = re.compile(r'@("[^"]+"|[\w.$]+)\(')
    i = 0
    while i < len(lines):
        l = lines[i]
        i += 1
        if l.startswith("define "):
            m = name_re.search(l)
            name, body = "@" + m.group(1), [_fix_line(l, attrs, strs)]
            while i < len(lines) and lines[i] != "}":
                body.append(_fix_line(lines[i], attrs, strs))
                i += 1
            i += 1
            kinds[name], bodies[name] = "define", "\n".join(body)
        elif l.startswith("declare "):
            m = name_re.search(l)
            kinds["@" + m.group(1)] = "declare"
            bodies["@" + m.group(1)] = _fix_line(l, attrs, strs)
        elif l.startswith("@"):
            if re.match(r"@\.str(\.\d+)? =", l):
                continue
            name = l.split(" ", 1)[0]
            kinds[name], bodies[name] = "global", _fix_line(l, attrs, strs)
        elif l.startswith("%"):
            name = l.split(" ", 1)[0]
            kinds[name], bodies[name] = "type", _fix_line(l, attrs, strs)
        elif l == "" or l.startswith(("attributes ", ";", "source_filename")) or re.match(r"!\d+ = ", l):
            continue
        else:
            misc.append(_fix_line(l, attrs, strs))
    return kinds, bodies, sorted(misc)


def compare_extended(tb, ta):
    """Compare two -femit-all-decls modules.  Returns (fatal reasons, number of library-only entities).
    Fatal: module-level text differs; an entity on both sides with different text; a function or global on one
    side only that is not a libc++ (std) instantiation.  Not fatal: std instantiations on one side only (a literal
    that changes type, 0 to 0U, instantiates `std::is_integral_v<unsigned>` where it was `<int>`: a name, no
    behaviour, and any code that uses it differs by that name in the plain IR), declarations and types on one side
    only (their users differ), string constants (compared by content in their users)."""
    kb, bb, mb = ir_entities(tb)
    ka, ba, ma = ir_entities(ta)
    fatal, noise = [], 0
    if mb != ma:
        fatal.append("module-level text differs")
    for n in sorted(set(kb) & set(ka)):
        if bb[n] != ba[n]:
            fatal.append(f"{n} differs")
    for side, only, kinds in (("before", set(kb) - set(ka), kb), ("after", set(ka) - set(kb), ka)):
        for n in sorted(only):
            if kinds[n] in ("declare", "type"):
                continue
            if STD_NAME.match(n):
                noise += 1
            else:
                fatal.append(f"{n} exists only {side}")
    return fatal, noise



def compare_tu(tu, sides, kinds, scratch, root, cmds, results):
    entry = {"tu": tu.key, "obj": None, "ir": None, "ir_ext": None, "ext_noise": 0, "objdump": None,
             "verdict": None, "notes": [], "line_sites": 0, "obj_compared": True}
    out = {n: sides[n]["out"] / tu.target for n in sides}
    for k in kinds:
        for name in sides:
            ok, err, _ = results[(name, tu.key, k)]
            if not ok:
                entry["notes"].append(f"{name} {k} compile failed:\n" + err)
    if entry["notes"]:
        entry["verdict"] = "COMPILE-FAILED"
        return entry
    ob, oa = (out[n] / (tu.stem + ".o") for n in ("before", "after"))
    entry["obj"] = filecmp.cmp(ob, oa, shallow=False)
    entry["ir"] = True
    modulo = False
    if "ir" in kinds:
        lb, la = (out[n] / (tu.stem + ".ll") for n in ("before", "after"))
        tb, ta = (normalize_ir(x.read_text(errors="replace")) for x in (lb, la))
        entry["ir_bytes"] = len(tb)
        if tb != ta:
            nb, sb = normalize_lines(tb, root)
            na, sa = normalize_lines(ta, root)
            if nb == na and sb == sa and sb:
                modulo = True
                entry["line_sites"] = sb
            else:
                entry["ir"] = False
                d = short_diff(tb, ta, tu.stem + ".ll")
                dd = scratch / "diffs"
                dd.mkdir(exist_ok=True)
                (dd / (tu.stem + ".ll.diff")).write_text("\n".join(d) + "\n")
                entry["notes"].append(f"IR differs ({len(d)} diff lines): {dd / (tu.stem + '.ll.diff')}")
                entry["notes"] += d[:14]
        else:
            entry["line_sites"] = normalize_lines(tb, root)[1]
    if "ira" in kinds:
        xb, xa = (normalize_ir((out[n] / (tu.stem + ".all.ll")).read_text(errors="replace")) for n in
                  ("before", "after"))
        entry["ir_ext_bytes"] = len(xb)
        entry["ir_ext"] = True
        if xb != xa:
            fatal, noise = compare_extended(normalize_lines(xb, root)[0], normalize_lines(xa, root)[0])
            entry["ext_noise"] = noise
            if fatal:
                entry["ir_ext"] = False
                d = short_diff(xb, xa, tu.stem + ".all.ll")
                dd = scratch / "diffs"
                dd.mkdir(exist_ok=True)
                (dd / (tu.stem + ".all.ll.diff")).write_text("\n".join(d) + "\n")
                entry["notes"].append(f"-femit-all-decls IR differs: {len(fatal)} entities, e.g. {fatal[:4]}; "
                                      f"{dd / (tu.stem + '.all.ll.diff')}")
    if not entry["ir"] or entry["ir_ext"] is False:
        entry["verdict"] = "DIFFERS"
        return entry
    if modulo:
        # the object bytes differ by the line immediates of those sites and cannot be compared; the IR was
        entry["obj_compared"] = False
        entry["verdict"] = "identical-modulo-lines"
        entry["notes"].append(f"identical modulo {entry['line_sites']} line immediates of EDG assertion sites in this "
                              f"TU's own sources (IR compared with them normalised; object bytes differ and are not "
                              f"compared)")
        return entry
    if entry["obj"]:
        entry["verdict"] = "identical"
        return entry
    objdump = find_objdump()
    if objdump:
        rb, xb = objdump_text(objdump, ob)
        ra, xa = objdump_text(objdump, oa)
        entry["objdump"] = rb == 0 and ra == 0 and xb == xa
    else:
        entry["notes"].append("no objdump found: bytes differ and cannot be compared otherwise")
    entry["verdict"] = "identical-by-objdump" if entry["objdump"] else "DIFFERS"
    return entry


# ---------------------------------------------------------------- the token pass (--tokens)

_PUNCT = ["%:%:", "...", "<=>", "->*", "<<=", ">>=", "::", "->", "++", "--", "<<", ">>", "<=", ">=", "==", "!=", "&&",
          "||", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", ".*", "##"]
_TOKEN = re.compile(r"""
 (?P<ws>[ \t\f\v]+)
|(?P<nl>\n)
|(?P<lc>//[^\n]*)
|(?P<bc>/\*.*?\*/)
|(?P<raw>(?:u8|u|U|L)?R"(?P<d>[^ ()\\\t\v\f\n]{0,16})\((?:.*?)\)(?P=d)")
|(?P<str>(?:u8|u|U|L)?"(?:[^"\\\n]|\\.)*"(?:[A-Za-z_]\w*)?)
|(?P<chr>(?:u8|u|U|L)?'(?:[^'\\\n]|\\.)*')
|(?P<num>\.?\d(?:[eEpP][+-]|[\w.']|)*)
|(?P<id>[A-Za-z_]\w*)
|(?P<p>""" + "|".join(re.escape(p) for p in _PUNCT) + r"""|[^\sA-Za-z_0-9])
""", re.S | re.X)
EOD = "<end of directive>"


def tokenize(text):
    """The preprocessing tokens of a C++ file in translation phases 2 and 3: backslash-newline splices removed (a
    `//` comment continued by one swallows the next line), comments and whitespace dropped, every directive closed by
    an EOD token at its logical line's end.  `>>` stays one token.  Raises ProofInvalid on text it cannot lex."""
    text = text.replace("\r\n", "\n")
    # splices: removed everywhere; their positions in the result tell whether one lay inside a raw string literal,
    # where C++ reverts the splice and this lexer would be wrong
    cuts = [m.start() - 2 * k for k, m in enumerate(re.finditer(r"\\\n", text))]
    text = text.replace("\\\n", "")
    out, i, at_bol, in_dir = [], 0, True, False
    while i < len(text):
        m = _TOKEN.match(text, i)
        if not m:
            raise ProofInvalid(f"the token pass cannot lex {text[i:i + 40]!r}")
        s = m.group(0)
        if m.group("raw") and any(i < c < m.end() for c in cuts):
            raise ProofInvalid("a backslash-newline inside a raw string literal: the token pass does not model it")
        i = m.end()
        if m.group("ws") or m.group("lc") or m.group("bc"):
            continue
        if m.group("nl"):
            if in_dir:
                out.append(EOD)
                in_dir = False
            at_bol = True
            continue
        if at_bol and s == "#":
            in_dir = True
        at_bol = False
        out.append(s)
    if in_dir:
        out.append(EOD)
    return out


def compare_tokens(ta, tb, mode):
    """Compare two token streams.  Returns (problems, inserted braces).  `none` allows no difference; `braces` allows
    inserted `{` and `}` tokens only, as many of one as of the other, each `{` after `)`, `else` or `do` (the places
    clang-format's InsertBraces puts them) and each `}` after `;` or `}`."""
    if ta == tb:
        return [], 0
    i = j = 0
    opens = closes = 0
    while i < len(ta) or j < len(tb):
        if i < len(ta) and j < len(tb) and ta[i] == tb[j]:
            i += 1
            j += 1
            continue
        ctx = f"before: {' '.join(ta[max(0, i - 6):i + 4])!r}  after: {' '.join(tb[max(0, j - 6):j + 4])!r}"
        if mode == "braces" and j < len(tb) and tb[j] in ("{", "}"):
            prev = tb[j - 1] if j else ""
            if tb[j] == "{" and prev not in (")", "else", "do"):
                return [f"an inserted `{{` after {prev!r}: {ctx}"], opens + closes
            if tb[j] == "}" and prev not in (";", "}"):
                return [f"an inserted `}}` after {prev!r}: {ctx}"], opens + closes
            opens += tb[j] == "{"
            closes += tb[j] == "}"
            j += 1
            continue
        return [f"token {i} / {j}: {ctx}"], opens + closes
    if opens != closes:
        return [f"{opens} `{{` and {closes} `}}` inserted"], opens + closes
    return [], opens + closes


def token_pass(sides, mode):
    """Compare every file of the two exports.  Returns {relative path: {"status", "inserted", "detail"}}; status is
    "same", "braces" (only inserted braces, in `braces` mode) or "DIFFERS"."""
    trees = {n: Path(s["tree"]) for n, s in sides.items()}
    files = {n: {str(p.relative_to(t)) for p in t.rglob("*") if p.is_file()} for n, t in trees.items()}
    result = {}
    for f in sorted(set().union(*files.values())):
        gone = [n for n in trees if f not in files[n]]
        if gone:
            result[f] = {"status": "DIFFERS", "inserted": 0, "detail": f"only on {'/'.join(n for n in trees if n not in gone)}"}
            continue
        a, b = (trees[n] / f for n in ("before", "after"))
        if f.endswith(TOKEN_SUFFIXES):
            ta = tokenize(a.read_text(errors="surrogateescape"))
            tb = tokenize(b.read_text(errors="surrogateescape"))
            problems, ins = compare_tokens(ta, tb, mode)
            if problems:
                result[f] = {"status": "DIFFERS", "inserted": ins, "detail": problems[0]}
            else:
                result[f] = {"status": "braces" if ins else "same", "inserted": ins, "detail": ""}
        elif filecmp.cmp(a, b, shallow=False):
            result[f] = {"status": "same", "inserted": 0, "detail": ""}
        else:
            result[f] = {"status": "DIFFERS", "inserted": 0, "detail": "not a C++ source file and not byte-identical"}
    return result


def report_tokens(res, mode):
    bad = [f for f, r in res.items() if r["status"] == "DIFFERS"]
    for f, r in res.items():
        if r["status"] == "DIFFERS":
            print(f"  TOKENS DIFFER  {f}: {r['detail']}")
        elif r["status"] == "braces":
            print(f"  tokens ok      {f}: {r['inserted']} inserted brace tokens, nothing else")
    ins = sum(r["inserted"] for r in res.values() if r["status"] == "braces")
    print(f"tokens ({mode}): {len(res)} files compared: {sum(r['status'] == 'same' for r in res.values())} identical, "
          f"{sum(r['status'] == 'braces' for r in res.values())} with only inserted braces ({ins} tokens), "
          f"{len(bad)} DIFFER")
    return len(bad)


def run(opts):
    root = os.path.realpath(opts.root)
    cc_path = opts.compile_commands or os.path.join(root, "build/cmake/compile_commands.json")
    # absolute: the compiles run from each entry's own directory, so a relative --scratch would name the wrong place
    scratch = Path(os.path.abspath(opts.scratch or os.path.join(root, "build/s7")))
    scratch.mkdir(parents=True, exist_ok=True)
    if opts.no_compile and opts.tokens == "off":
        raise ProofInvalid("--no-compile with --tokens off compares nothing")

    t0 = time.time()
    tus, files, selected = [], [], []
    if not opts.no_compile:
        if not os.path.isfile(cc_path):
            raise ProofInvalid(f"{cc_path} does not exist (run scripts/bootstrap.sh)")
        tus = load_tus(cc_path, root)
        files = sorted({t.rel for t in tus})
        # check 4, the constants
        if len(tus) != opts.expected_tus or len(files) != opts.expected_files:
            raise ProofInvalid(
                f"{cc_path} has {len(tus)} cerastes TU entries over {len(files)} files, expected {opts.expected_tus} "
                f"over {opts.expected_files} (EXPECTED_TU_ENTRIES/EXPECTED_TU_FILES in scripts/object_identity.py: a "
                f"new or removed TU must be added there, with its reason)")
        selected = tus
        if opts.tu:
            selected = []
            for want in opts.tu:
                w = want.strip("/")
                hit = [t for t in tus if t.rel == w or t.key == w or t.rel.endswith("/" + w)]
                if not hit:
                    raise ProofInvalid(f"--tu {want}: no cerastes TU of that name (see {cc_path})")
                selected += [t for t in hit if t not in selected]

    sides = {}
    for name, spec in (("before", opts.before), ("after", opts.after)):
        tree_tmp = scratch / f"{name}.exporting"
        label = export_side(spec, root, tree_tmp)
        final = scratch / f"{name}-{label}"
        if final.exists():
            shutil.rmtree(final)
        tree = final / "tree"
        tree.parent.mkdir(parents=True)
        tree_tmp.rename(tree)
        sides[name] = {"spec": spec, "label": label, "dir": final, "tree": str(os.path.realpath(tree)),
                       "out": final / "out"}
    print(f"object_identity: before: {opts.before} -> {sides['before']['dir']}", flush=True)
    print(f"                 after:  {opts.after} -> {sides['after']['dir']}", flush=True)

    # the token pass
    tok_res, tok_bad = None, 0
    if opts.tokens != "off":
        tok_res = token_pass(sides, opts.tokens)
        tok_bad = report_tokens(tok_res, opts.tokens)
        if opts.json and opts.no_compile:
            Path(opts.json).write_text(json.dumps({"tokens": tok_res}, indent=1))
    if opts.no_compile:
        return 1 if tok_bad else 0

    # check 4: the exported .cpp files and the database agree, on both sides
    for name, s in sides.items():
        have = {str(p.relative_to(s["tree"])) for p in Path(s["tree"], "src").rglob("*.cpp")} \
            if Path(s["tree"], "src").is_dir() else set()
        if have - set(files):
            raise ProofInvalid(f"{name}: {sorted(have - set(files))} are tracked .cpp files of src/ that "
                               f"{cc_path} does not compile (their edits would not be covered)")
        if set(files) - have:
            raise ProofInvalid(f"{name}: {sorted(set(files) - have)} are in {cc_path} but not in the exported tree")

    # check 1: the rewritten commands
    cmds = {}
    for name, s in sides.items():
        for tu in selected:
            args = rewrite_args(tu.args, root, s["tree"])
            if not args:
                raise ProofInvalid("empty command")
            cmds[(name, tu.key)] = args + path_flags(root, s["tree"])
    cxx = selected[0].args[0]
    is_clang, banner = compiler_is_clang(cxx)
    if not is_clang and not opts.no_ir:
        raise ProofInvalid(f"{cxx} is not clang ({banner!r}): the IR pass needs `-emit-llvm` (or pass --no-ir, which "
                           f"weakens the proof)")
    cwd_of = {tu.key: (tu.directory if os.path.isdir(tu.directory) else root) for tu in selected}
    kinds = ("obj",) if opts.no_ir else ("obj", "ir") if opts.no_emit_all_decls else ("obj", "ir", "ira")
    jobs = [(name, tu, kind) for name in sides for tu in selected for kind in kinds]
    print(f"object_identity: {len(selected)} of {len(tus)} TUs, 2 sides, {len(jobs)} compiles, -j{opts.jobs}; "
          f"compiler: {banner}", flush=True)
    # the toolchain's directories, per distinct command (compiler and flags), for the dependency allow-list
    toolchain = {}
    for (name, key), args in cmds.items():
        tu = next(t for t in selected if t.key == key)
        ck = tuple(a for a in args if not a.startswith("-D") and a != os.path.join(sides[name]["tree"], tu.rel)
                   and not a.startswith("-fmacro-prefix-map"))
        if ck not in toolchain:
            toolchain[ck] = search_dirs(args, os.path.join(sides[name]["tree"], tu.rel))
        toolchain[(name, key)] = toolchain[ck]

    results = {}
    t1 = time.time()
    with concurrent.futures.ThreadPoolExecutor(max_workers=opts.jobs) as ex:
        futs = {}
        for name, tu, kind in jobs:
            s = sides[name]
            # the IR compile's dependency file is its own, but the object's is the one that is checked too
            futs[ex.submit(compile_job, name, tu, kind, cmds[(name, tu.key)], s["out"], cwd_of[tu.key])] = (
                name, tu.key, kind)
        for f in concurrent.futures.as_completed(futs):
            results[futs[f]] = f.result()
    t_compile = time.time() - t1

    # check 2: dependency files
    violations, read_by_side, generated = [], {n: set() for n in sides}, set()
    for name, s in sides.items():
        for tu in selected:
            in_tree = os.path.join(s["tree"], tu.rel)
            for kind in kinds:
                if not results[(name, tu.key, kind)][0]:
                    continue
                dp = s["out"] / tu.target / (tu.stem + "." + kind + ".d")
                deps = parse_depfile(dp.read_text())
                read, viol, gen = check_deps(deps, in_tree, root, s["tree"], scratch, toolchain[(name, tu.key)])
                read_by_side[name] |= read
                generated |= gen
                violations += [f"{name} {tu.key} ({kind}): {v}" for v in viol]
    if violations:
        raise ProofInvalid("dependencies outside the allow-list (the side's own tree, third_party/edg, build/, the "
                           "toolchain: the working tree, the other side or another checkout would let both sides "
                           "agree vacuously):\n  " + "\n  ".join(violations[:20]))

    # the generated headers: do they include src/ headers?
    gen_inc = {}
    for g in sorted(generated):
        try:
            txt = Path(g).read_text(errors="replace")
        except OSError:
            continue
        for m in re.finditer(r'^\s*#\s*include\s*["<]([^">]+)[">]', txt, re.M):
            nm = m.group(1)
            if "src/" in nm or "include/" in nm or any(os.path.basename(nm) == os.path.basename(h)
                                                         for h in read_by_side["after"]):
                gen_inc.setdefault(os.path.relpath(g, root), []).append(nm)

    # compare
    report = []
    n_ident = n_objdump = n_lines = n_diff = n_fail = 0
    ir_bytes = ext_bytes = 0
    ext_noise = {}
    line_sites = {}
    not_byte_compared = []
    for tu in selected:
        entry = compare_tu(tu, sides, kinds, scratch, root, cmds, results)
        ir_bytes += entry.pop("ir_bytes", 0)
        ext_bytes += entry.pop("ir_ext_bytes", 0)
        ext_noise[tu.key] = entry["ext_noise"]
        if entry.get("line_sites"):
            line_sites[tu.key] = entry["line_sites"]
        v = entry["verdict"]
        if not entry["obj_compared"]:
            not_byte_compared.append(tu.key)
        n_ident += v == "identical"
        n_objdump += v == "identical-by-objdump"
        n_lines += v == "identical-modulo-lines"
        n_diff += v == "DIFFERS"
        n_fail += v == "COMPILE-FAILED"
        report.append(entry)

    # check 3: files no compile reads (CMakeLists.txt, cmake/*, *.in, unread headers)
    unread_msgs, uncovered_err = [], []
    full = not opts.tu and not n_fail
    if full:
        allow = set(opts.allow_uncovered)
        tracked = {}
        for name, sd in sides.items():
            tr = Path(sd["tree"])
            tracked[name] = {str(p.relative_to(tr)) for p in tr.rglob("*") if p.is_file()}
        compiled = set(files)
        for h in sorted(set().union(*tracked.values())):
            if h in compiled:
                continue
            unread_by = [n for n in sides if h in tracked[n] and h not in read_by_side[n]]
            missing = [n for n in sides if h not in tracked[n]]
            if not unread_by and not (missing and any(h in read_by_side[n] for n in sides)):
                continue
            same = (not missing and filecmp.cmp(Path(sides["before"]["tree"]) / h, Path(sides["after"]["tree"]) / h,
                                                shallow=False))
            tag = "unchanged" if same else ("CHANGED" if not missing else f"absent on {missing}")
            unread_msgs.append(f"{h} ({tag})")
            if not same and h not in allow:
                uncovered_err.append(h)
    else:
        unread_msgs.append("(not computed: --tu selects a subset, or a TU failed to compile)")

    # output
    for e in report:
        parts = []
        if e["obj"] is not None:
            parts.append("obj=" + ("same" if e["obj"] else "DIFF" + ("(objdump same)" if e["objdump"] else "")))
        if e["ir"] is not None and "ir" in kinds:
            parts.append("ir=" + ("same" if e["ir"] else "DIFF"))
        if e["ir_ext"] is not None:
            parts.append("all-decls=" + ("same" if e["ir_ext"] else "DIFF"))
        print(f"  {e['verdict']:<22} {e['tu']}  {' '.join(parts)}")
        if e["verdict"] != "identical":
            for n in e["notes"]:
                for ln in n.splitlines()[:16]:
                    print(f"      {ln}")
    print(f"summary: {len(report)} TUs: {n_ident} identical, {n_lines} identical modulo the lines of EDG assertion "
          f"sites in cerastes's own files, {n_objdump} identical by objdump only (object bytes differ, IR and "
          f"disassembly agree), {n_diff} DIFFER, {n_fail} failed to compile")
    print(f"objects compared byte for byte: {len(report) - len(not_byte_compared)} of {len(report)} TUs"
          + (f"; NOT compared (they differ by line immediates, the IR was compared): {', '.join(not_byte_compared)}"
             if not_byte_compared else ""))
    print(f"timing: compiles {t_compile:.1f} s, total {time.time() - t0:.1f} s; IR {ir_bytes / 1e6:.1f} MB per side"
          + ("" if opts.no_emit_all_decls or opts.no_ir else f", with -femit-all-decls {ext_bytes / 1e6:.1f} MB "
             f"({100.0 * (ext_bytes - ir_bytes) / max(ir_bytes, 1):+.0f}%)"))
    noisy = {k: n for k, n in ext_noise.items() if n}
    if noisy:
        print("-femit-all-decls IR: libc++ instantiations that exist on one side only (names of unused template "
              "instantiations that follow a literal's type, not behaviour; counted, not failed): "
              + ", ".join(f"{k} ({n})" for k, n in noisy.items()))
    if line_sites:
        print("TUs with EDG assertion sites (`check_assertion`, `is_typeref_kind` under EXPENSIVE_CHECKING) in their own "
              "sources, which pass `__LINE__`: " + ", ".join(f"{k} ({n})" for k, n in line_sites.items()))
    print("generated headers read: " + (", ".join(sorted(os.path.relpath(g, root) for g in generated)) or "none"))
    print("  of them including a src/ or include/ header: " + (json.dumps(gen_inc) if gen_inc else
          "none (so nothing in build/ can pull a header past the per-side tree; any that did would be caught by "
          "the dependency check)"))
    if unread_msgs:
        print("files no compile reads (edits not covered by object/IR identity; --tokens covers them): "
              + "; ".join(unread_msgs))
    if opts.json:
        Path(opts.json).write_text(json.dumps({"tus": report, "unread": unread_msgs, "tokens": tok_res}, indent=1))
    if uncovered_err:
        raise ProofInvalid("files that no compile reads differ between the sides and are not allowed "
                           f"(--allow-uncovered; --tokens covers C++ sources): {uncovered_err}")
    if n_diff or n_fail or tok_bad:
        return 1
    return 0


def parse(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--before", required=True, help="a revision or a directory holding src/ and include/")
    ap.add_argument("--after", default=WORKTREE, help="like --before; default: the working tree")
    ap.add_argument("-j", "--jobs", type=int, default=8)
    ap.add_argument("--tu", action="append", default=[], metavar="PATH",
                    help="restrict to this TU (path under src/, e.g. src/vir/check.cpp; repeatable); the coverage "
                         "checks that need every TU are skipped")
    ap.add_argument("--allow-uncovered", action="append", default=list(DEFAULT_ALLOW_UNCOVERED), metavar="PATH",
                    help="a file no compile reads that may differ (default: include/cerastes.h)")
    ap.add_argument("--json", help="write per-TU results here")
    ap.add_argument("--no-emit-all-decls", action="store_true", help="IR without -femit-all-decls")
    ap.add_argument("--tokens", choices=("off", "none", "braces"), default="off",
                    help="compare the token streams of every file of both exports: none = no difference, braces = "
                         "only inserted balanced { } (default: off)")
    ap.add_argument("--no-compile", action="store_true", help="skip the compile passes (with --tokens)")
    ap.add_argument("--no-ir", action="store_true", help="objects only (a weaker proof)")
    # for the self-test's fixtures
    ap.add_argument("--root", default=str(REPO), help=argparse.SUPPRESS)
    ap.add_argument("--compile-commands", default=None, help=argparse.SUPPRESS)
    ap.add_argument("--scratch", default=None, help=argparse.SUPPRESS)
    ap.add_argument("--expected-tus", type=int, default=EXPECTED_TU_ENTRIES, help=argparse.SUPPRESS)
    ap.add_argument("--expected-files", type=int, default=EXPECTED_TU_FILES, help=argparse.SUPPRESS)
    return ap.parse_args(argv)


def main(argv=None):
    opts = parse(sys.argv[1:] if argv is None else argv)
    try:
        return run(opts)
    except ProofInvalid as e:
        print(f"object_identity: PROOF INVALID: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
