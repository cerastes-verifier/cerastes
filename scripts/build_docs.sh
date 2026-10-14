#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Build the user documentation (doc/, Antora) into the playbook's output
# directory (build/doc for the default playbook).  Fails on any Antora warning
# (broken xref, include or attribute), and when the vendored UI bundle is not
# the pinned one.  Needs doc/node_modules; run scripts/bootstrap.sh first.
# Runs on macOS and Linux (CI).
#
# Usage: scripts/build_docs.sh [PLAYBOOK]
#   PLAYBOOK  an Antora playbook, absolute or relative to the repository root;
#             default doc/local-playbook.yml.  Works from any current directory.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DOC="$ROOT/doc"

# The vendored default Antora UI bundle (doc/ui/README.md has its provenance).
# Change the zip and this value together.
UI_BUNDLE="$DOC/ui/antora-ui-default.zip"
UI_BUNDLE_SHA256=e0ac81aad26961a9cd3cf9bce692a598eff4a1bd2822ce73c9cdc13a15a1e70d

die() { printf 'build_docs.sh: error: %s\n' "$*" >&2; exit 1; }

case "${1:-}" in
  -h|--help) sed -n '5,14p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
esac
[[ $# -le 1 ]] || die "usage: scripts/build_docs.sh [PLAYBOOK]"

PLAYBOOK="${1:-doc/local-playbook.yml}"
[[ "$PLAYBOOK" = /* ]] || PLAYBOOK="$ROOT/$PLAYBOOK"
[[ -f "$PLAYBOOK" ]] || die "playbook not found: $PLAYBOOK"

[[ -d "$DOC/node_modules" ]] || die "doc/node_modules is missing; run scripts/bootstrap.sh"
command -v node >/dev/null || die "node is missing; run scripts/bootstrap.sh"

if command -v sha256sum >/dev/null; then
  actual="$(sha256sum "$UI_BUNDLE" 2>/dev/null | awk '{print $1}')"
else
  actual="$(shasum -a 256 "$UI_BUNDLE" 2>/dev/null | awk '{print $1}')"
fi
[[ -n "$actual" ]] || die "vendored UI bundle not found: ${UI_BUNDLE#"$ROOT"/}"
[[ "$actual" == "$UI_BUNDLE_SHA256" ]] \
  || die "${UI_BUNDLE#"$ROOT"/} has SHA-256 $actual, expected $UI_BUNDLE_SHA256 (doc/ui/README.md)"

# Antora resolves a playbook path that starts with ./ (such as the UI bundle)
# against the playbook's directory, but any other relative path (content url,
# output dir, cache dir) against the current directory, so run from the
# playbook's directory.  The extensions it names are found through
# NODE_PATH even when the playbook lives outside the repository (the harness
# self-test uses one).
cd "$(dirname "$PLAYBOOK")"
export NODE_PATH="$DOC/node_modules${NODE_PATH:+:$NODE_PATH}"
printf '==> antora %s\n' "${PLAYBOOK#"$ROOT"/}"
# --ui-bundle-url overrides the playbook's ui.bundle.url, so the UI is always
# the vendored zip whose SHA-256 was checked above, whatever the playbook says
# (a tampered zip or the moving upstream URL cannot be used by mistake).
"$DOC/node_modules/.bin/antora" --log-failure-level=warn --ui-bundle-url "$UI_BUNDLE" "$PLAYBOOK"
