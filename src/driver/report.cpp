// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "report.h"

#include <algorithm>
#include <cstdlib>
#include <set>
#include <sstream>
#include <tuple>

namespace cerastes {
namespace {

std::vector<std::string> split_tabs(const std::string& line)
{
    std::vector<std::string> out;
    size_t start {0U};
    for (;;)
    {
        size_t t {line.find('\t', start)};
        if (t == std::string::npos)
        {
            out.push_back(line.substr(start));
            return out;
        }
        out.push_back(line.substr(start, t - start));
        start = t + 1U;
    }
}

bool parse_int(const std::string& s, int* out)
{
    if (s.empty())
    {
        return false;
    }
    char* end {nullptr};
    long v {std::strtol(s.c_str(), &end, 10)};
    if (*end != '\0' || v < 0 || v > 100000000L)
    {
        return false;
    }
    *out = static_cast<int>(v);
    return true;
}

} // namespace

std::vector<MapRow> parse_position_map(const std::string& text, std::string* problem)
{
    std::vector<MapRow> rows;
    std::istringstream in(text);
    std::string line;
    int number {0};
    while (std::getline(in, line))
    {
        ++number;
        if (line.empty())
        {
            continue;
        }
        std::vector<std::string> f {split_tabs(line)};
        MapRow r;
        if ((f.size() != 7U && f.size() != 8U) || !parse_int(f[0U], &r.vpr_line) || !parse_int(f[4U], &r.line) ||
            !parse_int(f[5U], &r.column))
        {
            if (problem)
            {
                *problem = "the position map line " + std::to_string(number) + " is malformed";
            }
            return rows;
        }
        r.viper_id = f[1U];
        const size_t open {r.viper_id.find('[')};
        if (open != std::string::npos)
        {
            // decision 0059 item 7: `assert.failed[insufficient.permission]`, a marked range row; any other bracket is refused
            const std::string base {r.viper_id.substr(0U, open)};
            const std::string reason {r.viper_id.substr(open + 1U)};
            if (base != "assert.failed" || reason != "insufficient.permission]")
            {
                if (problem)
                {
                    *problem = "the position map line " + std::to_string(number) + " has a malformed error id";
                }
                return rows;
            }
            r.viper_id = base;
            r.expected_reason = reason.substr(0U, reason.size() - 1U);
        }
        r.category = f[2U];
        r.file = f[3U];
        r.message = f[6U];
        if (f.size() == 8U)
        {
            // `<file>:<line>:<text>`: the file may contain no colon (paths are repository-relative or absolute
            // without one); the text may.
            const std::string& n {f[7U]};
            size_t a {n.find(':')};
            size_t b {a == std::string::npos ? a : n.find(':', a + 1U)};
            if (b == std::string::npos || !parse_int(n.substr(a + 1U, b - a - 1U), &r.note_line))
            {
                if (problem)
                {
                    *problem = "the position map line " + std::to_string(number) + " has a malformed note";
                }
                return rows;
            }
            r.has_note = true;
            r.note_file = n.substr(0U, a);
            r.note_text = n.substr(b + 1U);
        }
        rows.push_back(std::move(r));
    }
    return rows;
}

MappedVerdict map_errors(const SiliconVerdict& verdict, const std::vector<MapRow>& map, const std::string& vpr_name)
{
    MappedVerdict out;
    std::set<std::string> failures;
    if (!verdict.problem.empty())
    {
        failures.insert(verdict.problem);
    }
    for (const SiliconError& e : verdict.errors)
    {
        if (e.viper_id == "unknown" || e.reason == "unknown")
        {
            failures.insert("Silicon reported an error that is not a known kind: " + e.message);
            continue;
        }
        if (e.reason == "timeout")
        {
            failures.insert("the solver timed out: " + e.message);
            continue;
        }
        if (e.file != vpr_name || e.line <= 0)
        {
            failures.insert("Silicon reported an error without a position in the program: " + e.message);
            continue;
        }
        std::vector<const MapRow*> hits;
        for (const MapRow& r : map)
        {
            if (r.vpr_line == e.line && r.viper_id == e.viper_id)
            {
                hits.push_back(&r);
            }
        }
        if (hits.empty())
        {
            failures.insert("Silicon reported " + e.viper_id + " at line " + std::to_string(e.line) +
                            " of the program, which the position map does not explain: " + e.message);
            continue;
        }
        const MapRow* first {hits.front()};
        bool same {true};
        for (const MapRow* h : hits)
        {
            if (h->category != first->category || h->file != first->file || h->line != first->line ||
                h->message != first->message || h->expected_reason != first->expected_reason)
            {
                same = false;
            }
        }
        if (!same)
        {
            failures.insert("the position map has several different entries for " + e.viper_id + " at line " +
                            std::to_string(e.line) + " of the program");
            continue;
        }
        if (first->category == "internal")
        {
            // Only a defect of the encoder can fail one of these checks (0011 item 11).
            failures.insert(first->file + ":" + std::to_string(first->line) + ": " + first->message +
                            " (cerastes bug: " + e.message + ")");
            continue;
        }
        // The checks the encoder emits are assertions, preconditions, postconditions and invariants: each can fail only
        // because an assertion does not hold.  Any other reason (a division by zero Viper found, insufficient
        // permission, ...) is something the encoder did not mean to say (decision 0011 item 11).
        // A marked range row (decision 0059 item 7) is the exception: it is a permission assertion, its failure is the missing permission, and a failed
        // assertion on it is something the encoder did not mean to say; it accepts exactly its expected reason.
        const bool termination {e.viper_id == "termination.failed"};
        const bool marked {!first->expected_reason.empty()};
        const bool reason_ok {marked
                                  ? e.reason == first->expected_reason
                                  : (termination ? (e.reason == "tuple.false" || e.reason == "tuple.condition.false" ||
                                                    e.reason == "termination.condition.false")
                                                 : e.reason == "assertion.false")};
        if (!reason_ok)
        {
            failures.insert("Silicon reported " + e.viper_id + " at line " + std::to_string(e.line) +
                            " of the program for the reason \"" + e.reason + "\", not " +
                            (marked ? "the missing permission \"" + first->expected_reason + "\" that the row checks"
                                    : (termination ? "a termination measure that fails" : "a failed assertion")) +
                            ": " + e.message);
            continue;
        }
        UserError u;
        u.file = first->file;
        u.line = first->line;
        u.category = first->category;
        u.message = first->message;
        u.has_note = first->has_note;
        u.note_file = first->note_file;
        u.note_line = first->note_line;
        u.note_text = first->note_text;
        out.errors.push_back(std::move(u));
    }
    std::sort(out.errors.begin(), out.errors.end(),
              [](const UserError& a, const UserError& b)
              {
                  return std::tie(a.file, a.line, a.category, a.message, a.note_file, a.note_line, a.note_text) <
                         std::tie(b.file, b.line, b.category, b.message, b.note_file, b.note_line, b.note_text);
              });
    out.errors.erase(std::unique(out.errors.begin(), out.errors.end(),
                                 [](const UserError& a, const UserError& b)
                                 {
                                     return render_error(a) == render_error(b);
                                 }),
                     out.errors.end());
    out.failures.assign(failures.begin(), failures.end());
    return out;
}

std::string render_error(const UserError& e)
{
    std::string s {e.file + ":" + std::to_string(e.line) + ": error: " + e.category + ": " + e.message + "\n"};
    if (e.has_note)
    {
        s += e.note_file + ":" + std::to_string(e.note_line) + ": note: " + e.note_text + "\n";
    }
    return s;
}

} // namespace cerastes
