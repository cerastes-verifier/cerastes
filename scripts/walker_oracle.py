#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""The walker oracle: check a walker golden against EDG's --il_display.

The walker listing (``--cerastes_walk``, ``src/backend/walker.cpp``) is the IL
evidence for annotations, and its goldens are expectations (invariant 1), so
every claim in one has to be checked against an independent print of the same
IL.  ``--il_display`` is that print.  This tool reads both, rebuilds from
il_display what the listing should say, and reports per golden

* facts checked: claims of the listing that il_display confirms, by category;
* facts unverifiable by il_display, by category (claims il_display cannot
  show; see below), which are counted, never silently skipped;
* mismatches: claims il_display contradicts, and il_display facts the
  listing does not report.

It exits 1 on any mismatch, on a front-end run that failed, and on a golden
with no checkable fact (a vacuous pass is a failure); 2 on a usage error.

What is compared
----------------
The routines of the listing and of il_display (those declared in the primary
file) are matched as multisets keyed by the routine line and its signature,
so a routine on one side only is a mismatch, and so is the ``end walk:``
count.  For each routine:

* ``returns`` / ``param`` lines, the ``template NAME args <...>`` line, the
  ``constraint`` lines (names of concepts and their structure), and the
  ``attr ... on param N`` lines: equal as lists;
* ``attr`` blocks on the routine, its function type and its variables: name,
  namespace, tokens, owner, position and, for ``cerastes::`` attributes, the
  whole annotation tree (every node: operation kind, type_kind, type, value
  category, cast kind, names, positions).  Registered attributes must agree
  as lists; the others as an ordered subsequence, and an unrecognized entry
  that only this routine owns must be listed (the listing prints each entry
  once, so entries shared by several owners are checked globally);
* the ``op`` lines of a prototype body (and its ``tconst`` lines): equal as
  multisets to the operation nodes il_display reaches from the body, the
  constructor initializers, the local statics and the default arguments,
  following the expressions of unnamed folded constants as the walker does;
* ``stmt ... attr`` lines: a statement of that kind at that position that
  carries an attribute pointer, and conversely every attributed statement is
  listed.  il_display does not dump the attribute entries that only
  function-region entities point to, so the attribute's name and tokens are
  unverifiable; the argument trees of ``cerastes::`` statement attributes are
  matched against the expression nodes of the routine's memory region that
  no body reaches (those nodes are dumped), and each ``arg`` position against
  the positions in its tree;
* top-level ``attr ... on <entity>`` lines against the entity's attributes.

What is wildcarded or unverifiable, and counted as such
-------------------------------------------------------
* ``<tptk_...>`` / ``<tk_...>`` type spellings: the types of dependent
  expressions are not dumped, so they compare as one wildcard;
* attribute entries and constant entries that il_display does not dump (it
  prints the pointer only): the entry's content, or the constant's kind and
  position, is unverifiable; the owner and the use are still checked;
* ``warning:`` lines of the listing and lines about its own bookkeeping.

``iek_macro`` entries in il_display (RECORD_MACROS_IN_IL) are ignored: records
of kinds this tool does not know are parsed and then not followed.

Usage::

    python3 scripts/walker_oracle.py [-v] [-j N] [paths...]

The default paths are ``tests`` and ``doc/modules/ROOT/examples``: every
test with a ``// expect: golden`` header, in each of its modes.  A path may be
a golden, a source file or a directory.  ``-v`` lists every golden with its
counts and the notes.

Importable API (no subprocesses)::

    report = analyse(golden_text, il_display_text)   # a Report
    report.ok, report.checked, report.checked_by, report.unverifiable,
    report.mismatches

and ``run_il_display(source, mode)``, ``discover(paths)`` and
``check_job(job)`` for the subprocess side.  Standard library only,
Python >= 3.9.
"""

from __future__ import annotations

import argparse
import collections
import concurrent.futures
import os
import re
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple

ROOT = Path(__file__).resolve().parent.parent

# ---------------------------------------------------------------------------
# Part 1: reading il_display
# ---------------------------------------------------------------------------

HEADER_RE = re.compile(r"^(file-scope|func-scope) ([a-z][a-z0-9-]*)@([0-9a-f]+)\s*$")
REF_RE = re.compile(r"^(file-scope|func-scope) ([a-z][a-z0-9-]*)@([0-9a-f]+)(?:: (.*))?$")
KEY_RE = re.compile(r"^( *)([A-Za-z_][A-Za-z0-9_. ]*?):(?: +(.*))?$")
REGION_RE = re.compile(r"^Intermediate language for memory region (\d+) ")


@dataclass
class Rec:
    """One IL entry as il_display prints it."""

    kind: str
    addr: str
    scope: str  # "file-scope" or "func-scope"
    region: int  # memory region number (1 is the file scope)
    fields: List[Tuple[str, str]] = field(default_factory=list)
    _d: Optional[Dict[str, str]] = None

    @property
    def id(self) -> str:
        return f"{self.kind}@{self.addr}"

    def get(self, key: str, default: Optional[str] = None) -> Optional[str]:
        if self._d is None:
            d: Dict[str, str] = {}
            for k, v in self.fields:
                d.setdefault(k, v)
            self._d = d
        return self._d.get(key, default)

    def all(self, key: str) -> List[str]:
        return [v for k, v in self.fields if k == key]

    def flag(self, key: str) -> bool:
        return self.get(key) == "TRUE"


def ref_parts(value: Optional[str]) -> Optional[Tuple[str, str, Optional[str]]]:
    """(``kind@addr``, scope, label) of a pointer value, or None."""
    if not value:
        return None
    m = REF_RE.match(value)
    if not m:
        return None
    return f"{m.group(2)}@{m.group(3)}", m.group(1), m.group(4)


class IL:
    """A parsed il_display."""

    def __init__(self, text: str) -> None:
        self.recs: Dict[str, Rec] = {}
        self.by_kind: Dict[str, List[Rec]] = collections.defaultdict(list)
        self.order: List[Rec] = []
        self.region_names: Dict[int, str] = {}
        self._parse(text)
        self.source_files = self.by_kind.get("source-file", [])
        self.lookup = sorted(
            ((int(r.get("first")), int(r.get("last")), int(r.get("line_number")), self.ref(r.get("source_file")))
             for r in self.by_kind.get("seq-number-lookup-entry", [])),
            key=lambda t: t[0],
        )
        self.primary: Optional[Rec] = None
        hdr = [r for r in self.by_kind.get("il-header", [])]
        self.primary_name = ""
        m = re.search(r'^primary_source_file:\s+\S+ source-file@\S+: "(.*)"\s*$', text, re.M)
        if m:
            self.primary_name = m.group(1)
        self._file_cache: Dict[str, Tuple[str, bool, bool]] = {}
        self._labels: Optional[Dict[str, str]] = None

    # -- parsing -----------------------------------------------------------

    def _parse(self, text: str) -> None:
        cur: Optional[Rec] = None
        region = 1
        stack: List[Tuple[int, str]] = []  # (indent, key) of keys that opened a section
        last_key: Optional[int] = None
        for raw in text.split("\n"):
            m = REGION_RE.match(raw)
            if m:
                region = int(m.group(1))
                cur = None
                continue
            m = HEADER_RE.match(raw)
            if m:
                cur = Rec(m.group(2), m.group(3), m.group(1), region)
                self.recs[cur.id] = cur
                self.by_kind[cur.kind].append(cur)
                self.order.append(cur)
                stack = []
                last_key = None
                continue
            if cur is None or raw == "":
                continue
            indent = len(raw) - len(raw.lstrip(" "))
            if indent >= 20 and last_key is not None:
                # A value that did not fit next to a long key.
                k, v = cur.fields[last_key]
                cur.fields[last_key] = (k, (v + " " + raw.strip()).strip())
                if stack and stack[-1][1] == k.rsplit(".", 1)[-1]:
                    stack.pop()  # the key was a flag whose value wrapped, not a section
                continue
            m = KEY_RE.match(raw)
            if not m:
                continue
            ind, key, val = len(m.group(1)), m.group(2), m.group(3) or ""
            while stack and stack[-1][0] >= ind:
                stack.pop()
            # Sub-keys of source_corresp are the entity's own; others are
            # qualified by their section (template_arg_list.type, ...).
            prefix = ".".join(k for _i, k in stack if k != "source_corresp")
            full = prefix + "." + key if prefix else key
            cur.fields.append((full, val))
            last_key = len(cur.fields) - 1
            if val == "":
                stack.append((ind, key))

    # -- lookups -----------------------------------------------------------

    def ref(self, value: Optional[str]) -> Optional[Rec]:
        p = ref_parts(value)
        if p is None:
            return None
        return self.recs.get(p[0])

    def label(self, value: Optional[str]) -> str:
        p = ref_parts(value)
        return (p[2] or "") if p else ""

    def rec_label(self, rec: Rec) -> str:
        """The label il_display prints after pointers to ``rec`` (its name, as
        form_name/form_type show it); "" when no pointer to it has a label."""
        if self._labels is None:
            labels: Dict[str, str] = {}
            for r in self.order:
                for _k, v in r.fields:
                    if "@" in v:
                        p = ref_parts(v)
                        if p is not None and p[2] and p[0] not in labels:
                            labels[p[0]] = p[2]
            self._labels = labels
        return self._labels.get(rec.id, "")

    def name_of(self, rec: Rec, key: str = "name") -> str:
        """The unquoted name of an entity ("" when it has none)."""
        v = rec.get(key)
        if not v:
            return ""
        lab = self.label(v)
        if len(lab) >= 2 and lab[0] == '"' and lab[-1] == '"':
            return lab[1:-1]
        return lab

    def refs(self, rec: Rec, key: str) -> List[Rec]:
        out = []
        for v in rec.all(key):
            r = self.ref(v)
            if r is not None:
                out.append(r)
        return out

    def chain(self, head: Optional[Rec], key: str = "next") -> Iterable[Rec]:
        seen = set()
        r = head
        while r is not None and r.id not in seen:
            seen.add(r.id)
            yield r
            r = self.ref(r.get(key))

    # -- positions ---------------------------------------------------------

    def _file_for_seq(self, seq: int) -> Optional[Tuple[Rec, int]]:
        for first, last, line, sf in self.lookup:
            if first <= seq <= last:
                return (sf, line + (seq - first)) if sf is not None else None
        return None

    def seq_line(self, seq: int) -> Optional[Tuple[str, int, Rec]]:
        r = self._file_for_seq(seq)
        if r is None:
            return None
        sf, line = r
        name = self.name_of(sf, "file_name")
        return name, line, sf

    def in_include(self, seq: int) -> bool:
        r = self._file_for_seq(seq)
        if r is None:
            return False
        sf = r[0]
        if sf.flag("is_include_file"):
            return True
        if sf.get("full_name") in (None, "NULL", ""):
            return self._fname(sf) != self.primary_name
        return False

    def in_system(self, seq: int) -> bool:
        r = self._file_for_seq(seq)
        return bool(r and r[0].flag("from_system_include_dir"))

    def _fname(self, sf: Rec) -> str:
        return self.name_of(sf, "file_name")

    def pos(self, rec: Rec, prefix: str, annot: bool = False) -> str:
        """``L<line>:<col>`` of the position fields ``prefix.seq/column``.

        ``annot`` reads the original (macro-definition-independent) position
        when there is one that is not in a system header: the rule of
        diag_position in src/backend/diagnostics.cpp, which the annotation
        trees use.  Otherwise the plain seq/column (``resolve``).
        """
        seq = rec.get(prefix + ".seq")
        col = rec.get(prefix + ".column")
        if seq is None:
            return "-"
        seq_i, col_i = int(seq), int(col or 0)
        if annot:
            oseq = rec.get(prefix + ".orig_seq")
            ocol = rec.get(prefix + ".orig_column")
            if oseq and int(oseq) != 0 and not self.in_system(int(oseq)):
                seq_i, col_i = int(oseq), int(ocol or 0)
        return self.fmt_pos(seq_i, col_i, annot)

    def fmt_pos(self, seq: int, col: int, annot: bool) -> str:
        if seq == 0:
            return "-"
        r = self.seq_line(seq)
        if r is None:
            return "-"
        name, line, _sf = r
        s = f"L{line}:{col}"
        if annot and self.in_include(seq):
            return f"{name}:{s}"
        return s

    def is_primary_seq(self, seq: int) -> bool:
        return not self.in_include(seq)


# ---------------------------------------------------------------------------
# Part 2: reading the walker listing (the golden)
# ---------------------------------------------------------------------------


@dataclass
class Block:
    """A listing item: a head line and the lines indented under it.

    ``kids`` hold the child lines with their indentation relative to the
    first level of children (0 spaces for a direct child)."""

    head: str
    kids: List[str] = field(default_factory=list)
    aid: str = ""  # il_display id of the attribute entry (il side only)

    def text(self) -> str:
        return "\n".join([self.head] + self.kids)


@dataclass
class View:
    """One routine of the listing, split into categories.

    The golden and il_display are both turned into Views; the comparison is
    View against View."""

    header: str
    sig: List[str] = field(default_factory=list)  # returns / param lines
    pattr: List[str] = field(default_factory=list)  # attr ... on param N
    tmpl: List[str] = field(default_factory=list)  # template ... args <...>
    cons: List[str] = field(default_factory=list)  # constraint lines
    attrs: List[Block] = field(default_factory=list)  # attr on routine / type / variable
    stmts: List[Block] = field(default_factory=list)  # stmt ... attr ...
    ops: List[str] = field(default_factory=list)  # op lines of a prototype body
    tconsts: List[str] = field(default_factory=list)
    misc: List[str] = field(default_factory=list)  # inherits, contract from, warning

    @property
    def key(self) -> str:
        return "\n".join([self.header] + self.sig + self.tmpl)


@dataclass
class Golden:
    views: List[View]
    top: List[Block]  # top-level `attr ... on <entity>` blocks
    warnings: List[str]
    end: Optional[Tuple[int, int]]


END_RE = re.compile(r"^end walk: (\d+) routines reported, (\d+) routines with bodies in non-system files outside the primary file$")


def split_items(lines: List[Tuple[int, str]], base: int) -> List[Block]:
    """Group (indent, text) lines into items at indent ``base`` with their kids."""
    out: List[Block] = []
    for ind, txt in lines:
        if ind <= base:
            out.append(Block(txt))
        elif out:
            out[-1].kids.append(" " * (ind - base - 2) + txt)
    return out


def parse_golden(text: str) -> Golden:
    lines = text.split("\n")
    raw: List[Tuple[int, str]] = []
    for ln in lines:
        if ln == "--- stderr ---":
            break
        if ln.startswith("\\ No newline"):
            continue
        if ln.strip() == "":
            continue
        raw.append((len(ln) - len(ln.lstrip(" ")), ln.lstrip(" ")))
    routines: List[Tuple[str, List[Tuple[int, str]]]] = []
    top_lines: List[Tuple[int, str]] = []
    end = None
    warnings: List[str] = []
    cur: Optional[List[Tuple[int, str]]] = None
    for ind, txt in raw:
        if ind == 0:
            if txt.startswith("routine "):
                cur = []
                routines.append((txt, cur))
                continue
            cur = None
            m = END_RE.match(txt)
            if m:
                end = (int(m.group(1)), int(m.group(2)))
                continue
            if txt.startswith("warning:"):
                warnings.append(txt)
                continue
            top_lines.append((ind, txt))
        elif cur is not None:
            cur.append((ind, txt))
        else:
            top_lines.append((ind, txt))
    views = []
    for header, body in routines:
        v = View(header)
        for it in split_items(body, 2):
            h = it.head
            if h.startswith(("returns ", "param ")):
                v.sig.append(h)
            elif h.startswith("template "):
                v.tmpl.append(h)
            elif h.startswith("constraint "):
                v.cons.append(h)
            elif h.startswith("attr "):
                if re.search(r"\]\] on param \d+ @ ", h):
                    v.pattr.append(h)
                else:
                    v.attrs.append(it)
            elif h.startswith("stmt "):
                v.stmts.append(it)
            elif h.startswith("op "):
                v.ops.append(h)
            elif h.startswith("tconst "):
                v.tconsts.append(h)
            else:
                v.misc.append(h)
        views.append(v)
    return Golden(views, split_items(top_lines, 0), warnings, end)


# ---------------------------------------------------------------------------
# Part 3: the same views from il_display
# ---------------------------------------------------------------------------

TREE_DEPTH_CAP = 256
WILD_TYPE = "<unnamed-type>"
_WILD_RE = re.compile(r"<(?:tptk|tk)_[a-z_]+>")


def wild(text: str) -> str:
    """Type kinds of unnamed (dependent) types are not in il_display; both
    sides are compared with them wildcarded."""
    return _WILD_RE.sub(WILD_TYPE, text)


PSEUDO_ATTR_KINDS = ("empty attribute", '"using" prefix')


def unescape(s: str) -> str:
    """Undo the C escapes il_display puts in quoted strings."""
    out = []
    i = 0
    while i < len(s):
        c = s[i]
        if c == "\\" and i + 1 < len(s):
            n = s[i + 1]
            if n in "01234567":
                j = i + 1
                while j < len(s) and j < i + 4 and s[j] in "01234567":
                    j += 1
                out.append(chr(int(s[i + 1 : j], 8)))
                i = j
                continue
            out.append({"n": "\n", "t": "\t", '"': '"', "\\": "\\", "'": "'"}.get(n, n))
            i += 2
            continue
        out.append(c)
        i += 1
    return "".join(out)


class Extractor:
    """Builds Views (and the pools the statement checks need) from an IL."""

    def __init__(self, il: IL) -> None:
        self.il = il
        self.fn_scope: Dict[str, Rec] = {}
        for sc in il.by_kind.get("scope", []):
            if sc.get("kind") == "sck_function":
                r = il.ref(sc.get("routine.ptr"))
                if r is not None:
                    self.fn_scope[r.id] = sc
        self.closure_ids = set()
        for lam in il.by_kind.get("lambda", []):
            c = il.ref(lam.get("closure_class"))
            if c is not None:
                self.closure_ids.add(c.id)
        # A closure class that no `lambda` record names: the copy the front end makes when it rescans a lambda
        # of a function template's contract for an instance (EDG bug 0020 fix, fork series L) has its own class
        # and its own operator(), but no lambda record.  It is a closure class when EDG's own flag on its class
        # supplement says so AND its scope holds the lambda body (an `operator()` with is_lambda_body): both,
        # never a printed name.
        body_scopes = set()
        for r in il.by_kind.get("routine", []):
            if r.flag("is_lambda_body"):
                ps = il.ref(r.get("parent_scope"))
                if ps is not None:
                    body_scopes.add(ps.id)
        for t in il.by_kind.get("type", []):
            if t.get("kind") != "tk_class":
                continue
            sup = il.ref(t.get("extra_info"))
            if sup is None or not sup.flag("is_lambda_closure_class"):
                continue
            sc = il.ref(sup.get("assoc_scope"))
            if sc is not None and sc.id in body_scopes:
                self.closure_ids.add(t.id)
        # problems found while building the views, reported by analyse(): the IL
        # itself is inconsistent with the listing's classification of routines
        self.view_mismatches: List[str] = []
        self.proto_owner: Dict[str, Rec] = {}
        for t in il.by_kind.get("template", []):
            r = il.ref(t.get("routine"))
            if r is not None and r.id not in self.proto_owner:
                canon = il.ref(t.get("canonical_template"))
                if canon is None or canon.id == t.id:
                    self.proto_owner[r.id] = il.ref(t.get("definition_template")) or t

    # -- types and names -----------------------------------------------------

    def kind(self, t: Optional[Rec]) -> str:
        return (t.get("kind") or "") if t is not None else ""

    def skip_typerefs(self, t: Optional[Rec]) -> Optional[Rec]:
        guard = 0
        while t is not None and self.kind(t) == "tk_typeref" and guard < 64:
            n = self.il.ref(t.get("typeref_type"))
            if n is None:
                break
            t = n
            guard += 1
        return t

    def unnamed(self, addr: str) -> str:
        t = self.il.recs.get("type@" + addr)
        if t is None:
            # il_display does not dump this type: an expression type the
            # front end could not name (dependent).  Compared as a wildcard.
            return WILD_TYPE
        if t.id in self.closure_ids:
            return "<lambda@" + self.il.pos(t, "decl_position", annot=True) + ">"
        u = self.skip_typerefs(t)
        if u is not None and self.kind(u) == "tk_template_param":
            ks = [v for kk, v in u.fields if kk == "kind"]
            return "<" + (ks[1] if len(ks) > 1 else "tk_template_param") + ">"
        return "<unnamed>"

    def nl(self, lab: str) -> str:
        """Normalize a printed type label to the listing's spelling."""
        lab = re.sub(r"#\(\d+,\d+\)", "", lab)
        lab = re.sub(r"<unnamed@([0-9a-f]+)>", lambda m: self.unnamed(m.group(1)), lab)
        return lab

    def tl(self, value: Optional[str]) -> str:
        p = ref_parts(value)
        if p is None:
            return "<null>"
        return wild(self.nl(p[2] or ""))

    def scope_prefix(self, sc: Optional[Rec]) -> str:
        il = self.il
        guard = 0
        parts: List[str] = []
        while sc is not None and guard < 64:
            guard += 1
            k = sc.get("kind")
            if k == "sck_namespace":
                ns = il.ref(sc.get("assoc_namespace"))
                nm = il.label(sc.get("assoc_namespace")) if ns is not None else ""
                parts.append(nm + "::")
                sc = il.ref(sc.get("parent"))
            elif k == "sck_class_struct_union":
                lab = self.tl(sc.get("assoc_type"))
                # the qualifier of an unnamed class is just `<unnamed>`.
                lab = re.sub(r"^(?:struct|class|union) (?=<unnamed>)", "", lab)
                parts.append(lab + "::")
                break
            else:
                break
        return "".join(reversed(parts))

    def qname(self, rec: Rec, name: Optional[str] = None) -> str:
        if name is None:
            name = self.il.name_of(rec) or "<unnamed>"
        return self.scope_prefix(self.il.ref(rec.get("parent_scope"))) + name

    # -- routines --------------------------------------------------------------

    def routine_kind(self, r: Rec) -> str:
        # Derived from the IL's own flags, not from the walker's order of tests: an explicit specialization
        # written inside its class template (CWG 727) has is_specialized and is_in_class_specialization, and
        # is also a prototype instantiation because its class is a template; it is a specialization.  Every
        # other routine is a prototype if the IL says so (a prototype that is specialized without being an
        # in-class specialization is not explained: _extractor_views reports it).
        if r.flag("is_specialized") and r.flag("is_in_class_specialization"):
            return "specialization"
        if r.flag("is_prototype_instantiation"):
            return "prototype"
        if r.flag("is_template_function") or r.get("assoc_template"):
            return "specialization" if r.flag("is_specialized") else "instance"
        return "plain"

    def routine_type(self, r: Rec) -> Optional[Rec]:
        t = self.skip_typerefs(self.il.ref(r.get("type")))
        return t if self.kind(t) == "tk_routine" else None

    def params(self, r: Rec) -> List[Rec]:
        t = self.routine_type(r)
        return list(self.il.chain(self.il.ref(t.get("param_type_list")))) if t is not None else []

    def routine_name(self, r: Rec) -> str:
        return self.qname(r)

    def template_args(self, tmpl: Optional[Rec], owner: Rec, prefix: str = "template_arg_list") -> str:
        il = self.il
        entries: List[Tuple[str, str, Dict[str, bool]]] = []  # (key, value, flags)
        for k, v in owner.fields:
            if not k.startswith(prefix + "."):
                continue
            kk = k[len(prefix) + 1 :]
            if kk in ("is_pack_element", "is_pack", "explicitly_specified", "is_pack_expansion"):
                if entries:
                    entries[-1][2][kk] = v == "TRUE"
                continue
            entries.append((kk, v, {}))
        if not entries:
            return "none"
        td = il.ref(tmpl.get("template_decl")) if tmpl is not None else None
        tp = il.ref(td.get("param_list")) if td is not None else None
        parts: List[str] = []
        pack: List[str] = []
        pack_name = ""
        in_pack = False

        def pname(tp_: Optional[Rec], index: int) -> str:
            if tp_ is not None:
                nm = il.name_of(tp_)
                if nm:
                    return nm
            return "#" + str(index)

        def flush() -> None:
            nonlocal in_pack, pack
            if in_pack:
                parts.append(pack_name + "=pack{" + ", ".join(pack) + "}")
                in_pack = False
                pack = []

        def render(kk: str, v: str) -> str:
            if kk == "type":
                return self.tl(v)
            if kk == "constant":
                c = il.ref(v)
                return self.constant_value(c, v) if c is not None else il.label(v)
            if kk == "template":
                t = il.ref(v)
                return self.qname(t) if t is not None else il.label(v)
            return "<" + kk + ">"

        index = 0
        for kk, v, fl in entries:
            if kk == "pack exp placeholder":
                flush()
                in_pack = True
                pack_name = pname(tp, index)
                tp = il.ref(tp.get("next")) if tp is not None else None
                index += 1
                continue
            if in_pack and fl.get("is_pack_element"):
                pack.append(render(kk, v))
                continue
            flush()
            parts.append(pname(tp, index) + "=" + render(kk, v))
            tp = il.ref(tp.get("next")) if tp is not None else None
            index += 1
        flush()
        return ", ".join(parts)

    def constant_value(self, c: Optional[Rec], ref_value: Optional[str] = None) -> str:
        """The `const VALUE` text of an integer/float/address constant."""
        if c is None:
            return "<null>"
        k = c.get("kind")
        if k == "ck_integer":
            t = self.skip_typerefs(self.il.ref(c.get("type")))
            v = c.get("integer_value") or ""
            if t is not None and self.kind(t) == "tk_nullptr":
                return "nullptr"
            if t is not None and self.kind(t) == "tk_integer" and t.flag("bool_type"):
                return "false" if v == "0" else "true"
            # il_display prints the literal with its suffix (2UL); the listing the value.
            return re.sub(r"[uUlL]+$", "", v)
        if k in ("ck_float", "ck_address"):
            return self.il.label(ref_value) if ref_value else ""
        return "<" + (k or "?") + ">"

    # -- annotation trees (the `arg ...` lines under a registered attribute) ----

    def vflags(self, e: Rec) -> str:
        return " lvalue" if e.flag("is_lvalue") else (" xvalue" if e.flag("is_xvalue") else "")

    def cast_kind(self, e: Rec) -> str:
        if e.get("operation.kind") not in ("eok_cast", "eok_lvalue_cast", "eok_ref_cast"):
            return ""
        k = "c-style"
        if e.flag("is_reinterpret_cast"):
            k = "reinterpret"
        elif e.flag("is_const_cast"):
            k = "const"
        elif e.flag("is_static_cast"):
            k = "static"
        elif e.flag("is_functional_notation_cast"):
            k = "functional"
        elif e.flag("compiler_generated"):
            k = "implicit"
        return " cast=" + k

    def nth_param(self, owner: Optional[Rec], n: int) -> Optional[Rec]:
        if owner is None:
            return None
        for i, p in enumerate(self.params(owner), start=1):
            if i == n:
                return p
        return None

    def result_type(self, owner: Optional[Rec]) -> str:
        t = self.routine_type(owner) if owner is not None else None
        return self.tl(t.get("return_type")) if t is not None else "<unknown>"

    def tree(self, e: Optional[Rec], indent: int, out: List[str], ctx: "TreeCtx", depth: int = 0) -> None:
        il = self.il

        def line(ind: int, text: str) -> None:
            out.append(" " * ind + text)

        if e is None:
            line(indent, "node <null>")
            return
        if depth >= TREE_DEPTH_CAP:
            line(indent, "(depth limit)")
            return
        if e.id in ctx.visited:
            line(indent, "(repeated node)")
            return
        ctx.visited.add(e.id)
        pos = il.pos(e, "position", annot=True)
        ty = self.tl(e.get("type"))
        k = e.get("kind")
        if k == "enk_operation":
            line(indent, f"op {e.get('operation.kind')} type_kind={e.get('operation.type_kind')} type={ty}"
                         f"{self.vflags(e)}{self.cast_kind(e)} @ {pos}")
            for c in il.chain(il.ref(e.get("operands"))):
                self.tree(c, indent + 2, out, ctx, depth + 1)
        elif k == "enk_constant":
            c = il.ref(e.get("constant"))
            if c is None and ref_parts(e.get("constant")) is not None:
                # The constant entry is not dumped; only the use is known.
                line(indent, f"{CONST_MARK} type={ty} @ {pos}")
            elif c is not None and c.get("kind") == "ck_template_param":
                self.tconst_tree(e, c, indent, out, ctx, depth, pos)
            else:
                line(indent, f"const {self.constant_value(c, e.get('constant'))} type={ty} @ {pos}")
        elif k == "enk_param_ref":
            n = int(e.get("param_ref.param_num") or 0)
            up = int(e.get("param_ref.levels_up") or 0)
            role = "unmapped"
            if n == 0:
                role = "this" if up == 0 else "unmapped"
            elif ctx.annot == "ensures":
                if up == 1:
                    role = "param"
                elif up == 0 and n == 1 and ctx.binder_name is not None:
                    role = "binder"
            elif up == 0:
                role = "param"
            if role == "param":
                p = self.nth_param(ctx.owner, n)
                pname = il.name_of(p) if p is not None else ""
                pty = self.tl(p.get("type")) if p is not None else ty
                line(indent, f"param {n} {pname or '_'}: {pty}{self.vflags(e)} @ {pos}")
            elif role == "binder":
                line(indent, f"binder {ctx.binder_name or '_'}: {self.result_type(ctx.owner)}{self.vflags(e)} @ {pos}")
            elif role == "this":
                line(indent, f"this: {ty} @ {pos}")
            else:
                line(indent, f"node enk_param_ref param_num={n} levels_up={up} type={ty} @ {pos} (unmapped)")
        elif k == "enk_variable":
            v = il.ref(e.get("variable"))
            if v is None:
                line(indent, f"node enk_variable type={ty} @ {pos} (no variable)")
            elif v.flag("is_this_parameter"):
                line(indent, f"this: {ty} @ {pos}")
            else:
                line(indent, f"var {self.qname(v)}: {ty} decl={il.pos(v, 'decl_position', annot=True)}{self.vflags(e)} @ {pos}")
        elif k == "enk_field":
            f = il.ref(e.get("field"))
            line(indent, f"field {self.qname(f) if f is not None else '<null>'}: {ty}{self.vflags(e)} @ {pos}")
        elif k == "enk_routine":
            r = il.ref(e.get("routine"))
            if r is None:
                line(indent, f"routine <none>: {ty} @ {pos}")
            else:
                s = "routine " + self.routine_name(r)
                if any(kk == "template_arg_list" for kk, _v in r.fields):
                    s += " args <" + self.template_args(il.ref(r.get("assoc_template")), r) + ">"
                line(indent, f"{s}: {ty} @ {pos}")
        elif k == "enk_lambda":
            lam = il.ref(e.get("source.lambda"))
            line(indent, f"lambda {self.tl(lam.get('closure_class')) if lam is not None else '<null>'} @ {pos}")
            if lam is not None:
                for cap in il.chain(il.ref(lam.get("capture_list"))):
                    cpos = il.pos(cap, "position", annot=True)
                    how = "by-ref" if cap.flag("capture_by_reference") else "by-value"
                    if cap.flag("is_init_capture"):
                        f = il.ref(cap.get("closure_field"))
                        line(indent + 2, f"capture init {il.name_of(f) if f is not None else '_'} {how} @ {cpos}")
                        line(indent + 4, "node <init-capture child not modelled>")
                    elif cap.flag("is_indirect_init_capture"):
                        f = il.ref(cap.get("captured.init_capture_field"))
                        line(indent + 2, f"capture indirect-init {il.name_of(f) if f is not None else '_'} {how} @ {cpos}")
                    else:
                        v = il.ref(cap.get("captured.variable"))
                        if v is not None and v.flag("is_this_parameter"):
                            line(indent + 2, f"capture {'this' if cap.flag('capture_by_reference') else '*this'}"
                                             f"{' implicit' if cap.flag('is_implicit') else ''} @ {cpos}")
                        elif v is not None:
                            line(indent + 2, f"capture {how} {self.qname(v)}: {self.tl(v.get('type'))}"
                                             f"{' implicit' if cap.flag('is_implicit') else ''} @ {cpos}")
                        else:
                            line(indent + 2, f"capture {how} <unknown> @ {cpos}")
                self.lambda_body(lam, indent + 2, out, ctx, depth)
        elif k == "enk_sizeof_pack":
            if e.flag("is_template_template"):
                t = il.ref(e.get("templ"))
                line(indent, f"sizeof... {self.qname(t) if t is not None else '<null>'} type={ty} @ {pos}")
            elif e.flag("is_type"):
                # the node's own `type` comes first, the pack's type second
                line(indent, f"sizeof... {self.tl(e.all('type')[1])} type={ty} @ {pos}")
            else:
                line(indent, f"sizeof... type={ty} @ {pos}")
                self.tree(il.ref(e.get("expr")), indent + 2, out, ctx, depth + 1)
        elif k == "enk_type_operand":
            tt = e.all("type")
            line(indent, "type " + (self.tl(tt[1]) if len(tt) > 1 else "<default>"))
        else:
            line(indent, f"node {k} type={ty} @ {pos} (children not printed)")

    def lambda_body(self, lam: Rec, indent: int, out: List[str], ctx: "TreeCtx", depth: int) -> None:
        il = self.il
        r = il.ref(lam.get("lambda_routine"))
        sc = self.fn_scope.get(r.id) if r is not None else None
        blk = il.ref(sc.get("assoc_block")) if sc is not None else None
        if blk is None:
            out.append(" " * indent + "body <unavailable>")
            return
        stmts = list(il.chain(il.ref(blk.get("statements")))) if blk.get("kind") == "stmk_block" else []
        if len(stmts) == 1 and stmts[0].get("kind") == "stmk_return" and il.ref(stmts[0].get("expr")) is not None:
            out.append(" " * indent + "body return")
            self.tree(il.ref(stmts[0].get("expr")), indent + 2, out, ctx, depth + 1)
        else:
            out.append(" " * indent + f"body <{len(stmts)} statements>")

    def tconst_tree(self, e: Rec, c: Rec, indent: int, out: List[str], ctx: "TreeCtx", depth: int, pos: str) -> None:
        il = self.il
        kinds = [v for kk, v in c.fields if kk == "kind"]
        tp = kinds[1] if len(kinds) > 1 else "?"
        ty = " type=" + self.tl(e.get("type")) + " @ " + pos
        nm = il.name_of(c)
        if tp == "tpck_param":
            out.append(" " * indent + f"tconst {tp} {nm or '_'} #{c.get('coordinates.position')}@{c.get('coordinates.depth')}{ty}")
            return
        if tp == "tpck_unknown_function":
            out.append(" " * indent + f"tconst {tp} {self.qname(c, nm) if nm else '_'}{ty}")
            return
        if tp in ("tpck_sizeof", "tpck_datasizeof", "tpck_alignof", "tpck_uuidof", "tpck_typeid", "tpck_noexcept"):
            types = c.all("type")
            exprs = c.all("expr")
            t = self.tl(types[1]) if len(types) > 1 and ref_parts(types[1]) else None
            out.append(" " * indent + f"tconst {tp}" + (f" operand={t}" if t else "") + ty)
            own = il.ref(exprs[-1]) if exprs else None
            if own is not None:
                self.tree(own, indent + 2, out, ctx, depth + 1)
            return
        out.append(" " * indent + f"tconst {tp}" + (f" {nm}" if nm else "") + ty)
        exprs = c.all("expr")
        own = il.ref(exprs[-1]) if tp == "tpck_expression" and exprs else None
        if own is None and exprs:
            own = il.ref(exprs[0])
        if own is not None:
            self.tree(own, indent + 2, out, ctx, depth + 1)

    # -- attributes --------------------------------------------------------------

    def attr_list(self, head_value: Optional[str]) -> List[Rec]:
        """The attribute entries behind a pointer.  il_display does not dump
        the entries that only function-region entities (statements, local
        variables) point to; those come back as one MISSING placeholder."""
        out: List[Rec] = []
        v = head_value
        seen = set()
        while ref_parts(v) is not None:
            r = self.il.ref(v)
            if r is None:
                out.append(MISSING_ATTR)
                break
            if r.id in seen:
                break
            seen.add(r.id)
            out.append(r)
            v = r.get("next")
        return out

    def is_pseudo(self, a: Rec) -> bool:
        if a is MISSING_ATTR:
            return False
        return a.get("kind") in PSEUDO_ATTR_KINDS or a.get("syntactic_location") == "implicit"

    def is_registered(self, a: Rec) -> bool:
        return a.get("kind") == "registered" and self.il.name_of(a, "namespace_name") == "cerastes"

    def attr_args(self, a: Rec) -> List[Rec]:
        return list(self.il.chain(self.il.ref(a.get("arguments"))))

    def render_attribute(self, a: Rec) -> str:
        il = self.il
        s = "[["
        ns = il.name_of(a, "namespace_name")
        if a.get("namespace_name") and a.get("namespace_name") != "NULL" and ns:
            s += ns + "::"
        s += il.name_of(a) or "<unnamed>"
        if a.get("arguments") and il.ref(a.get("arguments")) is not None:
            pieces = []
            for g in self.attr_args(a):
                gk = g.get("kind")
                if gk == "empty":
                    continue
                if gk in ("raw token", "token"):
                    lab = il.label(g.get("token"))
                    if len(lab) >= 2 and lab[0] == '"' and lab[-1] == '"':
                        lab = unescape(lab[1:-1])
                    pieces.append(lab)
                elif gk == "constant":
                    pieces.append("<aak_constant>")
                elif gk == "type":
                    pieces.append("<aak_type>")
                elif gk == "expression":
                    pieces.append("<aak_expression>")
                else:
                    pieces.append("<aak?>")
            s += "(" + " ".join(pieces) + ")"
        return s + "]]"

    def attr_block(self, a: Rec, on: str, owner: Optional[Rec]) -> Block:
        b = self._attr_block(a, on, owner)
        b.aid = "" if a is MISSING_ATTR else a.id
        return b

    def _attr_block(self, a: Rec, on: str, owner: Optional[Rec]) -> Block:
        il = self.il
        if a is MISSING_ATTR:
            return Block(f"attr {MISSING_MARK} on {on}")
        if self.is_registered(a):
            head = f"attr [[cerastes::{il.name_of(a) or '?'}]] on {on} @ {il.pos(a, 'position', annot=True)}"
            ctx = TreeCtx(owner, il.name_of(a))
            kids: List[str] = []
            k = 0
            args = [g for g in self.attr_args(a) if g.get("kind") != "empty"]
            for g in args:
                if g.get("kind") == "token":
                    ctx.binder_name = unescape(il.label(g.get("token")).strip('"'))
            for g in args:
                k += 1
                gpos = il.pos(g, "position", annot=True)
                gk = g.get("kind")
                if gk == "token":
                    kids.append(f"arg {k} binder {ctx.binder_name or '_'}: {self.result_type(owner)} @ {gpos}")
                elif gk == "expression":
                    kids.append(f"arg {k} @ {gpos}")
                    ex = il.ref(g.get("expr"))
                    out: List[str] = []
                    self.tree(ex, 0, out, ctx)
                    kids.extend("  " + o for o in out)
                elif gk == "constant":
                    c = il.ref(g.get("constant"))
                    cty = self.tl(c.get("type")) if c is not None else "<null>"
                    kids.append(f"arg {k} const {self.constant_value(c, g.get('constant'))} type={cty} @ {gpos}")
                elif gk == "type":
                    kids.append(f"arg {k} type {self.tl(g.get('type'))} @ {gpos}")
                else:
                    kids.append(f"arg {k} <unsupported argument kind> @ {gpos}")
            return Block(head, kids)
        return Block(f"attr {self.render_attribute(a)} on {on} @ {il.pos(a, 'position')}")

    def attr_blocks(self, head_value: Optional[str], on: str, owner: Optional[Rec]) -> List[Block]:
        return [self.attr_block(a, on, owner) for a in self.attr_list(head_value) if not self.is_pseudo(a)]

    # -- constraints ---------------------------------------------------------------

    def render_constraint(self, e: Optional[Rec]) -> str:
        il = self.il
        if e is None:
            return "<null>"
        k = e.get("kind")
        if k == "enk_concept_id":
            t = il.ref(e.get("concept_id.concept_template"))
            s = self.qname(t) if t is not None else "<null>"
            if any(kk == "concept_id.args" and v == "" for kk, v in e.fields):
                args = []
                for kk, v in e.fields:
                    if kk.startswith("concept_id.args."):
                        sub = kk[len("concept_id.args.") :]
                        if sub == "type":
                            args.append(self.tl(v))
                        elif sub == "constant":
                            c = il.ref(v)
                            args.append(self.constant_value(c, v))
                        elif sub == "template":
                            tt = il.ref(v)
                            args.append(self.qname(tt) if tt is not None else "<template>")
                if args:
                    s += "<" + ", ".join(args) + ">"
            return s
        if k == "enk_operation":
            kind = e.get("operation.kind")
            ops = list(il.chain(il.ref(e.get("operands"))))
            if kind in ("eok_land", "eok_lor") and len(ops) >= 2:
                def side(n: Rec) -> str:
                    r = self.render_constraint(n)
                    other = n.get("kind") == "enk_operation" and n.get("operation.kind") in ("eok_land", "eok_lor") \
                        and n.get("operation.kind") != kind
                    return "(" + r + ")" if other else r
                return side(ops[0]) + (" && " if kind == "eok_land" else " || ") + side(ops[1])
            return f"<expr:{kind}>"
        return f"<expr:{k}>"

    def constraints(self, r: Rec, tmpl: Optional[Rec]) -> List[str]:
        il = self.il
        out: List[str] = []
        d = il.ref(tmpl.get("template_decl")) if tmpl is not None else None
        if d is not None:
            index = -1
            for tp in il.chain(il.ref(d.get("param_list"))):
                index += 1
                if tp.get("kind") != "tpk_type":
                    continue
                ty = self.skip_typerefs(il.ref(tp.get("ptr")))
                if ty is None or self.kind(ty) != "tk_template_param":
                    continue
                tc = il.ref(ty.get("constraint.type_constraint"))
                if tc is None:
                    continue
                nm = il.name_of(tp) or f"#{index}"
                out.append(f"constraint {'abbreviated' if tp.flag('is_abbreviated') else 'type-constraint'} {nm} "
                           f"{self.render_constraint(tc)}")
            rc = il.ref(d.get("constraint.requires_clause"))
            if rc is not None:
                out.append("constraint requires " + self.render_constraint(il.ref(rc.get("constraint"))))
        tr = il.ref(r.get("trailing_requires_clause"))
        if tr is not None:
            out.append("constraint trailing " + self.render_constraint(il.ref(tr.get("constraint"))))
        return out

    # -- bodies --------------------------------------------------------------------

    STMT_KEYS_SINGLE = ("statement", "expr", "dynamic_init", "initialization", "increment", "incr_call_expr",
                        "ne_call_expr", "then_statement", "else_statement", "body_statement", "loop_statement",
                        "switch_statement", "return_dynamic_init")

    def walk_body(self, r: Rec) -> "BodyInfo":
        il = self.il
        info = BodyInfo()
        sc = self.fn_scope.get(r.id)
        if sc is None:
            return info
        proto = r.flag("is_prototype_instantiation")
        seen: set = set()
        consts_seen: set = set()
        state = {"own_default": False}

        def visit_expr(e: Optional[Rec]) -> None:
            if e is None or e.id in seen:
                return
            seen.add(e.id)
            info.nodes.add(e.id)
            k = e.get("kind")
            if k == "enk_operation" and proto:
                info.ops.append(f"op {e.get('operation.kind')} type_kind={e.get('operation.type_kind')} "
                                f"type={self.tl(e.get('type'))} @ {il.pos(e, 'position')}")
            if k == "enk_constant":
                c = il.ref(e.get("constant"))
                visit_constant(c, e)
            for c in il.chain(il.ref(e.get("operands"))) if e.get("operands") else []:
                visit_expr(c)
            di = il.ref(e.get("dynamic_init"))
            if di is not None:
                visit_dyn(di)
            if k not in ("enk_constant",):
                for v in e.all("expr"):
                    visit_expr(il.ref(v))

        def followable(x: Rec) -> bool:
            return state["own_default"] or x.scope == "func-scope"

        def visit_constant(c: Optional[Rec], node: Optional[Rec]) -> None:
            if c is None:
                # The constant entry is not dumped (a file-scope entry nothing
                # in the file scope lists): only the use is known.
                if proto and node is not None and ref_parts(node.get("constant")) is not None:
                    info.tconsts.append("tconst ? @ " + il.pos(node, "position") + "|-|*")
                return
            if c.get("kind") == "ck_template_param" and proto:
                key = (c.id, node.id if node is not None else None)
                if key not in consts_seen:
                    consts_seen.add(key)
                    kinds = [v for kk, v in c.fields if kk == "kind"]
                    cands = []
                    if node is not None:
                        cands.append(il.pos(node, "position"))
                    cands.append(il.pos(c, "decl_position"))
                    cands.append("-")
                    info.tconsts.append(f"tconst {kinds[1] if len(kinds) > 1 else '?'} type={self.tl(c.get('type'))} @ " + "|".join(cands))
            for v in c.all("expr"):
                x = il.ref(v)
                if x is not None and not il.name_of(c) and followable(x):
                    visit_expr(x)
            for v in c.all("dynamic_init"):
                visit_dyn(il.ref(v))

        def visit_dyn(d: Optional[Rec]) -> None:
            if d is None or d.id in seen:
                return
            seen.add(d.id)
            for v in d.all("expression"):
                visit_expr(il.ref(v))
            for e in il.chain(il.ref(d.get("args"))) if d.get("args") else []:
                visit_expr(e)
            for v in d.all("constant"):
                visit_constant(il.ref(v), None)

        def visit_var(v: Optional[Rec]) -> None:
            if v is None:
                return
            for x in v.all("dynamic"):
                visit_dyn(il.ref(x))
            for x in v.all("constant"):
                visit_constant(il.ref(x), None)

        def visit_stmt(s: Optional[Rec]) -> None:
            if s is None or s.id in seen:
                return
            seen.add(s.id)
            info.stmts.append(s)
            for st in il.chain(il.ref(s.get("statements"))) if s.get("statements") else []:
                visit_stmt(st)
            for key in self.STMT_KEYS_SINGLE:
                for v in s.all(key):
                    t = il.ref(v)
                    if t is None:
                        continue
                    if t.kind == "statement":
                        visit_stmt(t)
                    elif t.kind == "expr-node":
                        visit_expr(t)
                    elif t.kind == "dynamic-init":
                        visit_dyn(t)
            for key in ("entities", "begin", "end", "range", "iterator"):
                for v in il.refs(s, key):
                    visit_var(v)

        # default arguments of the routine's own parameters (file-scope expressions).
        state["own_default"] = True
        for p in self.params(r):
            if p.get("default_arg_expr"):
                visit_expr(il.ref(p.get("default_arg_expr")))
        state["own_default"] = False
        for ci in il.chain(il.ref(sc.get("constructor_inits"))) if sc.get("constructor_inits") else []:
            visit_dyn(il.ref(ci.get("initializer")))
            for v in ci.all("source_expr"):
                visit_expr(il.ref(v))

        def static_inits(scp: Rec) -> None:
            head = il.ref(scp.get("local_static_variable_inits"))
            for li in il.chain(head) if head is not None else []:
                if li.get("init_kind") == "initk_static":
                    for v in li.all("constant"):
                        visit_constant(il.ref(v), None)
            for c in il.chain(il.ref(scp.get("scopes"))):
                if c.get("kind") in ("sck_block", "sck_condition"):
                    static_inits(c)

        static_inits(sc)
        visit_stmt(il.ref(sc.get("assoc_block")))
        return info


class BodyInfo:
    """What a routine's body reaches: expression nodes, statements, and the
    `op` / `tconst` lines the listing prints for a prototype."""

    def __init__(self) -> None:
        self.nodes: set = set()
        self.ops: List[str] = []
        self.tconsts: List[str] = []
        self.stmts: List[Rec] = []


CONST_MARK = "\x00CONST"
MISSING_ATTR = Rec("attribute", "missing", "file-scope", 1)
MISSING_MARK = "\x00MISSING"


class TreeCtx:
    def __init__(self, owner: Optional[Rec], annot: str) -> None:
        self.owner = owner
        self.annot = annot
        self.binder_name: Optional[str] = None
        self.visited: set = set()


def _extractor_views(self: Extractor) -> Tuple[List[View], Dict[str, "ViewExtra"]]:
    il = self.il
    views: List[View] = []
    extras: Dict[str, ViewExtra] = {}
    for r in il.by_kind.get("routine", []):
        seq = r.get("decl_position.seq")
        if not seq or int(seq) == 0 or not il.is_primary_seq(int(seq)):
            continue
        kind = self.routine_kind(r)
        if r.flag("is_prototype_instantiation") and r.flag("is_specialized") \
                and not r.flag("is_in_class_specialization"):
            self.view_mismatches.append(
                f"routine {self.routine_name(r)} @ {il.pos(r, 'decl_position')}: the IL says "
                "is_prototype_instantiation and is_specialized without is_in_class_specialization, "
                "a combination this oracle does not explain")
        sc = self.fn_scope.get(r.id)
        header = f"routine {self.routine_name(r)} {kind}" + (" implicit" if r.flag("compiler_generated") else "") + \
                 (" body" if sc is not None else " nobody") + f" @ {il.pos(r, 'decl_position')}"
        v = View(header)
        rt = self.routine_type(r)
        if rt is not None:
            v.sig.append("returns " + self.tl(rt.get("return_type")))
            for i, p in enumerate(self.params(r), start=1):
                v.sig.append(f"param {i} {il.name_of(p) or '_'}: {self.tl(p.get('type'))}")
                for a in self.attr_list(p.get("attributes")):
                    if not self.is_pseudo(a):
                        v.pattr.append(self.attr_block(a, f"param {i}", r).head)
        else:
            v.sig.append("returns <not-a-routine-type>")
        tmpl = il.ref(r.get("assoc_template"))
        if kind == "prototype" and tmpl is None:
            tmpl = self.proto_owner.get(r.id)
        if kind in ("instance", "specialization"):
            tn = self.qname(tmpl) if tmpl is not None else "<none>"
            v.tmpl.append(f"template {tn} args <{self.template_args(tmpl, r) if any(k == 'template_arg_list' for k, _ in r.fields) else 'none'}>")
        if kind == "prototype":
            v.cons.extend(self.constraints(r, tmpl))
        v.attrs.extend(self.attr_blocks(r.get("attributes"), "routine", r))
        t = il.ref(r.get("type"))
        guard = 0
        while t is not None and guard < 64:
            guard += 1
            if self.kind(t) != "tk_typeref":
                v.attrs.extend(self.attr_blocks(t.get("attributes"), "type", r))
                break
            ks = [x for kk, x in t.fields if kk == "kind"]
            if len(ks) > 1 and ks[1] == "trk_for_type_attributes":
                v.attrs.extend(self.attr_blocks(t.get("attributes"), "type", r))
            t = il.ref(t.get("typeref_type"))
        if sc is not None:
            self.variable_attrs(v, sc)
        info = self.walk_body(r)
        v.ops = info.ops
        v.tconsts = info.tconsts
        ex = ViewExtra(info, r)
        extras[v.header + "\x00" + "\n".join(v.sig + v.tmpl)] = ex
        views.append(v)
    return views, extras


def _variable_attrs(self: Extractor, v: View, sc: Rec) -> None:
    il = self.il

    def each(head: Optional[str]) -> None:
        for var in il.chain(il.ref(head)):
            nm = il.name_of(var) or "<unnamed>"
            v.attrs.extend(self.attr_blocks(var.get("attributes"), f"variable {nm}", None))

    each(sc.get("nonstatic_variables"))
    each(sc.get("variables"))
    if sc.get("kind") == "sck_function":
        each(sc.get("parameters"))
    for c in il.chain(il.ref(sc.get("scopes"))):
        if c.get("kind") in ("sck_block", "sck_condition"):
            self.variable_attrs(v, c)


Extractor.views = _extractor_views  # type: ignore[attr-defined]
Extractor.variable_attrs = _variable_attrs  # type: ignore[attr-defined]


@dataclass
class ViewExtra:
    info: BodyInfo
    routine: Rec


# ---------------------------------------------------------------------------
# Part 3b: comparison
# ---------------------------------------------------------------------------


@dataclass
class Report:
    """The result for one golden.

    ``checked_by`` counts the facts il_display confirmed, by category;
    ``unverifiable`` counts claims of the listing that il_display cannot show
    (and that were therefore not checked), by category; ``mismatches`` are
    claims il_display contradicts, or il_display facts the listing lacks."""

    checked_by: "collections.Counter[str]" = field(default_factory=collections.Counter)
    unverifiable: "collections.Counter[str]" = field(default_factory=collections.Counter)
    mismatches: List[str] = field(default_factory=list)
    notes: List[str] = field(default_factory=list)

    @property
    def checked(self) -> int:
        return sum(self.checked_by.values())

    def fact(self, category: str, n: int = 1) -> None:
        if n:
            self.checked_by[category] += n

    @property
    def ok(self) -> bool:
        return not self.mismatches and self.checked > 0


def tree_match(g: List[str], i: List[str]) -> Optional[int]:
    """Compare tree lines.  An il_display line for a constant entry that is not
    dumped matches a ``const``/``tconst`` line with the same type and position,
    and the lines the listing prints under it.  Returns the number of such
    placeholders used, or None when the trees differ."""
    gi = ii = 0
    used = 0
    while ii < len(i):
        if gi >= len(g):
            return None
        gl, il_line = g[gi], i[ii]
        stripped = il_line.lstrip(" ")
        if stripped.startswith(CONST_MARK):
            ind = len(il_line) - len(stripped)
            suffix = stripped[len(CONST_MARK):]
            m = re.match(r"^( *)t?const .*?( type=.* @ \S+)$", gl)
            if not m or len(m.group(1)) != ind or m.group(2) != suffix:
                return None
            used += 1
            gi += 1
            while gi < len(g) and len(g[gi]) - len(g[gi].lstrip(" ")) > ind:
                gi += 1
            ii += 1
            continue
        if gl != il_line:
            return None
        gi += 1
        ii += 1
    return used if gi == len(g) else None


def tree_eq(g: List[str], i: List[str], rep: Optional[Report] = None) -> bool:
    n = tree_match(g, i)
    if n is None:
        return False
    if rep is not None and n:
        rep.unverifiable["constant-entry-not-dumped (kind, value, operand)"] += n
    return True


def attr_matches(g: str, i: str, rep: Optional[Report] = None) -> int:
    """0 no, 1 equal, 2 matched a placeholder for an attribute entry that
    il_display does not dump (it stands for any number of attributes), 3 equal
    except for a constant entry that il_display does not dump."""
    if g == i:
        return 1
    if MISSING_MARK in i:
        on = i.split(" on ", 1)[1]
        if re.match(r"^attr \[\[.*\]\] on " + re.escape(on) + r" @ \S+(\n|$)", g) and "cerastes::" not in g.split("\n")[0]:
            return 2
        return 0
    if CONST_MARK in i:
        gl, il_l = g.split("\n"), i.split("\n")
        if gl[0] == il_l[0] and tree_eq(gl[1:], il_l[1:], rep):
            return 3
    return 0


def is_subsequence(small: List[str], big: List[str], rep: Report) -> Optional[str]:
    """None when ``small`` is an ordered subsequence of ``big``, else the first
    element that is not.  A placeholder in ``big`` may match several."""
    i = 0
    for s in small:
        while i < len(big):
            m = attr_matches(s, big[i], rep)
            if m:
                if m == 2 and MISSING_MARK in big[i]:
                    rep.unverifiable["attribute-entry-not-dumped"] += 1
                if m in (1, 3):
                    i += 1
                break
            i += 1
        else:
            return s
    return None


def multiset_missing(small: List[str], big: List[str]) -> List[str]:
    c = collections.Counter(big)
    out = []
    for s in small:
        if c[s] > 0:
            c[s] -= 1
        else:
            out.append(s)
    return out


def outside_user_bodies(il: IL, ex: Extractor) -> int:
    """The routines with a body whose declaration is in a non-system file other than
    the primary one (a user header): the second count of the `end walk:` line, read
    from il_display's source-file entries, not from the walker's filter."""
    n = 0
    for r in il.by_kind.get("routine", []):
        seq = r.get("decl_position.seq")
        if not seq or int(seq) == 0 or il._file_for_seq(int(seq)) is None:
            continue
        s = int(seq)
        if il.is_primary_seq(s) or il.in_system(s):
            continue
        if ex.fn_scope.get(r.id) is not None:
            n += 1
    return n


def analyse(golden_text: str, il_text: str, source_text: Optional[str] = None) -> Report:
    rep = Report()
    n_wild = len(_WILD_RE.findall(golden_text))
    if n_wild:
        rep.unverifiable["dependent-type-kind (<tptk_*>)"] += n_wild
    g = parse_golden(wild(golden_text))
    il = IL(il_text)
    ex = Extractor(il)
    ex.source_lines = source_text.split("\n") if source_text is not None else None
    il_views, extras = ex.views()
    if g.end is None:
        rep.mismatches.append("the golden has no `end walk:` line")
    # Routine sets: every routine of the listing is in il_display and the other way round.
    gk = collections.Counter(v.key for v in g.views)
    ik = collections.Counter(v.key for v in il_views)
    for k, n in (gk - ik).items():
        rep.mismatches.append(f"listing routine not found in il_display (x{n}):\n  " + k.replace("\n", "\n  "))
    for k, n in (ik - gk).items():
        rep.mismatches.append(f"il_display routine missing from the listing (x{n}):\n  " + k.replace("\n", "\n  "))
    rep.fact("routine", sum((gk & ik).values()))
    if g.end is not None and g.end[0] != len(g.views):
        rep.mismatches.append(f"`end walk:` says {g.end[0]} routines, the listing has {len(g.views)}")
    if g.end is not None:
        outside = outside_user_bodies(il, ex)
        if g.end[1] != outside:
            rep.mismatches.append(f"`end walk:` says {g.end[1]} routines with bodies in non-system "
                                  f"files outside the primary file, il_display shows {outside}")
        elif outside:  # (a zero count is no fact: a listing of nothing must still fail)
            rep.fact("end-walk-user-header-bodies")
    rep.mismatches.extend(ex.view_mismatches)
    # Per routine.
    by_key: Dict[str, List[View]] = collections.defaultdict(list)
    for v in il_views:
        by_key[v.key].append(v)
    owners: collections.Counter = collections.Counter()
    for v in il_views:
        for aid in {b.aid for b in v.attrs if b.aid}:
            owners[aid] += 1
    shared_attrs = {a for a, n in owners.items() if n > 1}
    listed_texts = {b.text() for gv in g.views for b in gv.attrs}
    for v in il_views:
        for b in v.attrs:
            if b.aid in shared_attrs and b.text() not in listed_texts and not any(attr_matches(t, b.text()) for t in listed_texts):
                rep.mismatches.append(f"{v.header}: il_display attribute (shared by several owners) is listed under none: {b.head}")
                listed_texts.add(b.text())
    for gv in g.views:
        cands = by_key.get(gv.key)
        if not cands:
            continue
        iv = cands.pop(0) if len(cands) > 1 else cands[0]
        label = gv.header
        compare_view(rep, label, gv, iv, ex, extras, shared_attrs)
    # Statement attributes: the pointer of every attributed statement.
    # Top-level attributes and listing warnings.
    for w in g.warnings:
        rep.unverifiable["listing-warning-line"] += 1
    check_top(rep, g, il, ex)
    return rep


def attr_unlisted(b: Block, listed: List[Block]) -> bool:
    return not any(g.text() == b.text() or attr_matches(g.text(), b.text()) for g in listed)


def compare_view(rep: Report, label: str, gv: View, iv: View, ex: Extractor, extras: Dict[str, ViewExtra],
                 shared_attrs: set) -> None:
    def fail(what: str, detail: str) -> None:
        rep.mismatches.append(f"{label}: {what}: {detail}")

    # signature, template, constraints, param attributes are complete lists.
    for name, gl, il_l in (("pattr", gv.pattr, iv.pattr), ("tmpl", gv.tmpl, iv.tmpl), ("cons", gv.cons, iv.cons)):
        if gl != il_l:
            fail(name, f"listing {gl!r} != il_display {il_l!r}")
        else:
            rep.fact(name, len(gl))
    # attributes on the routine, its type and its variables: an ordered subsequence.
    gh = [b.text() for b in gv.attrs]
    ih = [b.text() for b in iv.attrs]
    miss = is_subsequence(gh, ih, rep)
    # An attribute entry that only this routine owns must be listed here (the
    # listing prints each unrecognized entry once, so a shared one may be
    # listed under another owner; those are checked globally).
    unlisted_attrs = [b for b in iv.attrs if b.aid and b.aid not in shared_attrs and attr_unlisted(b, gv.attrs)]
    for b in unlisted_attrs:
        fail("attribute", "il_display attribute missing from the listing: " + b.head)
    if miss is not None:
        fail("attribute", f"not in il_display (or out of order):\n    " + miss.replace("\n", "\n    ") +
             "\n  il_display has:\n    " + "\n    ".join(h.replace("\n", "\n    ") for h in ih))
    else:
        rep.fact("attribute", len(gh))
        # registered ones are printed once per owner: the lists must agree.
        greg = [t for t in gh if t.startswith("attr [[cerastes::")]
        ireg = [t for t in ih if t.startswith("attr [[cerastes::")]
        if greg != ireg:
            fail("registered attributes", f"listing {len(greg)} vs il_display {len(ireg)}")
    # prototype body operations and template-parameter constants: multisets.
    bad = multiset_missing(gv.ops, iv.ops)
    if bad:
        fail("op", "not in il_display: " + "; ".join(bad[:5]))
    else:
        rep.fact("op", len(gv.ops))
    extra = multiset_missing(iv.ops, gv.ops)
    if extra:
        fail("op", "il_display has operation(s) the listing does not report: " + "; ".join(extra[:5]))
    # tconst lines: kind and type must match; a position other than `-` must
    # be the position of a use or the declaration of the constant (the
    # listing prints `-` when its own bookkeeping lost the position).
    avail = []
    for t in iv.tconsts:
        base, _, cands = t.rpartition(" @ ")
        avail.append((base, set(cands.split("|"))))
    left = []
    for t in sorted(gv.tconsts, key=lambda x: x.endswith(" @ -")):
        base, _, pos = t.rpartition(" @ ")
        for i, (b, cs) in enumerate(avail):
            if (b == base or b == "tconst ?") and (pos == "-" or pos in cs or "*" in cs):
                del avail[i]
                if b == "tconst ?":
                    rep.unverifiable["constant-entry-not-dumped (kind of tconst)"] += 1
                else:
                    rep.fact("tconst")
                break
        else:
            left.append(t)
    if left:
        fail("tconst", "not in il_display: " + "; ".join(left[:5]))
    unlisted = [b for b, _cs in avail if b != "tconst ?"]
    if unlisted:
        fail("tconst", "il_display has template-parameter constant(s) the listing does not report: " + "; ".join(unlisted[:5]))
    for _m in gv.misc:
        rep.unverifiable["listing-internal-line"] += 1
    # statement attributes
    check_stmts(rep, label, gv, iv, ex, extras)


def child_nodes(il: IL, e: Rec) -> List[Rec]:
    out: List[Rec] = []
    if e.get("operands"):
        out.extend(il.chain(il.ref(e.get("operands"))))
    for v in e.all("expr"):
        x = il.ref(v)
        if x is not None:
            out.append(x)
    c = il.ref(e.get("constant")) if e.get("kind") == "enk_constant" else None
    if c is not None and not il.name_of(c):
        for v in c.all("expr"):
            x = il.ref(v)
            if x is not None and x.scope == "func-scope":
                out.append(x)
    di = il.ref(e.get("dynamic_init"))
    if di is not None:
        for v in di.all("expression"):
            x = il.ref(v)
            if x is not None:
                out.append(x)
    return out


def orphan_pool(ex: Extractor, region: int) -> Dict[str, List[set]]:
    """The annotation trees whose nodes no body reaches, in a function memory
    region: the argument trees of statement attributes (il_display does not
    print the attribute entries, but the expression nodes are in the region).
    Keyed by the text of the tree rooted at each such node."""
    cache = ex.__dict__.setdefault("_pools", {})
    if region in cache:
        return cache[region]
    il = ex.il
    if "_reached" not in ex.__dict__:
        reached: set = set()
        for r in il.by_kind.get("routine", []):
            if r.id in ex.fn_scope:
                reached |= ex.walk_body(r).nodes
        ex.__dict__["_reached"] = reached
    reached = ex.__dict__["_reached"]
    orphans = [e for e in il.by_kind.get("expr-node", []) if e.region == region and e.id not in reached]
    kids = set()
    for e in orphans:
        for c in child_nodes(il, e):
            kids.add(c.id)
    pool: Dict[str, List[set]] = collections.defaultdict(list)
    for e in orphans:
        if e.id in kids:
            continue
        out: List[str] = []
        ex.tree(e, 0, out, TreeCtx(None, "check"))
        # The argument's own position is the start of its expression (or the
        # node's position for a node the front end generated).
        # EDG's expr_range start is not always the leftmost token (member calls),
        # so any node of the tree may carry it: all their positions are kept.
        poss = set()
        stack = [e]
        while stack:
            n = stack.pop()
            poss.add(il.pos(n, "start", annot=True))
            poss.add(il.pos(n, "position", annot=True))
            stack.extend(child_nodes(il, n))
        pool["\n".join(out)].append(poss)
    cache[region] = pool
    return pool


def arg_trees(kids: List[str]) -> List[Tuple[str, List[str]]]:
    """Split a registered attribute's child lines into (arg line, tree lines)."""
    out: List[Tuple[str, List[str]]] = []
    for k in kids:
        if k.startswith("arg "):
            out.append((k, []))
        elif out:
            out[-1][1].append(k[2:] if k.startswith("  ") else k)
    return out


def check_stmts(rep: Report, label: str, gv: View, iv: View, ex: Extractor, extras: Dict[str, ViewExtra]) -> None:
    ext = extras.get(iv.header + "\x00" + "\n".join(iv.sig + iv.tmpl))
    il = ex.il
    if ext is None:
        if gv.stmts:
            rep.mismatches.append(f"{label}: statement attributes listed for a routine without a body in il_display")
        return
    attributed = collections.Counter()
    for s in ext.info.stmts:
        if ref_parts(s.get("attributes")) is not None and s.get("position.seq"):
            for annot in (False, True):
                attributed[f"stmt {s.get('kind')} @ {il.pos(s, 'position', annot=annot)}"] += 1
    listed = set()
    pool = None
    for b in gv.stmts:
        m = re.match(r"^(stmt \S+ @ \S+) attr ", b.head)
        if not m:
            rep.mismatches.append(f"{label}: unparsable statement line {b.head!r}")
            continue
        listed.add(m.group(1))
        if attributed[m.group(1)] == 0:
            rep.mismatches.append(f"{label}: no statement with an attribute pointer at {m.group(1)!r} in il_display")
        else:
            rep.fact("statement-attribute")
        if "attr [[cerastes::" in b.head:
            rep.unverifiable["statement-attribute-name-and-arg-lines"] += 1
            if pool is None:
                region = int(ext.routine.get("memory_region") or 0)
                pool = orphan_pool(ex, region)
            groups = arg_trees(b.kids)
            if not b.kids or not b.kids[0].startswith("arg "):
                rep.mismatches.append(f"{label}: {m.group(1)}: a cerastes:: statement attribute without `arg` lines")
            for arg, tree in groups:
                if not tree:
                    continue
                key = "\n".join(tree)
                hit = next((k for k in pool if tree_match(tree, k.split("\n")) is not None), None)
                if hit is not None:
                    rep.fact("statement-annotation-tree")
                    nw = tree_match(tree, hit.split("\n")) or 0
                    if nw:
                        rep.unverifiable["constant-entry-not-dumped (kind, value, operand)"] += nw
                    apos = arg.rsplit(" @ ", 1)[-1]
                    if not any(apos in poss or poss <= {"-"} for poss in pool[hit]):
                        rep.mismatches.append(f"{label}: {m.group(1)}: `{arg}` is at no node of its expression "
                                              f"(il_display positions: {sorted(pool[hit][0])})")
                    else:
                        rep.fact("statement-annotation-arg-position")
                else:
                    rep.mismatches.append(f"{label}: {m.group(1)}: annotation tree not among the expression nodes of "
                                          f"the routine's memory region:\n    " + key.replace("\n", "\n    "))
        else:
            rep.unverifiable["statement-attribute-tokens"] += 1
    # Every statement that carries an attribute pointer should be listed.
    seen = set()
    for s in ext.info.stmts:
        if ref_parts(s.get("attributes")) is None or not s.get("position.seq"):
            continue
        keys = {f"stmt {s.get('kind')} @ {il.pos(s, 'position', annot=a)}" for a in (False, True)}
        if not (keys & listed):
            # An empty attribute list `[[]]` is an attribute entry the listing does not print;
            # with the source at hand the oracle can tell it from a dropped attribute.
            m = re.match(r"stmt \S+ @ L(\d+):", sorted(keys)[0])
            lines = getattr(ex, "source_lines", None)
            if m and lines is not None and int(m.group(1)) <= len(lines) and \
                    re.search(r"\[\[\s*\]\]", lines[int(m.group(1)) - 1]):
                rep.fact("statement-empty-attribute")
                continue
            rep.mismatches.append(f"{label}: il_display shows an attribute on {sorted(keys)[0]!r} that the listing "
                                  "does not report (pass the source text to analyse() to rule out an empty `[[]]`)")


def top_candidates(ex: Extractor) -> List[Block]:
    """`attr ... on <entity>` blocks the listing prints at the top level: the
    unrecognized and cerastes:: attributes of entities that are not routines
    (their routine lines carry their own)."""
    il = ex.il
    out: List[Block] = []

    def add(head: Optional[str], on: str, seqrec: Optional[Rec] = None) -> None:
        for a in ex.attr_list(head):
            if a is MISSING_ATTR:
                out.append(ex.attr_block(a, on, None))
                continue
            if ex.is_pseudo(a):
                continue
            if not (a.get("kind") == "unrecognized" or il.name_of(a, "namespace_name") == "cerastes"):
                continue
            seq = a.get("position.seq")
            if seq and not il.is_primary_seq(int(seq)):
                continue
            out.append(ex.attr_block(a, on, None))

    def primary(r: Rec) -> bool:
        seq = r.get("decl_position.seq")
        return bool(seq) and int(seq) != 0 and il.is_primary_seq(int(seq))

    for t in il.by_kind.get("type", []):
        if ref_parts(t.get("attributes")) is None or not primary(t):
            continue
        alias = ex.kind(t) == "tk_typeref"
        add(t.get("attributes"), ("typedef " if alias else "type ") + ex.nl(il.rec_label(t)).replace(WILD_TYPE, "<unnamed>"))
    for v in il.by_kind.get("variable", []):
        if ref_parts(v.get("attributes")) is None or not primary(v):
            continue
        if il.ref(v.get("enclosing_routine")) is not None or v.flag("is_local_to_function"):
            continue  # a local: listed under its routine
        add(v.get("attributes"), "variable " + ex.qname(v))
    for f in il.by_kind.get("field", []):
        if ref_parts(f.get("attributes")) is not None and primary(f):
            add(f.get("attributes"), "field " + ex.qname(f))
    for n in il.by_kind.get("namespace", []):
        if ref_parts(n.get("attributes")) is not None and primary(n):
            add(n.get("attributes"), "namespace " + (il.rec_label(n) or il.name_of(n)))
    for t in il.by_kind.get("template", []):
        if ref_parts(t.get("attributes")) is not None and primary(t):
            add(t.get("attributes"), "template " + ex.qname(t))
    return out


def check_top(rep: Report, g: Golden, il: IL, ex: Extractor) -> None:
    cands = top_candidates(ex)
    for b in g.top:
        if not b.head.startswith("attr "):
            rep.mismatches.append(f"unrecognized top-level listing line {b.head!r}")
            continue
        text = b.text()
        for i, c in enumerate(cands):
            m = attr_matches(text, c.text(), rep)
            if m:
                if m == 2:
                    rep.unverifiable["attribute-entry-not-dumped"] += 1
                rep.fact("top-level-attribute")
                del cands[i]
                break
        else:
            rep.mismatches.append(f"top-level attribute not found on its entity in il_display:\n    "
                                  + text.replace("\n", "\n    ") + "\n  candidates:\n    "
                                  + "\n    ".join(c.head for c in cands[:8]))
    for c in cands:
        rep.mismatches.append("il_display shows an attribute the listing does not report: " + c.head.replace(MISSING_MARK, "(entry not dumped)"))


# ---------------------------------------------------------------------------
# Part 4: running the front end, discovering tests, the command line
# ---------------------------------------------------------------------------

MODE_OPTIONS = {"c++20": "--c++20", "c23": "--c23"}
DEFAULT_PATHS = ("tests", "doc/modules/ROOT/examples")
FE_SCRIPT = ROOT / "scripts" / "cerastes-fe.sh"
FE_TIMEOUT_S = 120


class OracleError(Exception):
    pass


def read_header(text: str) -> Tuple[bool, List[str]]:
    """(is_golden, explicit modes) from the leading comment block of a test."""
    golden = False
    modes: List[str] = []
    for raw in text.splitlines():
        line = raw.strip()
        if line and not line.startswith("//"):
            break
        m = re.match(r"^//\s*expect:\s*(.*?)\s*$", line)
        if m and m.group(1) == "golden":
            golden = True
        m = re.match(r"^//\s*mode:\s*(.*?)\s*$", line)
        if m:
            modes.extend(s.strip() for s in m.group(1).split(","))
    return golden, modes


def golden_file(source: Path, mode: str, modes: List[str]) -> Path:
    if len(modes) == 1:
        return source.with_name(source.name + ".golden")
    return source.with_name(f"{source.name}.{mode}.golden")


def run_il_display(source: Path, mode: str, cache: Optional[Path] = None) -> str:
    """Run the cerastes front end with --il_display on ``source``."""
    rel = os.path.relpath(source, ROOT)
    cpath = None
    if cache is not None:
        cpath = cache / (rel.replace("/", "__") + "." + mode + ".il")
        if cpath.is_file():
            return cpath.read_text(errors="replace")
    cmd = [str(FE_SCRIPT), MODE_OPTIONS[mode], "--il_display", rel]
    try:
        p = subprocess.run(cmd, cwd=ROOT, capture_output=True, timeout=FE_TIMEOUT_S)
    except subprocess.TimeoutExpired:
        raise OracleError(f"{rel} [{mode}]: front end timed out") from None
    out = p.stdout.decode("utf-8", "replace")
    if p.returncode != 0 or not out.strip():
        err = p.stderr.decode("utf-8", "replace").strip().splitlines()[-3:]
        raise OracleError(f"{rel} [{mode}]: front end failed (status {p.returncode}): {' | '.join(err)}")
    if cpath is not None:
        cache.mkdir(parents=True, exist_ok=True)
        cpath.write_text(out)
    return out


@dataclass
class Job:
    source: Path
    mode: str
    golden: Path


def discover(paths: List[str]) -> List[Job]:
    files: List[Path] = []
    for p in paths:
        pp = Path(p)
        if not pp.is_absolute():
            pp = (Path.cwd() / pp)
        if pp.is_dir():
            for f in sorted(pp.rglob("*")):
                if f.suffix in (".c", ".cpp") and f.is_file():
                    files.append(f)
        elif pp.suffix == ".golden":
            # <src>.golden or <src>.<mode>.golden
            stem = pp.name[: -len(".golden")]
            m = re.match(r"^(.*\.(?:c|cpp))(?:\.(c\+\+20|c23))?$", stem)
            if not m:
                raise OracleError(f"{p}: not a golden file name")
            files.append(pp.with_name(m.group(1)))
        elif pp.is_file():
            files.append(pp)
        else:
            raise OracleError(f"{p}: no such file or directory")
    seen = set()
    jobs: List[Job] = []
    for f in files:
        if f in seen:
            continue
        seen.add(f)
        golden, modes = read_header(f.read_text(errors="replace"))
        if not golden:
            continue
        if not modes:
            modes = ["c23"] if f.suffix == ".c" else ["c++20"]
        for mode in modes:
            g = golden_file(f, mode, modes)
            if not g.is_file():
                raise OracleError(f"{os.path.relpath(f, ROOT)} [{mode}]: golden file {os.path.relpath(g, ROOT)} is missing")
            jobs.append(Job(f, mode, g))
    return jobs


def check_job(job: Job, cache: Optional[Path] = None) -> Tuple[Job, Optional[Report], Optional[str]]:
    try:
        il = run_il_display(job.source, job.mode, cache)
        rep = analyse(job.golden.read_text(errors="replace"), il, job.source.read_text(errors="replace"))
        return job, rep, None
    except OracleError as e:
        return job, None, str(e)


def main(argv: Optional[List[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("paths", nargs="*", help="goldens, sources or directories (default: tests, doc examples)")
    ap.add_argument("-v", "--verbose", action="store_true", help="per-golden counts for every golden, and the facts")
    ap.add_argument("-j", "--jobs", type=int, default=min(8, os.cpu_count() or 2))
    ap.add_argument("--cache", type=Path, help="directory for il_display output (development)")
    args = ap.parse_args(argv)
    t0 = time.time()
    try:
        jobs = discover(args.paths or list(DEFAULT_PATHS))
    except OracleError as e:
        print(f"walker_oracle: {e}", file=sys.stderr)
        return 2
    if not jobs:
        print("walker_oracle: no golden tests found", file=sys.stderr)
        return 2
    results = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, args.jobs)) as ex:
        for r in ex.map(lambda j: check_job(j, args.cache), jobs):
            results.append(r)
    bad = 0
    total = collections.Counter()
    by_cat: collections.Counter = collections.Counter()
    unv_total: collections.Counter = collections.Counter()
    for job, rep, err in results:
        name = f"{os.path.relpath(job.source, ROOT)} [{job.mode}]"
        if err is not None:
            bad += 1
            print(f"FAIL {name}: {err}")
            continue
        assert rep is not None
        total["checked"] += rep.checked
        by_cat.update(rep.checked_by)
        unv_total.update(rep.unverifiable)
        if not rep.ok:
            bad += 1
            print(f"FAIL {name}: {rep.checked} checked, {sum(rep.unverifiable.values())} unverifiable")
            if rep.checked == 0:
                print("  no checkable fact: a golden that checks nothing is a failure")
            for m in rep.mismatches[:40]:
                print("  " + m.replace("\n", "\n  "))
            if len(rep.mismatches) > 40:
                print(f"  ... {len(rep.mismatches) - 40} more")
        elif args.verbose:
            unv = ", ".join(f"{k}={v}" for k, v in sorted(rep.unverifiable.items())) or "none"
            cats = ", ".join(f"{k}={v}" for k, v in sorted(rep.checked_by.items()))
            print(f"ok   {name}: {rep.checked} checked ({cats}); unverifiable: {unv}")
            for n in rep.notes:
                print("  note: " + n)
    dt = time.time() - t0
    unv = ", ".join(f"{k}={v}" for k, v in sorted(unv_total.items())) or "none"
    if args.verbose:
        print("checked by category: " + ", ".join(f"{k}={v}" for k, v in sorted(by_cat.items())))
    print(f"walker_oracle: {len(results)} goldens, {total['checked']} facts checked, "
          f"{bad} failed; unverifiable by il_display: {unv}; {dt:.1f}s")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
