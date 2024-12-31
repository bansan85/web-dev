#include <boost/pfr/core.hpp>
#include <clang/Format/Format.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Inclusions/IncludeStyle.h>
#include <emscripten/bind.h>
#include <llvm/ADT/ArrayRef.h> // IWYU pragma: keep
#include <llvm/Support/Error.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>

#include "../native/formatter.h"

namespace web_formatter {

namespace {

template <typename T> std::enable_if_t<std::is_aggregate_v<T>, T> initialize() {
  T obj{};
  boost::pfr::for_each_field(obj, [](auto &field) {
    if constexpr (std::is_same_v<decltype(field),
                                 clang::format::FormatStyle::LanguageKind &>) {
      // LK_None is not allowed by YAML.
      field = clang::format::FormatStyle::LK_Cpp;
    } else {
      field = {};
    }
  });
  return obj;
}

template <typename T>
std::enable_if_t<!std::is_aggregate_v<T>, T> initialize() {
  return T{};
}

void registerFormatStyle() {
#include "web-formatter-binding.cpp.inc" // IWYU pragma: keep
}

} // namespace

} // namespace web_formatter

EMSCRIPTEN_BINDINGS(web_formatter) {
  emscripten::function("formatter", &web_formatter::format);
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
      "getMicrosoftStyle", +[] {
        return clang::format::getMicrosoftStyle(
            clang::format::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getClangFormatStyle",
                       &clang::format::getClangFormatStyle);
  emscripten::function("getNoStyle", &clang::format::getNoStyle);
  emscripten::function("serializeToYaml", &clang::format::configurationAsText);
  emscripten::function(
      "deserializeFromYaml", +[](const std::string &yaml) {
        clang::format::FormatStyle retval;
        retval.Language = clang::format::FormatStyle::LanguageKind::LK_Cpp;
        retval.InheritsParentConfig = false;
        const std::error_code ec =
            clang::format::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  web_formatter::registerFormatStyle();
}
