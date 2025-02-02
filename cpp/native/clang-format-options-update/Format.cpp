#include "Format.h"
#include "10.0.1/Format.h"
#include "11.1.0/Format.h"
#include "12.0.1/Format.h"
#include "13.0.1/Format.h"
#include "14.0.6/Format.h"
#include "15.0.7/Format.h"
#include "16.0.6/Format.h"
#include "17.0.6/Format.h"
#include "18.1.8/Format.h"
#include "19.1.6/Format.h"
#include "3.3.0/Format.h"
#include "3.4.2/Format.h"
#include "3.5.2/Format.h"
#include "3.6.2/Format.h"
#include "3.7.1/Format.h"
#include "3.8.1/Format.h"
#include "3.9.1/Format.h"
#include "4.0.1/Format.h"
#include "5.0.2/Format.h"
#include "6.0.1/Format.h"
#include "7.1.0/Format.h"
#include "8.0.1/Format.h"
#include "9.0.1/Format.h"
#include <iostream>
#include <sstream>

namespace clang_vx {

tok::TokenKind getTokenFromQualifier(const std::string &Qualifier) {
  // Don't let 'type' be an identifier, but steal typeof token.
  return llvm::StringSwitch<tok::TokenKind>(Qualifier)
      .Case("type", tok::kw_typeof)
      .Case("const", tok::kw_const)
      .Case("volatile", tok::kw_volatile)
      .Case("static", tok::kw_static)
      .Case("inline", tok::kw_inline)
      .Case("constexpr", tok::kw_constexpr)
      .Case("restrict", tok::kw_restrict)
      .Case("friend", tok::kw_friend)
      .Default(tok::identifier);
}

#define XSTR(s) STR(s)
#define STR(s) #s

#define PARSE_CONFIG(VERSION)                                                  \
  {                                                                            \
    clang_v##VERSION::FormatStyle format;                                      \
    format.Language = clang_v##VERSION::FormatStyle::LanguageKind::LK_Cpp;     \
    std::error_code ec =                                                       \
        clang_v##VERSION::parseConfiguration(config, &format);                 \
    std::cout << ec << "\n";                                                   \
    if (ec.value() == 0) {                                                     \
      retval.push_back(Version::V##VERSION);                                   \
    } else {                                                                   \
      std::cout << "v" << STR(VERSION) << "\n";                                \
    }                                                                          \
  }                                                                            \
  static_assert(true)

std::vector<Version> getCompatibleVersion(const std::string &config) {
  std::vector<Version> retval;

  {
    clang_v3_4::FormatStyle format;
    std::error_code ec = clang_v3_4::parseConfiguration(config, &format);
    if (ec.value() == 0) {
      retval.push_back(Version::V3_4);
    }
  }
  PARSE_CONFIG(3_5);
  PARSE_CONFIG(3_6);
  PARSE_CONFIG(3_7);
  PARSE_CONFIG(3_8);
  PARSE_CONFIG(3_9);
  PARSE_CONFIG(4);
  PARSE_CONFIG(5);
  PARSE_CONFIG(6);
  PARSE_CONFIG(7);
  PARSE_CONFIG(8);
  PARSE_CONFIG(9);
  PARSE_CONFIG(10);
  PARSE_CONFIG(11);
  PARSE_CONFIG(12);
  PARSE_CONFIG(13);
  PARSE_CONFIG(14);
  PARSE_CONFIG(15);
  PARSE_CONFIG(16);
  PARSE_CONFIG(17);
  PARSE_CONFIG(18);
  PARSE_CONFIG(19);

  return retval;
}

std::vector<std::string> getStyleNames(clang_vx::Version version) {
  switch (version) {
  case clang_vx::Version::V3_3: {
    return clang_v3_3::getStyleNames();
  }
  case clang_vx::Version::V3_4: {
    return clang_v3_4::getStyleNames();
  }
  case clang_vx::Version::V3_5: {
    return clang_v3_5::getStyleNames();
  }
  case clang_vx::Version::V3_6: {
    return clang_v3_6::getStyleNames();
  }
  case clang_vx::Version::V3_7: {
    return clang_v3_7::getStyleNames();
  }
  case clang_vx::Version::V3_8: {
    return clang_v3_8::getStyleNames();
  }
  case clang_vx::Version::V3_9: {
    return clang_v3_9::getStyleNames();
  }
  case clang_vx::Version::V4: {
    return clang_v4::getStyleNames();
  }
  case clang_vx::Version::V5: {
    return clang_v5::getStyleNames();
  }
  case clang_vx::Version::V6: {
    return clang_v6::getStyleNames();
  }
  case clang_vx::Version::V7: {
    return clang_v7::getStyleNames();
  }
  case clang_vx::Version::V8: {
    return clang_v8::getStyleNames();
  }
  case clang_vx::Version::V9: {
    return clang_v9::getStyleNames();
  }
  case clang_vx::Version::V10: {
    return clang_v10::getStyleNames();
  }
  case clang_vx::Version::V11: {
    return clang_v11::getStyleNames();
  }
  case clang_vx::Version::V12: {
    return clang_v12::getStyleNames();
  }
  case clang_vx::Version::V13: {
    return clang_v13::getStyleNames();
  }
  case clang_vx::Version::V14: {
    return clang_v14::getStyleNames();
  }
  case clang_vx::Version::V15: {
    return clang_v15::getStyleNames();
  }
  case clang_vx::Version::V16: {
    return clang_v16::getStyleNames();
  }
  case clang_vx::Version::V17: {
    return clang_v17::getStyleNames();
  }
  case clang_vx::Version::V18: {
    return clang_v18::getStyleNames();
  }
  case clang_vx::Version::V19: {
    return clang_v19::getStyleNames();
  }
  default: {
    return {};
  }
  }
}

} // namespace clang_vx