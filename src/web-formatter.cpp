#include <clang/Format/Format.h>
#include <clang/Tooling/Tooling.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <string>

namespace web_demangler {

std::string Format(const std::string &code,
                   const clang::format::FormatStyle &format_style) {
  clang::tooling::Range range(0, code.size());

  clang::format::FormattingAttemptStatus status;

  clang::tooling::Replacements replacements =
      clang::format::reformat(format_style, code, {range}, "<stdin>", &status);

  if (status.FormatComplete) {
    llvm::Expected<std::string> formattedCode =
        clang::tooling::applyAllReplacements(code, replacements);

    if (formattedCode) {
      return formattedCode.get();
    }
  }
  return code;
}

void RegisterFormatStyle() {
#include "web-formatter-binding.cpp.inc"
}

} // namespace web_demangler

EMSCRIPTEN_BINDINGS(web_formatter) {
  emscripten::function("formatter", &web_demangler::Format);
  emscripten::register_vector<std::string>("StringList");
  emscripten::register_vector<clang::tooling::IncludeStyle::IncludeCategory>(
      "IncludeCategoryList");
  emscripten::register_vector<clang::format::FormatStyle::RawStringFormat>(
      "RawStringFormatList");
  emscripten::register_optional<unsigned int>();

  emscripten::function(
      "getLLVMStyle", +[] {
        return clang::format::getLLVMStyle(
            clang::format::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyle", +[] {
        return clang::format::getGoogleStyle(
            clang::format::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyle", +[] {
        return clang::format::getChromiumStyle(
            clang::format::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyle", &clang::format::getMozillaStyle);
  emscripten::function("getWebKitStyle", &clang::format::getWebKitStyle);
  emscripten::function("getGNUStyle", &clang::format::getGNUStyle);
  emscripten::function(
      "getGoogleStyle", +[] {
        return clang::format::getMicrosoftStyle(
            clang::format::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getClangFormatStyle",
                       &clang::format::getClangFormatStyle);
  emscripten::function("getNoStyle", &clang::format::getNoStyle);

  web_demangler::RegisterFormatStyle();
}
