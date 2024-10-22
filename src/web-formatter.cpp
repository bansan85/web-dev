#include <clang/Format/Format.h>
#include <clang/Tooling/Tooling.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <string>

std::string format(const std::string &code) {
  clang::format::FormatStyle style = clang::format::getGoogleStyle(
      clang::format::FormatStyle::LanguageKind::LK_Cpp);

  clang::tooling::Range range(0, code.size());

  clang::format::FormattingAttemptStatus status;

  clang::tooling::Replacements replacements =
      clang::format::reformat(style, code, {range}, "<stdin>", &status);

  if (status.FormatComplete) {
    llvm::Expected<std::string> formattedCode =
        clang::tooling::applyAllReplacements(code, replacements);

    if (formattedCode) {
      return formattedCode.get();
    }
  }
  return code;
}

EMSCRIPTEN_BINDINGS(web_formatter) {
  emscripten::function("formatter", &format);
}
