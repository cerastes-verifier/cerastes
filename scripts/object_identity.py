#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Prove that a refactor of cerastes's tool code (src/, include/) changed no behaviour (session S7).

    scripts/object_identity.py --before <rev|dir> [--after <rev|dir>] [-j N] [--tu <path> ...]

Each side is exported into build/s7/<side>-<label>/tree/ (a rev by `git archive`; a directory by copying its src/
and include/; the default `--after`, the working tree, by copying the files git lists in it).  Every cerastes
translation unit's command is taken from build/cmake/compile_commands.json, with the repository root rewritten
to the exported tree for paths under src/ and include/ only (EDG's headers and the headers generated into build/
stay where they are), and compiled for both sides with the same compiler, twice:

  * an object: the build's flags, then `-g0 -DNDEBUG -MD -MF <dep> -o <side>/<tu>.o`;
  * LLVM IR: `-S -emit-llvm -femit-all-decls -g0 -DNDEBUG` (the build's -O0), with the `; ModuleID` and
    `source_filename` lines normalised.  At -O0 machine code can hide a semantic change (`x - 1` becoming
    `x - 1U` on an `int` is the same instruction, `sub nsw` against `sub` in the IR); the IR keeps it.

A TU is identical when its IR is identical and its object is byte-identical.  When only the object bytes differ the
`objdump -d -r -s` text is compared and the TU is reported as "identical by objdump" (a warning, never silent).

This is a proof, so it refuses to pass vacuously (exit status 2, "proof invalid"):
  1. after rewriting, no argument names <root>/src or <root>/include (but the one `-DCERASTES_SOURCE_INCLUDE_DIR`
     string, a path the tool reads at run time, which stays so that both sides carry the same string);
  2. the dependency file of every compile is read: every header taken from src/ or include/ must come from that
     side's exported tree, never from the working tree or the other side (otherwise both sides would read one
     header and agree trivially);
  3. the tracked headers (.h, .inc, .def) that no TU reads are listed; an unread one that differs between the sides
     and is not on --allow-uncovered (default: include/cerastes.h, which only verified programs include and which
     the suite's .vpr/.map identity covers instead) is an error;
  4. every src/**/*.cpp of both sides appears in compile_commands.json and the number of cerastes TUs equals the
     expected constants (EXPECTED_TU_ENTRIES, EXPECTED_TU_FILES; a new TU makes the tool fail until they are
     updated with it).

Exit status: 0 every selected TU identical; 1 a difference or a compile failure; 2 the proof is not valid.
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

HEADER_SUFFIXES = (".h", ".hpp", ".inc", ".def")
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


def export_side(spec, root, dest):
    """Export src/ and include/ of `spec` into dest (a fresh directory).  Returns the label."""
    if dest.exists():
        shutil.rmtree(dest)
    dest.mkdir(parents=True)
    if spec == WORKTREE:
        # the tracked files as they are in the working tree, plus untracked and not ignored ones (a new header
        # must be seen: a missing one is a compile error, never a silent fallback)
        names = _git(root, "ls-files", "-z", "--cached", "--others", "--exclude-standard", "--", "src",
                     "include").stdout.split(b"\0")
        for n in names:
            if not n:
                continue
            n = n.decode()
            s = Path(root) / n
            if s.is_file():
                (dest / n).parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(s, dest / n)
        return "worktree"
    if os.path.isdir(spec):
        for top in ("src", "include"):
            s = Path(spec) / top
            if s.is_dir():
                shutil.copytree(s, dest / top)
        return re.sub(r"[^A-Za-z0-9._-]", "_", os.path.basename(os.path.abspath(spec)))
    try:
        sha = _git(root, "rev-parse", "--verify", spec + "^{commit}").stdout.decode().strip()
    except subprocess.CalledProcessError:
        raise ProofInvalid(f"{spec!r} is neither a directory nor a revision")
    arch = subprocess.run(["git", "-C", str(root), "archive", "--format=tar", sha, "src", "include"],
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
    dep = ["-g0", "-DNDEBUG", "-MD", "-MF", str(d / (tu.stem + "." + kind + ".d")), "-o"]
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


def check_deps(deps, tu_file_in_tree, root, tree, scratch):
    """Classify one TU's dependencies.  Returns (read, violations, generated): `read` the src/include files of this
    side's tree (relative to the tree), `violations` the headers taken from the working tree or the other side,
    `generated` the headers under <root>/build (outside the scratch directory)."""
    rroot = os.path.realpath(root)
    rtree = os.path.realpath(tree)
    rscratch = os.path.realpath(scratch)
    read, viol, gen = set(), [], set()
    for d in deps:
        p = os.path.realpath(os.path.join(rroot, d) if not os.path.isabs(d) else d)
        if p == rtree or p.startswith(rtree + os.sep):
            read.add(os.path.relpath(p, rtree))
        elif p.startswith(os.path.join(rroot, "src") + os.sep) or p.startswith(os.path.join(rroot, "include") + os.sep):
            viol.append(p)
        elif p.startswith(rscratch + os.sep):
            viol.append(p)
        elif p.startswith(os.path.join(rroot, "build") + os.sep):
            gen.add(p)
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
    a path under <root>/src or <root>/include (the exported file, mapped back by -fmacro-prefix-map) becomes 0.
    Calls from EDG's own headers keep theirs.  Returns (text, number of sites)."""
    own = set(m.group(1) for m in re.finditer(
        r'^(@\.str[.\d]*) = [^\n]*c"' + re.escape(root) + r'/(?:src|include)/[^"\\]*\\00"', ir, re.M))
    n = 0

    def sub(m):
        nonlocal n
        if m.group(2) in own:
            n += 1
            return m.group(1) + "0"
        return m.group(0)

    out = re.sub(r"(\b(?:call|invoke)\b[^\n]*?\(ptr (?:noundef )?(@\.str[.\d]*), i32 (?:noundef )?)(\d+)", sub, ir)
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
    an unrelated entity does not renumber the others.  Returns (kinds, bodies, misc)."""
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
        elif l.startswith(("attributes ", ";", "!", "source_filename", "")) and not l.startswith("!llvm"):
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



def arch_flags(args):
    """The target options of a command, for compiling an .ll file with the same target."""
    out, i = [], 0
    while i < len(args):
        a = args[i]
        if a in ("-arch", "-target", "--target", "-isysroot") and i + 1 < len(args):
            out += [a, args[i + 1]]
            i += 2
            continue
        if a.startswith(("--target=", "-m")) and not a.startswith("-mmacos"):
            out.append(a)
        i += 1
    return out


def compare_tu(tu, sides, kinds, scratch, root, cmds, results):
    entry = {"tu": tu.key, "obj": None, "ir": None, "ir_ext": None, "ext_noise": 0, "objdump": None,
             "verdict": None, "notes": [], "line_sites": 0}
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
        # the objects may differ in the line immediates; compare objects built from the normalised IR instead
        cxx = cmds[("before", tu.key)][0]
        flags = arch_flags(cmds[("before", tu.key)])
        objs = []
        for n, txt in (("before", nb), ("after", na)):
            ll = out[n] / (tu.stem + ".norm.ll")
            ll.write_text(txt)
            o = out[n] / (tu.stem + ".norm.o")
            r = subprocess.run([cxx, *flags, "-x", "ir", "-c", "-g0", str(ll), "-o", str(o)], capture_output=True,
                               text=True)
            if r.returncode:
                entry["notes"].append(f"{n}: compiling the normalised IR failed: {r.stderr[-500:]}")
                entry["verdict"] = "DIFFERS"
                return entry
            objs.append(o)
        if filecmp.cmp(objs[0], objs[1], shallow=False):
            entry["verdict"] = "identical-modulo-lines"
            entry["notes"].append(f"{entry['line_sites']} assertion sites embed a line of this TU's own sources; "
                                  f"the IR agrees with those numbers replaced and so do the objects built from it")
        else:
            entry["verdict"] = "DIFFERS"
            entry["notes"].append("the objects built from the line-normalised IR differ")
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


def run(opts):
    root = os.path.realpath(opts.root)
    cc_path = opts.compile_commands or os.path.join(root, "build/cmake/compile_commands.json")
    scratch = Path(opts.scratch or os.path.join(root, "build/s7"))
    scratch.mkdir(parents=True, exist_ok=True)
    if not os.path.isfile(cc_path):
        raise ProofInvalid(f"{cc_path} does not exist (run scripts/bootstrap.sh)")

    t0 = time.time()
    tus = load_tus(cc_path, root)
    files = sorted({t.rel for t in tus})
    # check 4, the constants
    if len(tus) != opts.expected_tus or len(files) != opts.expected_files:
        raise ProofInvalid(
            f"{cc_path} has {len(tus)} cerastes TU entries over {len(files)} files, expected {opts.expected_tus} "
            f"over {opts.expected_files} (EXPECTED_TU_ENTRIES/EXPECTED_TU_FILES in scripts/object_identity.py: a new "
            f"or removed TU must be added there, with its reason)")
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
    print(f"  before: {opts.before} -> {sides['before']['dir']}", flush=True)
    print(f"  after:  {opts.after} -> {sides['after']['dir']}", flush=True)

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
                read, viol, gen = check_deps(deps, in_tree, root, s["tree"], scratch)
                read_by_side[name] |= read
                generated |= gen
                violations += [f"{name} {tu.key} ({kind}): {v}" for v in viol]
    if violations:
        raise ProofInvalid("headers read from the wrong place (the working tree or the other side), so both sides "
                           "could agree vacuously:\n  " + "\n  ".join(violations[:20]))

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
    for tu in selected:
        entry = compare_tu(tu, sides, kinds, scratch, root, cmds, results)
        ir_bytes += entry.pop("ir_bytes", 0)
        ext_bytes += entry.pop("ir_ext_bytes", 0)
        ext_noise[tu.key] = entry["ext_noise"]
        if entry.get("line_sites"):
            line_sites[tu.key] = entry["line_sites"]
        v = entry["verdict"]
        n_ident += v == "identical"
        n_objdump += v == "identical-by-objdump"
        n_lines += v == "identical-modulo-lines"
        n_diff += v == "DIFFERS"
        n_fail += v == "COMPILE-FAILED"
        report.append(entry)

    # check 3: tracked headers that no TU reads
    unread_msgs, uncovered_err = [], []
    full = not opts.tu and not n_fail
    if full:
        allow = set(opts.allow_uncovered)
        tracked = {}
        for name, s in sides.items():
            tr = Path(s["tree"])
            tracked[name] = {str(p.relative_to(tr)) for p in tr.rglob("*") if p.is_file()
                             and p.suffix in HEADER_SUFFIXES}
        for h in sorted(set().union(*tracked.values())):
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
        print("headers no TU reads (edits not covered by object/IR identity): " + "; ".join(unread_msgs))
    if opts.json:
        Path(opts.json).write_text(json.dumps({"tus": report, "unread": unread_msgs}, indent=1))
    if uncovered_err:
        raise ProofInvalid("tracked headers that no TU reads differ between the sides and are not allowed "
                           f"(--allow-uncovered): {uncovered_err}")
    if n_diff or n_fail:
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
                    help="a header no TU reads that may differ (default: include/cerastes.h)")
    ap.add_argument("--json", help="write per-TU results here")
    ap.add_argument("--no-emit-all-decls", action="store_true", help="IR without -femit-all-decls")
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
