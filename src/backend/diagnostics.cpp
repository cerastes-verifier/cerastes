// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "diagnostics.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <set>
#include <tuple>

#include "render.h"

namespace cerastes {
namespace {

struct Entry
{
    Diag diag;
    DiagPos pos;
    std::string message;
    std::string context;
};

std::vector<Entry>& entries()
{
    static std::vector<Entry> v;
    return v;
}
using SeenKey = std::tuple<int, std::string, unsigned long, unsigned long, std::string>;
std::set<SeenKey>& seen()
{
    static std::set<SeenKey> s;
    return s;
}

const char* template_of(Diag d)
{
    switch (d)
    {
        // The formatter lays an indented #define's body out from column 0, so the macro's lines stay as written.
        // clang-format off
        #define CERASTES_DIAG(tag, severity, message) \
            case Diag::tag:                           \
                return message;
        // clang-format on
        #include "diagnostics.def"
        #undef CERASTES_DIAG
    }
    return "";
}

std::string substitute(const char* tmpl, const std::vector<std::string>& args)
{
    std::string out;
    size_t next {0U};
    for (const char* p {tmpl}; *p; ++p)
    {
        if (p[0] == '%' && p[1] == 's')
        {
            out += next < args.size() ? args[next] : "?";
            ++next;
            ++p;
        }
        else
        {
            out += *p;
        }
    }
    return out;
}

// EDG's own layout (error.c format_output_line): lines wrap at
// MAX_ERROR_OUTPUT_LINE_LENGTH (host_envir.h), at a blank when possible, and
// continuation lines are indented by INDENT_AMOUNT.  --brief_diagnostics and
// --no_wrap_diagnostics switch it off.
std::string wrap(const std::string& text, size_t first_indent, size_t continuation_indent)
{
    const bool wrapping {!do_not_wrap_diagnostics && !brief_diagnostics};
    std::string out;
    size_t indent {first_indent};
    size_t start {0U};
    for (;;)
    {
        const size_t usable {MAX_ERROR_OUTPUT_LINE_LENGTH - indent};
        out.append(indent, ' ');
        const size_t length {text.size() - start};
        if (wrapping && length > usable)
        {
            size_t seg {usable};
            size_t cur {start + seg - 1U};
            if (text[cur + 1U] == ' ')
            {
                ++cur;
            }
            while (cur > start && text[cur] != ' ')
            {
                --cur;
            }
            if (text[cur] == ' ')
            {
                seg = cur - start;
            }
            out.append(text, start, seg);
            out += '\n';
            if (text[cur] == ' ')
            {
                ++seg;
            }
            start += seg;
            indent = continuation_indent;
        }
        else
        {
            out.append(text, start, std::string::npos);
            out += '\n';
            break;
        }
    }
    return out;
}

// Line `line` (1-based) of `path`, without its line terminator; empty and
// `ok` false when the file or line cannot be read.
std::string source_line(const std::string& path, unsigned long line, bool& ok)
{
    ok = false;
    if (path.empty() || line == 0U)
    {
        return "";
    }
    std::ifstream in(path);
    std::string s;
    for (unsigned long i {1U}; std::getline(in, s); ++i)
    {
        if (i == line)
        {
            if (!s.empty() && s.back() == '\r')
            {
                s.pop_back();
            }
            ok = true;
            return s;
        }
    }
    return "";
}

std::string render(const Entry& e)
{
    const char* tag {tag_of(e.diag)};
    std::string head;
    if (e.pos.known)
    {
        head = "\"" + e.pos.file + "\", line " + std::to_string(e.pos.line);
        if (brief_diagnostics)
        {
            head += " (col. " + std::to_string(e.pos.col) + ")";
        }
        head += ": ";
    }
    head += "error";
    if (display_error_number)
    {
        head += std::string(" #cerastes-") + tag;
    }
    head += ": " + e.message;
    std::string out {wrap(head, 0U, 10U)};
    if (!brief_diagnostics && e.pos.known)
    {
        bool ok {false};
        std::string text {source_line(e.pos.full_name, e.pos.line, ok)};
        if (ok)
        {
            out += "  " + text + "\n";
            if (e.pos.col != 0U)
            {
                std::string caret {"  "};
                for (unsigned long i {0U}; i + 1U < e.pos.col && i < text.size(); ++i)
                {
                    caret += text[i] == '\t' ? '\t' : ' ';
                }
                for (unsigned long i {text.size()}; i + 1U < e.pos.col; ++i)
                {
                    caret += ' ';
                }
                out += caret + "^\n";
            }
            else
            {
                out += "\n";
            }
        }
    }
    if (!e.context.empty())
    {
        out += wrap(e.context, 10U, 20U);
    }
    if (!brief_diagnostics)
    {
        out += "\n";
    }
    return out;
}

} // namespace

const char* tag_of(Diag d)
{
    switch (d)
    {
        // The formatter lays an indented #define's body out from column 0, so the macro's lines stay as written.
        // clang-format off
        #define CERASTES_DIAG(tag, severity, message) \
            case Diag::tag:                           \
                return #tag;
        // clang-format on
        #include "diagnostics.def"
        #undef CERASTES_DIAG
    }
    return "?";
}

DiagPos diag_position(const a_source_position& p)
{
    DiagPos r;
    a_seq_number seq {p.seq};
    a_column_number col {p.column};
    // The original position of macro-argument text counts when it lies in a user or a library file (decision 0046 item 3), not in a system one.
    if (p.orig_seq != 0U && !seq_is_in_system_file(p.orig_seq))
    {
        seq = p.orig_seq;
        col = p.orig_column;
    }
    if (seq == 0U)
    {
        return r;
    }
    r.known = true;
    r.seq = seq;
    r.col = col;
    r.in_primary = !seq_is_in_include_file(seq);
    a_const_char* file {nullptr};
    a_const_char* full {nullptr};
    a_line_number line {0U};
    a_boolean at_end {FALSE};
    conv_seq_to_file_and_line(seq, &file, &full, &line, &at_end);
    r.line = line;
    r.file = file ? file : "";
    r.full_name = full ? full : "";
    return r;
}

void report_at(Diag d, const DiagPos& at, const std::vector<std::string>& args, const std::string& context)
{
    Entry e {d, at, substitute(template_of(d), args), context};
    if (!seen().insert({static_cast<int>(d), e.pos.file, e.pos.line, e.pos.col, e.message}).second)
    {
        return;
    }
    entries().push_back(std::move(e));
}

void report(Diag d, const a_source_position& at, const std::vector<std::string>& args, const std::string& context)
{
    report_at(d, diag_position(at), args, context);
}

size_t reported_count()
{
    return entries().size();
}

void flush_diagnostics()
{
    std::vector<Entry>& v {entries()};
    if (v.empty())
    {
        return;
    }
    std::stable_sort(v.begin(), v.end(),
                     [](const Entry& a, const Entry& b)
                     {
                         return std::tie(a.pos.seq, a.pos.line, a.pos.col, a.message) <
                                std::tie(b.pos.seq, b.pos.line, b.pos.col, b.message);
                     });
    // EDG's own error stream (stderr unless --error_output redirected it), so that the two are in order in one place.
    FILE* out {f_error != nullptr ? f_error : stderr};
    for (const Entry& e : v)
    {
        std::fputs(render(e).c_str(), out);
    }
    std::fflush(out);
    diagnostic_counters.total.errors += v.size();
    v.clear();
    seen().clear();
}

} // namespace cerastes
