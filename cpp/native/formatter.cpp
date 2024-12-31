#include <clang/Format/Format.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Inclusions/IncludeStyle.h>
#include <llvm/ADT/ArrayRef.h> // IWYU pragma: keep
#include <llvm/Support/Error.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>

#include "formatter.h"

namespace web_formatter {

std::string format(const std::string &code,
                   const clang::format::FormatStyle &format_style) {
  const clang::tooling::Range range(0, code.size());

  clang::format::FormattingAttemptStatus status;

  const clang::tooling::Replacements replacements =
      clang::format::reformat(format_style, code, {range}, "<stdin>", &status);

  if (status.FormatComplete) {
    llvm::Expected<std::string> formatted_code =
        clang::tooling::applyAllReplacements(code, replacements);

    if (formatted_code) {
      return formatted_code.get();
    }
  }
  return code;
}

} // namespace web_formatter
