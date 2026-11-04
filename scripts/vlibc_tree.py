#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Which vlibc tree is cerastes built from?  (decision 0046 item 1; modelled on edg_tree.py)

vlibc is the C library with cerastes contracts (decision 0030).  third_party/vlibc
is a submodule of https://github.com/cerastes-verifier/vlibc.git and `check`
requires its gitlink to be contained in `refs/remotes/origin/main` (pushed) or,
failing that, in `refs/remotes/local/main` (a remote named `local`, the working
clone ~/Documents/cerastes/vlibc, not yet pushed: the report line says NOT YET
PUSHED and a warning names the push).

Subcommands (all print to stdout; diagnostics go to stderr):

  resolve    print `pinned <abs path>` or `override <abs realpath>`.  The override
             is the environment variable CERASTES_VLIBC_SRC (unset or empty:
             pinned, ROOT/third_party/vlibc).  Exit 2 with a message on stderr
             for a bad override: a relative path, missing, no include/vlibc, not
             the root of a git work tree, or equal to / inside
             ROOT/third_party/vlibc.
  check      the guards of decision 0046 item 1.  Prints one line
             `vlibc tree: ...` and exits 0, or prints `error: ...` lines and
             exits 1 (exit 2 for a relative CERASTES_VLIBC_SRC).  The pinned
             submodule is checked whatever the override says; an override is
             refused on branch main and on a detached HEAD, as CERASTES_EDG_SRC is.
  describe   print the same report line (exit 2 for a bad override).

Global option `--root DIR` (before the subcommand) points the script at another
repository (self-tests).  Python >= 3.9, standard library only.
"""

from __future__ import annotations

import argparse
import os
import sys
from typing import List, Optional, Tuple

import edg_tree
from edg_tree import RelativeOverride, TreeError, git, git_out, short

DEFAULT_ROOT = edg_tree.DEFAULT_ROOT
VLIBC_URL = "https://github.com/cerastes-verifier/vlibc.git"
PUSHED_REF = "refs/remotes/origin/main"
LOCAL_REF = "refs/remotes/local/main"   # the remote `local`: ~/Documents/cerastes/vlibc
SUBMODULE = "third_party/vlibc"
ENV_VAR = "CERASTES_VLIBC_SRC"


def pinned_path(root: str) -> str:
    return os.path.join(os.path.abspath(root), SUBMODULE)


def resolve(root: str = DEFAULT_ROOT, env: Optional[dict] = None) -> Tuple[str, str]:
    """(kind, path): ("pinned", ROOT/third_party/vlibc) or ("override", realpath).
    Raises TreeError for a bad override."""
    env = os.environ if env is None else env
    value = env.get(ENV_VAR, "")
    if not value:
        return "pinned", pinned_path(root)
    if not value.startswith("/"):
        raise RelativeOverride("%s=%s is a relative path; use an absolute path (decision 0046)"
                               % (ENV_VAR, value))
    if not os.path.isdir(value):
        raise TreeError("%s=%s is not a directory" % (ENV_VAR, value))
    real = os.path.realpath(value)
    if not os.path.isdir(os.path.join(real, "include", "vlibc")):
        raise TreeError("%s=%s has no include/vlibc; it must be the root of a vlibc checkout"
                        % (ENV_VAR, value))
    if edg_tree.own_toplevel(real) is None:
        raise TreeError("%s=%s is not the root of a git work tree" % (ENV_VAR, value))
    if edg_tree.is_inside(real, pinned_path(root)):
        raise TreeError("%s=%s is, or is inside, third_party/vlibc; leave the variable unset "
                        "to build the pin (decision 0046)" % (ENV_VAR, value))
    return "override", real


def gitlink(root: str) -> str:
    return git_out(["rev-parse", ":" + SUBMODULE], os.path.abspath(root))


def gitmodules_url(root: str) -> Optional[str]:
    gm = os.path.join(root, ".gitmodules")
    if not os.path.isfile(gm):
        return None
    proc = git(["config", "-f", gm, "--get-regexp", r"^submodule\..*\.path$"], root, check=False)
    for line in proc.stdout.decode(errors="replace").splitlines():
        key, _, path = line.partition(" ")
        if path.strip() == SUBMODULE:
            name = key[len("submodule."):-len(".path")]
            url = git(["config", "-f", gm, "--get", "submodule.%s.url" % name], root, check=False)
            return url.stdout.decode(errors="replace").strip() or None
    return None


def pin_state(root: str) -> str:
    """Where the gitlink of third_party/vlibc is: "pushed" (contained in
    origin/main), "local" (only in local/main) or "none"."""
    try:
        link = gitlink(root)
    except TreeError:
        return "none"
    sub = pinned_path(root)
    for ref, state in ((PUSHED_REF, "pushed"), (LOCAL_REF, "local")):
        proc = git(["merge-base", "--is-ancestor", link, ref], sub, check=False)
        if proc.returncode == 0:
            return state
    return "none"


NOT_PUSHED_WARNING = ("the pin %s is contained only in the `local` remote's main: the owner must "
                      "push vlibc's main before cerastes main (decision 0046)")


def guard_pin(root: str) -> List[str]:
    """Decision 0046 item 1: third_party/vlibc is clean, at its gitlink, with the
    vlibc URL, and the gitlink is a commit of vlibc's main (pushed, or at least in
    the local clone).  Always checked."""
    errors: List[str] = []
    sub = pinned_path(root)
    if not os.path.isdir(sub) or edg_tree.own_toplevel(sub) is None:
        return ["third_party/vlibc is not an initialized submodule; run scripts/bootstrap.sh"]
    status = git(["status", "--porcelain", "--untracked-files=all"], sub).stdout.decode(
        errors="replace").splitlines()
    if status:
        errors.append("third_party/vlibc is not clean (%d path%s changed, first: %s); vlibc changes "
                      "belong in its own repository and CERASTES_VLIBC_SRC (decision 0046)"
                      % (len(status), "" if len(status) == 1 else "s", status[0]))
    try:
        link = gitlink(root)
    except TreeError as exc:
        errors.append(str(exc))
        link = ""
    head = git_out(["rev-parse", "HEAD"], sub)
    if link and head != link:
        errors.append("third_party/vlibc is at %s but the index records %s for it"
                      % (short(head), short(link)))
    if link and pin_state(root) == "none":
        errors.append("the gitlink %s of third_party/vlibc is not contained in origin/main or in "
                      "the `local` remote's main: the pin is a commit of vlibc's main (decision "
                      "0046); run `git -C third_party/vlibc fetch origin` (the pushed branch) or "
                      "`git -C third_party/vlibc fetch local` (the working clone, added with "
                      "`git -C third_party/vlibc remote add local <clone>`)" % short(link))
    url = gitmodules_url(root)
    if url != VLIBC_URL:
        errors.append(".gitmodules URL of third_party/vlibc is %s, expected %s"
                      % (url or "<missing>", VLIBC_URL))
    origin = git(["config", "--get", "remote.origin.url"], sub, check=False).stdout.decode(
        errors="replace").strip()
    if origin != VLIBC_URL:
        errors.append("remote.origin.url of third_party/vlibc is %s, expected %s"
                      % (origin or "<missing>", VLIBC_URL))
    return errors


def report(root: str, kind: str, tree: str) -> Tuple[str, List[str]]:
    """(report line, warnings)."""
    warnings: List[str] = []
    head = short(git_out(["rev-parse", "HEAD"], tree))
    modified = edg_tree.modified_paths(tree)
    untracked = edg_tree.untracked_paths(tree)
    if kind == "pinned":
        local = pin_state(root) == "local"
        if local:
            warnings.append(NOT_PUSHED_WARNING % head)
        return ("vlibc tree: third_party/vlibc @ %s (gitlink, %s%s)"
                % (head, "DIRTY" if modified or untracked else "clean",
                   ", NOT YET PUSHED to origin/main" if local else "")), warnings
    try:
        link = gitlink(root)
    except TreeError:
        link = ""
    ahead = None
    if link:
        ok = git(["merge-base", "--is-ancestor", link, "HEAD"], tree, check=False)
        if ok.returncode == 0:
            ahead = git_out(["rev-list", "--count", "%s..HEAD" % link], tree)
        else:
            warnings.append("the pin %s is not an ancestor of HEAD in %s" % (short(link), tree))
    where = ("%s commits on top of pin %s" % (ahead, short(link)) if ahead is not None
             else "pin %s is not an ancestor" % (short(link) if link else "<unknown>"))
    return ("vlibc tree: OVERRIDE %s @ %s (branch %s, %s, %d modified, %d untracked)"
            % (tree, head, edg_tree.branch_of(tree), where, len(modified), len(untracked))), warnings


def check(root: str = DEFAULT_ROOT, env: Optional[dict] = None) -> Tuple[int, List[str], List[str]]:
    """(exit status, stdout lines, stderr lines)."""
    root = os.path.abspath(root)
    out: List[str] = []
    err: List[str] = []
    errors = guard_pin(root)
    kind, tree = "pinned", pinned_path(root)
    try:
        kind, tree = resolve(root, env)
    except RelativeOverride as exc:
        return 2, out, ["error: %s" % exc]
    except TreeError as exc:
        errors.append(str(exc))
        kind = "bad"
    if kind == "override":
        proc = git(["symbolic-ref", "--short", "-q", "HEAD"], root, check=False)
        branch = proc.stdout.decode(errors="replace").strip() if proc.returncode == 0 else ""
        if branch == "main":
            errors.append("CERASTES_VLIBC_SRC is set on branch main; check.sh on main runs against "
                          "the pin (decision 0046)")
        elif not branch:
            errors.append("CERASTES_VLIBC_SRC is set with a detached HEAD; check.sh needs a "
                          "named topic branch other than main (decision 0046)")
    if errors:
        err.extend("error: " + e for e in errors)
        return 1, out, err
    line, warnings = report(root, kind, tree)
    err.extend("warning: " + w for w in warnings)
    out.append(line)
    return 0, out, err


def describe(root: str = DEFAULT_ROOT, env: Optional[dict] = None) -> Tuple[int, List[str], List[str]]:
    root = os.path.abspath(root)
    try:
        kind, tree = resolve(root, env)
    except TreeError as exc:
        return 2, [], ["error: %s" % exc]
    line, warnings = report(root, kind, tree)
    return 0, [line], ["warning: " + w for w in warnings]


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--root", default=DEFAULT_ROOT, help="repository root (self-tests)")
    sub = parser.add_subparsers(dest="cmd", required=True)
    sub.add_parser("resolve")
    sub.add_parser("check")
    sub.add_parser("describe")
    args = parser.parse_args(argv)
    try:
        if args.cmd == "resolve":
            kind, tree = resolve(args.root)
            print("%s %s" % (kind, tree))
            return 0
        if args.cmd == "check":
            return edg_tree.emit(*check(args.root))
        return edg_tree.emit(*describe(args.root))
    except TreeError as exc:
        print("error: %s" % exc, file=sys.stderr)
        return 2 if args.cmd in ("resolve", "describe") or isinstance(exc, RelativeOverride) else 1


if __name__ == "__main__":
    sys.exit(main())
