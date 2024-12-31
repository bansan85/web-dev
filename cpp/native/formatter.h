#pragma once

#include <clang/Format/Format.h>
#include <string>

namespace web_formatter {

std::string format(const std::string &code,
                   const clang::format::FormatStyle &format_style);

} // namespace web_formatter
