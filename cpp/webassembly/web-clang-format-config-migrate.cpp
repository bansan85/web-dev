#include "../native/clang-format-config-migrate/Format.h"
#include <boost/pfr/core.hpp>
#include <emscripten/bind.h>
#include <string>

#undef BUILD_UNUSED

#ifdef BUILD_UNUSED

#include "../native/clang-format-config-migrate/10.0.1/Format.h"
#include "../native/clang-format-config-migrate/11.1.0/Format.h"
#include "../native/clang-format-config-migrate/12.0.1/Format.h"
#include "../native/clang-format-config-migrate/13.0.1/Format.h"
#include "../native/clang-format-config-migrate/14.0.6/Format.h"
#include "../native/clang-format-config-migrate/15.0.7/Format.h"
#include "../native/clang-format-config-migrate/16.0.6/Format.h"
#include "../native/clang-format-config-migrate/17.0.6/Format.h"
#include "../native/clang-format-config-migrate/18.1.8/Format.h"
#include "../native/clang-format-config-migrate/19.1.6/Format.h"
#include "../native/clang-format-config-migrate/3.3.0/Format.h"
#include "../native/clang-format-config-migrate/3.4.2/Format.h"
#include "../native/clang-format-config-migrate/3.5.2/Format.h"
#include "../native/clang-format-config-migrate/3.6.2/Format.h"
#include "../native/clang-format-config-migrate/3.7.1/Format.h"
#include "../native/clang-format-config-migrate/3.8.1/Format.h"
#include "../native/clang-format-config-migrate/3.9.1/Format.h"
#include "../native/clang-format-config-migrate/4.0.1/Format.h"
#include "../native/clang-format-config-migrate/5.0.2/Format.h"
#include "../native/clang-format-config-migrate/6.0.1/Format.h"
#include "../native/clang-format-config-migrate/7.1.0/Format.h"
#include "../native/clang-format-config-migrate/8.0.1/Format.h"
#include "../native/clang-format-config-migrate/9.0.1/Format.h"
#include "../native/clang-format-config-migrate/update.h"

namespace web_clang_format_config_migrate {

namespace {

template <class... Ts> struct Overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> Overload(Ts...) -> Overload<Ts...>;

template <typename T> std::enable_if_t<std::is_aggregate_v<T>, T> initialize() {
  T obj{};
  boost::pfr::for_each_field(
      obj, Overload{[](clang_v3_5::FormatStyle::LanguageKind &field) {
                      // LK_None is not allowed by YAML.
                      field = clang_v3_5::FormatStyle::LK_Cpp;
                    },
                    [](clang_v3_6::FormatStyle::LanguageKind &field) {
                      field = clang_v3_6::FormatStyle::LK_Cpp;
                    },
                    [](clang_v3_7::FormatStyle::LanguageKind &field) {
                      field = clang_v3_7::FormatStyle::LK_Cpp;
                    },
                    [](clang_v3_8::FormatStyle::LanguageKind &field) {
                      field = clang_v3_8::FormatStyle::LK_Cpp;
                    },
                    [](clang_v3_9::FormatStyle::LanguageKind &field) {
                      field = clang_v3_9::FormatStyle::LK_Cpp;
                    },
                    [](clang_v4::FormatStyle::LanguageKind &field) {
                      field = clang_v4::FormatStyle::LK_Cpp;
                    },
                    [](clang_v5::FormatStyle::LanguageKind &field) {
                      field = clang_v5::FormatStyle::LK_Cpp;
                    },
                    [](clang_v6::FormatStyle::LanguageKind &field) {
                      field = clang_v6::FormatStyle::LK_Cpp;
                    },
                    [](clang_v7::FormatStyle::LanguageKind &field) {
                      field = clang_v7::FormatStyle::LK_Cpp;
                    },
                    [](clang_v8::FormatStyle::LanguageKind &field) {
                      field = clang_v8::FormatStyle::LK_Cpp;
                    },
                    [](clang_v9::FormatStyle::LanguageKind &field) {
                      field = clang_v9::FormatStyle::LK_Cpp;
                    },
                    [](clang_v10::FormatStyle::LanguageKind &field) {
                      field = clang_v10::FormatStyle::LK_Cpp;
                    },
                    [](clang_v11::FormatStyle::LanguageKind &field) {
                      field = clang_v11::FormatStyle::LK_Cpp;
                    },
                    [](clang_v12::FormatStyle::LanguageKind &field) {
                      field = clang_v12::FormatStyle::LK_Cpp;
                    },
                    [](clang_v13::FormatStyle::LanguageKind &field) {
                      field = clang_v13::FormatStyle::LK_Cpp;
                    },
                    [](clang_v14::FormatStyle::LanguageKind &field) {
                      field = clang_v14::FormatStyle::LK_Cpp;
                    },
                    [](clang_v15::FormatStyle::LanguageKind &field) {
                      field = clang_v15::FormatStyle::LK_Cpp;
                    },
                    [](clang_v16::FormatStyle::LanguageKind &field) {
                      field = clang_v16::FormatStyle::LK_Cpp;
                    },
                    [](clang_v17::FormatStyle::LanguageKind &field) {
                      field = clang_v17::FormatStyle::LK_Cpp;
                    },
                    [](clang_v18::FormatStyle::LanguageKind &field) {
                      field = clang_v18::FormatStyle::LK_Cpp;
                    },
                    [](clang_v19::FormatStyle::LanguageKind &field) {
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
#include "web-clang-format-config-migrate-v10.0.1-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v11.1.0-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v12.0.1-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v13.0.1-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v14.0.6-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v15.0.7-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v16.0.6-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v17.0.6-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v18.1.8-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v19.1.6-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v3.3.0-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v3.4.2-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v3.5.2-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v3.6.2-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v3.7.1-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v3.8.1-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v3.9.1-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v4.0.1-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v5.0.2-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v6.0.1-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v7.1.0-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v8.0.1-binding.cpp.inc" // IWYU pragma: keep
#include "web-clang-format-config-migrate-v9.0.1-binding.cpp.inc" // IWYU pragma: keep
}

} // namespace

} // namespace web_clang_format_config_migrate

#endif

EMSCRIPTEN_BINDINGS(web_clang_format_config_migrate) {

  emscripten::register_vector<std::string>("StringList");

#ifdef BUILD_UNUSED

  emscripten::register_optional<unsigned int>();

  emscripten::register_vector<clang_v3_8::FormatStyle::IncludeCategory>(
      "IncludeCategoryListV3_8");
  emscripten::register_vector<clang_v3_9::FormatStyle::IncludeCategory>(
      "IncludeCategoryListV3_9");
  emscripten::register_vector<clang_v4::FormatStyle::IncludeCategory>(
      "IncludeCategoryListV4");
  emscripten::register_vector<clang_v5::FormatStyle::IncludeCategory>(
      "IncludeCategoryListV5");
  emscripten::register_vector<clang_v6::FormatStyle::IncludeCategory>(
      "IncludeCategoryListV6");
  emscripten::register_vector<clang_v6::FormatStyle::RawStringFormat>(
      "RawStringFormatListV6");
  emscripten::register_vector<clang_v7::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV7");
  emscripten::register_vector<clang_v7::FormatStyle::RawStringFormat>(
      "RawStringFormatListV7");
  emscripten::register_vector<clang_v8::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV8");
  emscripten::register_vector<clang_v8::FormatStyle::RawStringFormat>(
      "RawStringFormatListV8");
  emscripten::register_vector<clang_v9::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV9");
  emscripten::register_vector<clang_v9::FormatStyle::RawStringFormat>(
      "RawStringFormatListV9");
  emscripten::register_vector<clang_v10::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV10");
  emscripten::register_vector<clang_v10::FormatStyle::RawStringFormat>(
      "RawStringFormatListV10");
  emscripten::register_vector<clang_v11::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV11");
  emscripten::register_vector<clang_v11::FormatStyle::RawStringFormat>(
      "RawStringFormatListV11");
  emscripten::register_vector<clang_v12::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV12");
  emscripten::register_vector<clang_v12::FormatStyle::RawStringFormat>(
      "RawStringFormatListV12");
  emscripten::register_vector<clang_v13::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV13");
  emscripten::register_vector<clang_v13::FormatStyle::RawStringFormat>(
      "RawStringFormatListV13");
  emscripten::register_vector<clang_v14::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV14");
  emscripten::register_vector<clang_v14::FormatStyle::RawStringFormat>(
      "RawStringFormatListV14");
  emscripten::register_vector<clang_v15::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV15");
  emscripten::register_vector<clang_v15::FormatStyle::RawStringFormat>(
      "RawStringFormatListV15");
  emscripten::register_vector<clang_v16::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV16");
  emscripten::register_vector<clang_v16::FormatStyle::RawStringFormat>(
      "RawStringFormatListV16");
  emscripten::register_vector<clang_v17::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV17");
  emscripten::register_vector<clang_v17::FormatStyle::RawStringFormat>(
      "RawStringFormatListV17");
  emscripten::register_vector<clang_v18::IncludeStyle::IncludeCategory>(
      "IncludeCategoryListV18");
  emscripten::register_vector<clang_v18::FormatStyle::RawStringFormat>(
      "RawStringFormatListV18");
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

  emscripten::function("upgradeV19",
                       &clang_update_v19::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV19",
                       &clang_update_v19::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV18", +[] {
        return clang_v18::getLLVMStyle(
            clang_v18::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV18", +[] {
        return clang_v18::getGoogleStyle(
            clang_v18::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV18", +[] {
        return clang_v18::getChromiumStyle(
            clang_v18::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV18", &clang_v18::getMozillaStyle);
  emscripten::function("getWebKitStyleV18", &clang_v18::getWebKitStyle);
  emscripten::function("getGNUStyleV18", &clang_v18::getGNUStyle);
  emscripten::function(
      "getMicrosoftStyleV18", +[] {
        return clang_v18::getMicrosoftStyle(
            clang_v18::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getClangFormatStyleV18",
                       &clang_v18::getClangFormatStyle);
  emscripten::function("getNoStyleV18", &clang_v18::getNoStyle);
  emscripten::function("serializeToYamlV18", &clang_v18::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV18", +[](const std::string &yaml) {
        clang_v18::FormatStyle retval;
        retval.Language = clang_v18::FormatStyle::LanguageKind::LK_Cpp;
        retval.InheritsParentConfig = false;
        const std::error_code ec = clang_v18::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV18",
                       &clang_update_v18::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV18",
                       &clang_update_v18::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV17", +[] {
        return clang_v17::getLLVMStyle(
            clang_v17::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV17", +[] {
        return clang_v17::getGoogleStyle(
            clang_v17::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV17", +[] {
        return clang_v17::getChromiumStyle(
            clang_v17::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV17", &clang_v17::getMozillaStyle);
  emscripten::function("getWebKitStyleV17", &clang_v17::getWebKitStyle);
  emscripten::function("getGNUStyleV17", &clang_v17::getGNUStyle);
  emscripten::function(
      "getMicrosoftStyleV17", +[] {
        return clang_v17::getMicrosoftStyle(
            clang_v17::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getNoStyleV17", &clang_v17::getNoStyle);
  emscripten::function("serializeToYamlV17", &clang_v17::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV17", +[](const std::string &yaml) {
        clang_v17::FormatStyle retval;
        retval.Language = clang_v17::FormatStyle::LanguageKind::LK_Cpp;
        retval.InheritsParentConfig = false;
        const std::error_code ec = clang_v17::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV17",
                       &clang_update_v17::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV17",
                       &clang_update_v17::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV16", +[] {
        return clang_v16::getLLVMStyle(
            clang_v16::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV16", +[] {
        return clang_v16::getGoogleStyle(
            clang_v16::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV16", +[] {
        return clang_v16::getChromiumStyle(
            clang_v16::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV16", &clang_v16::getMozillaStyle);
  emscripten::function("getWebKitStyleV16", &clang_v16::getWebKitStyle);
  emscripten::function("getGNUStyleV16", &clang_v16::getGNUStyle);
  emscripten::function(
      "getMicrosoftStyleV16", +[] {
        return clang_v16::getMicrosoftStyle(
            clang_v16::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getNoStyleV16", &clang_v16::getNoStyle);
  emscripten::function("serializeToYamlV16", &clang_v16::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV16", +[](const std::string &yaml) {
        clang_v16::FormatStyle retval;
        retval.Language = clang_v16::FormatStyle::LanguageKind::LK_Cpp;
        retval.InheritsParentConfig = false;
        const std::error_code ec = clang_v16::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV16",
                       &clang_update_v16::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV16",
                       &clang_update_v16::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV15", +[] {
        return clang_v15::getLLVMStyle(
            clang_v15::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV15", +[] {
        return clang_v15::getGoogleStyle(
            clang_v15::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV15", +[] {
        return clang_v15::getChromiumStyle(
            clang_v15::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV15", &clang_v15::getMozillaStyle);
  emscripten::function("getWebKitStyleV15", &clang_v15::getWebKitStyle);
  emscripten::function("getGNUStyleV15", &clang_v15::getGNUStyle);
  emscripten::function(
      "getMicrosoftStyleV15", +[] {
        return clang_v15::getMicrosoftStyle(
            clang_v15::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getNoStyleV15", &clang_v15::getNoStyle);
  emscripten::function("serializeToYamlV15", &clang_v15::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV15", +[](const std::string &yaml) {
        clang_v15::FormatStyle retval;
        retval.Language = clang_v15::FormatStyle::LanguageKind::LK_Cpp;
        retval.InheritsParentConfig = false;
        const std::error_code ec = clang_v15::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV15",
                       &clang_update_v15::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV15",
                       &clang_update_v15::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV14", +[] {
        return clang_v14::getLLVMStyle(
            clang_v14::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV14", +[] {
        return clang_v14::getGoogleStyle(
            clang_v14::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV14", +[] {
        return clang_v14::getChromiumStyle(
            clang_v14::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV14", &clang_v14::getMozillaStyle);
  emscripten::function("getWebKitStyleV14", &clang_v14::getWebKitStyle);
  emscripten::function("getGNUStyleV14", &clang_v14::getGNUStyle);
  emscripten::function(
      "getMicrosoftStyleV14", +[] {
        return clang_v14::getMicrosoftStyle(
            clang_v14::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getNoStyleV14", &clang_v14::getNoStyle);
  emscripten::function("serializeToYamlV14", &clang_v14::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV14", +[](const std::string &yaml) {
        clang_v14::FormatStyle retval;
        retval.Language = clang_v14::FormatStyle::LanguageKind::LK_Cpp;
        retval.InheritsParentConfig = false;
        const std::error_code ec = clang_v14::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV14",
                       &clang_update_v14::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV14",
                       &clang_update_v14::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV13", +[] {
        return clang_v13::getLLVMStyle(
            clang_v13::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV13", +[] {
        return clang_v13::getGoogleStyle(
            clang_v13::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV13", +[] {
        return clang_v13::getChromiumStyle(
            clang_v13::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV13", &clang_v13::getMozillaStyle);
  emscripten::function("getWebKitStyleV13", &clang_v13::getWebKitStyle);
  emscripten::function("getGNUStyleV13", &clang_v13::getGNUStyle);
  emscripten::function(
      "getMicrosoftStyleV13", +[] {
        return clang_v13::getMicrosoftStyle(
            clang_v13::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getNoStyleV13", &clang_v13::getNoStyle);
  emscripten::function("serializeToYamlV13", &clang_v13::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV13", +[](const std::string &yaml) {
        clang_v13::FormatStyle retval;
        retval.Language = clang_v13::FormatStyle::LanguageKind::LK_Cpp;
        retval.InheritsParentConfig = false;
        const std::error_code ec = clang_v13::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV13",
                       &clang_update_v13::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV13",
                       &clang_update_v13::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV12", +[] {
        return clang_v12::getLLVMStyle(
            clang_v12::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV12", +[] {
        return clang_v12::getGoogleStyle(
            clang_v12::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV12", +[] {
        return clang_v12::getChromiumStyle(
            clang_v12::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV12", &clang_v12::getMozillaStyle);
  emscripten::function("getWebKitStyleV12", &clang_v12::getWebKitStyle);
  emscripten::function("getGNUStyleV12", &clang_v12::getGNUStyle);
  emscripten::function(
      "getMicrosoftStyleV12", +[] {
        return clang_v12::getMicrosoftStyle(
            clang_v12::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getNoStyleV12", &clang_v12::getNoStyle);
  emscripten::function("serializeToYamlV12", &clang_v12::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV12", +[](const std::string &yaml) {
        clang_v12::FormatStyle retval;
        retval.Language = clang_v12::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec = clang_v12::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV12",
                       &clang_update_v12::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV12",
                       &clang_update_v12::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV11", +[] {
        return clang_v11::getLLVMStyle(
            clang_v11::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV11", +[] {
        return clang_v11::getGoogleStyle(
            clang_v11::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV11", +[] {
        return clang_v11::getChromiumStyle(
            clang_v11::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV11", &clang_v11::getMozillaStyle);
  emscripten::function("getWebKitStyleV11", &clang_v11::getWebKitStyle);
  emscripten::function("getGNUStyleV11", &clang_v11::getGNUStyle);
  emscripten::function(
      "getMicrosoftStyleV11", +[] {
        return clang_v11::getMicrosoftStyle(
            clang_v11::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getNoStyleV11", &clang_v11::getNoStyle);
  emscripten::function("serializeToYamlV11", &clang_v11::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV11", +[](const std::string &yaml) {
        clang_v11::FormatStyle retval;
        retval.Language = clang_v11::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec = clang_v11::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV11",
                       &clang_update_v11::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV11",
                       &clang_update_v11::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV10", +[] {
        return clang_v10::getLLVMStyle(
            clang_v10::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV10", +[] {
        return clang_v10::getGoogleStyle(
            clang_v10::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV10", +[] {
        return clang_v10::getChromiumStyle(
            clang_v10::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV10", &clang_v10::getMozillaStyle);
  emscripten::function("getWebKitStyleV10", &clang_v10::getWebKitStyle);
  emscripten::function("getGNUStyleV10", &clang_v10::getGNUStyle);
  emscripten::function(
      "getMicrosoftStyleV10", +[] {
        return clang_v10::getMicrosoftStyle(
            clang_v10::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getNoStyleV10", &clang_v10::getNoStyle);
  emscripten::function("serializeToYamlV10", &clang_v10::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV10", +[](const std::string &yaml) {
        clang_v10::FormatStyle retval;
        retval.Language = clang_v10::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec = clang_v10::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV10",
                       &clang_update_v10::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV10",
                       &clang_update_v10::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV9", +[] {
        return clang_v9::getLLVMStyle(
            clang_v9::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getGoogleStyleV9", +[] {
        return clang_v9::getGoogleStyle(
            clang_v9::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV9", +[] {
        return clang_v9::getChromiumStyle(
            clang_v9::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV9", &clang_v9::getMozillaStyle);
  emscripten::function("getWebKitStyleV9", &clang_v9::getWebKitStyle);
  emscripten::function("getGNUStyleV9", &clang_v9::getGNUStyle);
  emscripten::function("getNoStyleV9", &clang_v9::getNoStyle);
  emscripten::function("serializeToYamlV9", &clang_v9::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV9", +[](const std::string &yaml) {
        clang_v9::FormatStyle retval;
        retval.Language = clang_v9::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec = clang_v9::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV9",
                       &clang_update_v9::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV9",
                       &clang_update_v9::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV8", +[] { return clang_v8::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV8", +[] {
        return clang_v8::getGoogleStyle(
            clang_v8::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV8", +[] {
        return clang_v8::getChromiumStyle(
            clang_v8::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV8", &clang_v8::getMozillaStyle);
  emscripten::function("getWebKitStyleV8", &clang_v8::getWebKitStyle);
  emscripten::function("getGNUStyleV8", &clang_v8::getGNUStyle);
  emscripten::function("getNoStyleV8", &clang_v8::getNoStyle);
  emscripten::function("serializeToYamlV8", &clang_v8::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV8", +[](const std::string &yaml) {
        clang_v8::FormatStyle retval;
        retval.Language = clang_v8::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec = clang_v8::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV8",
                       &clang_update_v8::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV8",
                       &clang_update_v8::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV7", +[] { return clang_v7::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV7", +[] {
        return clang_v7::getGoogleStyle(
            clang_v7::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV7", +[] {
        return clang_v7::getChromiumStyle(
            clang_v7::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV7", &clang_v7::getMozillaStyle);
  emscripten::function("getWebKitStyleV7", &clang_v7::getWebKitStyle);
  emscripten::function("getGNUStyleV7", &clang_v7::getGNUStyle);
  emscripten::function("getNoStyleV7", &clang_v7::getNoStyle);
  emscripten::function("serializeToYamlV7", &clang_v7::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV7", +[](const std::string &yaml) {
        clang_v7::FormatStyle retval;
        retval.Language = clang_v7::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec = clang_v7::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV7",
                       &clang_update_v7::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV7",
                       &clang_update_v7::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV6", +[] { return clang_v6::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV6", +[] {
        return clang_v6::getGoogleStyle(
            clang_v6::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV6", +[] {
        return clang_v6::getChromiumStyle(
            clang_v6::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV6", &clang_v6::getMozillaStyle);
  emscripten::function("getWebKitStyleV6", &clang_v6::getWebKitStyle);
  emscripten::function("getGNUStyleV6", &clang_v6::getGNUStyle);
  emscripten::function("getNoStyleV6", &clang_v6::getNoStyle);
  emscripten::function("serializeToYamlV6", &clang_v6::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV6", +[](const std::string &yaml) {
        clang_v6::FormatStyle retval;
        retval.Language = clang_v6::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec = clang_v6::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV6",
                       &clang_update_v6::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV6",
                       &clang_update_v6::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV5", +[] { return clang_v5::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV5", +[] {
        return clang_v5::getGoogleStyle(
            clang_v5::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV5", +[] {
        return clang_v5::getChromiumStyle(
            clang_v5::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV5", &clang_v5::getMozillaStyle);
  emscripten::function("getWebKitStyleV5", &clang_v5::getWebKitStyle);
  emscripten::function("getGNUStyleV5", &clang_v5::getGNUStyle);
  emscripten::function("getNoStyleV5", &clang_v5::getNoStyle);
  emscripten::function("serializeToYamlV5", &clang_v5::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV5", +[](const std::string &yaml) {
        clang_v5::FormatStyle retval;
        retval.Language = clang_v5::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec = clang_v5::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV5",
                       &clang_update_v5::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV5",
                       &clang_update_v5::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV4", +[] { return clang_v4::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV4", +[] {
        return clang_v4::getGoogleStyle(
            clang_v4::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV4", +[] {
        return clang_v4::getChromiumStyle(
            clang_v4::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV4", &clang_v4::getMozillaStyle);
  emscripten::function("getWebKitStyleV4", &clang_v4::getWebKitStyle);
  emscripten::function("getGNUStyleV4", &clang_v4::getGNUStyle);
  emscripten::function("getNoStyleV4", &clang_v4::getNoStyle);
  emscripten::function("serializeToYamlV4", &clang_v4::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV4", +[](const std::string &yaml) {
        clang_v4::FormatStyle retval;
        retval.Language = clang_v4::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec = clang_v4::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV4",
                       &clang_update_v4::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV4",
                       &clang_update_v4::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV3_9", +[] { return clang_v3_9::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV3_9", +[] {
        return clang_v3_9::getGoogleStyle(
            clang_v3_9::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV3_9", +[] {
        return clang_v3_9::getChromiumStyle(
            clang_v3_9::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV3_9", &clang_v3_9::getMozillaStyle);
  emscripten::function("getWebKitStyleV3_9", &clang_v3_9::getWebKitStyle);
  emscripten::function("getGNUStyleV3_9", &clang_v3_9::getGNUStyle);
  emscripten::function("getNoStyleV3_9", &clang_v3_9::getNoStyle);
  emscripten::function("serializeToYamlV3_9", &clang_v3_9::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV3_9", +[](const std::string &yaml) {
        clang_v3_9::FormatStyle retval;
        retval.Language = clang_v3_9::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec =
            clang_v3_9::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV3_9",
                       &clang_update_v3_9::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV3_9",
                       &clang_update_v3_9::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV3_8", +[] { return clang_v3_8::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV3_8", +[] {
        return clang_v3_8::getGoogleStyle(
            clang_v3_8::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV3_8", +[] {
        return clang_v3_8::getChromiumStyle(
            clang_v3_8::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV3_8", &clang_v3_8::getMozillaStyle);
  emscripten::function("getWebKitStyleV3_8", &clang_v3_8::getWebKitStyle);
  emscripten::function("getGNUStyleV3_8", &clang_v3_8::getGNUStyle);
  emscripten::function("getNoStyleV3_8", &clang_v3_8::getNoStyle);
  emscripten::function("serializeToYamlV3_8", &clang_v3_8::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV3_8", +[](const std::string &yaml) {
        clang_v3_8::FormatStyle retval;
        retval.Language = clang_v3_8::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec =
            clang_v3_8::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV3_8",
                       &clang_update_v3_8::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV3_8",
                       &clang_update_v3_8::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV3_7", +[] { return clang_v3_7::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV3_7", +[] {
        return clang_v3_7::getGoogleStyle(
            clang_v3_7::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV3_7", +[] {
        return clang_v3_7::getChromiumStyle(
            clang_v3_7::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV3_7", &clang_v3_7::getMozillaStyle);
  emscripten::function("getWebKitStyleV3_7", &clang_v3_7::getWebKitStyle);
  emscripten::function("getGNUStyleV3_7", &clang_v3_7::getGNUStyle);
  emscripten::function("getNoStyleV3_7", &clang_v3_7::getNoStyle);
  emscripten::function("serializeToYamlV3_7", &clang_v3_7::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV3_7", +[](const std::string &yaml) {
        clang_v3_7::FormatStyle retval;
        retval.Language = clang_v3_7::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec =
            clang_v3_7::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV3_7",
                       &clang_update_v3_7::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV3_7",
                       &clang_update_v3_7::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV3_6", +[] { return clang_v3_6::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV3_6", +[] {
        return clang_v3_6::getGoogleStyle(
            clang_v3_6::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV3_6", +[] {
        return clang_v3_6::getChromiumStyle(
            clang_v3_6::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV3_6", &clang_v3_6::getMozillaStyle);
  emscripten::function("getWebKitStyleV3_6", &clang_v3_6::getWebKitStyle);
  emscripten::function("getGNUStyleV3_6", &clang_v3_6::getGNUStyle);
  emscripten::function("getNoStyleV3_6", &clang_v3_6::getNoStyle);
  emscripten::function("serializeToYamlV3_6", &clang_v3_6::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV3_6", +[](const std::string &yaml) {
        clang_v3_6::FormatStyle retval;
        retval.Language = clang_v3_6::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec =
            clang_v3_6::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV3_6",
                       &clang_update_v3_6::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV3_6",
                       &clang_update_v3_6::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV3_5", +[] { return clang_v3_5::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV3_5", +[] {
        return clang_v3_5::getGoogleStyle(
            clang_v3_5::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function(
      "getChromiumStyleV3_5", +[] {
        return clang_v3_5::getChromiumStyle(
            clang_v3_5::FormatStyle::LanguageKind::LK_Cpp);
      });
  emscripten::function("getMozillaStyleV3_5", &clang_v3_5::getMozillaStyle);
  emscripten::function("getWebKitStyleV3_5", &clang_v3_5::getWebKitStyle);
  emscripten::function("getGNUStyleV3_5", &clang_v3_5::getGNUStyle);
  emscripten::function("getNoStyleV3_5", &clang_v3_5::getNoStyle);
  emscripten::function("serializeToYamlV3_5", &clang_v3_5::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV3_5", +[](const std::string &yaml) {
        clang_v3_5::FormatStyle retval;
        retval.Language = clang_v3_5::FormatStyle::LanguageKind::LK_Cpp;
        const std::error_code ec =
            clang_v3_5::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV3_5",
                       &clang_update_v3_5::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV3_5",
                       &clang_update_v3_5::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV3_4", +[] { return clang_v3_4::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV3_4", +[] { return clang_v3_4::getGoogleStyle(); });
  emscripten::function(
      "getChromiumStyleV3_4", +[] { return clang_v3_4::getChromiumStyle(); });
  emscripten::function("getMozillaStyleV3_4", &clang_v3_4::getMozillaStyle);
  emscripten::function("getWebKitStyleV3_4", &clang_v3_4::getWebKitStyle);
  emscripten::function("serializeToYamlV3_4", &clang_v3_4::configurationAsText);
  emscripten::function(
      "deserializeFromYamlV3_4", +[](const std::string &yaml) {
        clang_v3_4::FormatStyle retval;
        const std::error_code ec =
            clang_v3_4::parseConfiguration(yaml, &retval);
        if (ec) {
          throw std::runtime_error("Failed to parse yaml config file.\n" +
                                   ec.message());
        }
        return retval;
      });

  emscripten::function("upgradeV3_4",
                       &clang_update_v3_4::update<clang_vx::Update::UPGRADE>);
  emscripten::function("downgradeV3_4",
                       &clang_update_v3_4::update<clang_vx::Update::DOWNGRADE>);

  emscripten::function(
      "getLLVMStyleV3_3", +[] { return clang_v3_3::getLLVMStyle(); });
  emscripten::function(
      "getGoogleStyleV3_3", +[] { return clang_v3_3::getGoogleStyle(); });
  emscripten::function(
      "getChromiumStyleV3_3", +[] { return clang_v3_3::getChromiumStyle(); });
  emscripten::function("getMozillaStyleV3_3", &clang_v3_3::getMozillaStyle);

  web_clang_format_config_migrate::registerFormatStyle();

#endif

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
      .value("V19", clang_vx::Version::V19);

  emscripten::register_vector<clang_vx::Version>("VersionList");

  emscripten::function(
      "getCompatibleVersion", +[](const std::string &yaml) {
        return clang_vx::getCompatibleVersion(yaml);
      });
  emscripten::function(
      "getStyleNames", +[](clang_vx::Version version) {
        return clang_vx::getStyleNames(version);
      });
}
