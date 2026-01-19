#include "../native/clang-format-config-migrate/Format.h"
#include "../native/clang-format-config-migrate/update.h"

#include <boost/pfr/core.hpp>
#include <emscripten/bind.h>
#include <string>

EMSCRIPTEN_BINDINGS(web_clang_format_config_migrate) {

  emscripten::register_vector<std::string>("StringList");

  emscripten::enum_<clang_vx::Version>("Version")
      .value("V3_3", clang_vx::Version::V3_3)
      .value("V3_4", clang_vx::Version::V3_4)
      .value("V3_5", clang_vx::Version::V3_5)
      .value("V3_6", clang_vx::Version::V3_6)
      .value("V3_7", clang_vx::Version::V3_7)
      .value("V3_8", clang_vx::Version::V3_8)
      .value("V3_9", clang_vx::Version::V3_9)
      .value("V4", clang_vx::Version::V4)
      .value("V5", clang_vx::Version::V5)
      .value("V6", clang_vx::Version::V6)
      .value("V7", clang_vx::Version::V7)
      .value("V8", clang_vx::Version::V8)
      .value("V9", clang_vx::Version::V9)
      .value("V10", clang_vx::Version::V10)
      .value("V11", clang_vx::Version::V11)
      .value("V12", clang_vx::Version::V12)
      .value("V13", clang_vx::Version::V13)
      .value("V14", clang_vx::Version::V14)
      .value("V15", clang_vx::Version::V15)
      .value("V16", clang_vx::Version::V16)
      .value("V17", clang_vx::Version::V17)
      .value("V18", clang_vx::Version::V18)
      .value("V19", clang_vx::Version::V19)
      .value("V20", clang_vx::Version::V20)
      .value("V21", clang_vx::Version::V21);

  emscripten::register_vector<clang_vx::Version>("VersionList");

  emscripten::function(
      "getCompatibleVersion", +[](const std::string &yaml) {
        return clang_vx::getCompatibleVersion(yaml);
      });
  emscripten::function(
      "versionEnumToString", +[](clang_vx::Version version) {
        return clang_vx::versionEnumToString(version);
      });
  emscripten::function(
      "versionStringToEnum", +[](const std::string &version) {
        return clang_vx::versionStringToEnum(version);
      });
  emscripten::function(
      "getStyleNames", +[](clang_vx::Version version) {
        return clang_vx::getStyleNames(version);
      });
  emscripten::function(
      "getStyleNamesRange",
      +[](clang_vx::Version vstart, clang_vx::Version vend) {
        return clang_vx::getStyleNamesRange(vstart, vend);
      });
  emscripten::function(
      "migrateTo", +[](clang_vx::Version vstart, clang_vx::Version vend,
                       const std::string &data,
                       const std::string &default_style, bool skip_same_value) {
        return clang_vx::migrateTo(vstart, vend, data, default_style,
                                   skip_same_value);
      });
}
