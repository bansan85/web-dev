#pragma once

#include "clang/Basic/OperatorPrecedence.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Debug.h"
#include <set>
#include <string>
#include <vector>

namespace clang_vx {

namespace tok = clang::tok;

tok::TokenKind getTokenFromQualifier(const std::string &Qualifier);

enum class Version {
  V3_3,
  V3_4,
  V3_5,
  V3_6,
  V3_7,
  V3_8,
  V3_9,
  V4,
  V5,
  V6,
  V7,
  V8,
  V9,
  V10,
  V11,
  V12,
  V13,
  V14,
  V15,
  V16,
  V17,
  V18,
  V19
};

std::vector<Version> getCompatibleVersion(const std::string &config);

} // namespace clang_vx