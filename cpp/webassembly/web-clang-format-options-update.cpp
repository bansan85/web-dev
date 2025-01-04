#include <boost/pfr/core.hpp>
#include <emscripten/bind.h>
#include <string>

#include "../native/clang-format-options-update/19.1.6/Format.h"
#include "../native/clang-format-options-update/19.1.6/IncludeStyle.h"

namespace web_clang_format_options_update {

namespace {

template <class... Ts> struct Overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> Overload(Ts...) -> Overload<Ts...>;

template <typename T> std::enable_if_t<std::is_aggregate_v<T>, T> initialize() {
  T obj{};
  boost::pfr::for_each_field(
      obj, Overload{[](clang_v19::FormatStyle::LanguageKind &field) {
                      // LK_None is not allowed by YAML.
                      field = clang_v19::FormatStyle::LK_Cpp;
                    },
                    [](auto &field) { field = {}; }});
  return obj;
}

template <typename T>
std::enable_if_t<!std::is_aggregate_v<T>, T> initialize() {
  return T{};
}

void registerFormatStyle() {
#include "web-clang-format-options-update-v19.1.6-binding.cpp.inc" // IWYU pragma: keep
}

} // namespace

} // namespace web_clang_format_options_update

EMSCRIPTEN_BINDINGS(web_clang_format_options_update) {
  emscripten::register_vector<std::string>("StringList");
  emscripten::register_optional<unsigned int>();

  emscripten::register_vector<clang_v19::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV19");
  emscripten::register_vector<clang_v19::FormatStyle::RawStringFormat>(
      "RawStringFormatListV19");

  emscripten::function(
      "getLLVMStyleV19", +[] {
        return clang_v19::getLLVMStyle(
            clang_v19::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV19", +[] {
        return clang_v19::getGoogleStyle(
            clang_v19::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV19", +[] {
        return clang_v19::getChromiumStyle(
            clang_v19::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV19", &clang_v19::getMozillaStyle);
  emscripten::function("getWebKitStyleV19", &clang_v19::getWebKitStyle);
  emscripten::function("getGNUStyleV19", &clang_v19::getGNUStyle);
  emscripten::function(
      "getMicrosoftStyleV19", +[] {
        return clang_v19::getMicrosoftStyle(
            clang_v19::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getClangFormatStyleV19",
                       &clang_v19::getClangFormatStyle);
  emscripten::function("getNoStyleV19", &clang_v19::getNoStyle);
  emscripten::function("serializeToYamlV19", &clang_v19::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV19", +[](const std::string &yaml) {
        clang_v19::FormatStyle retval;
        retval.Language = clang_v19::FormatStyle::LanguageKind::LK_Cpp;
        retval.InheritsParentConfig = false;
        const std::error_code ec = clang_v19::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  web_clang_format_options_update::registerFormatStyle();
}
