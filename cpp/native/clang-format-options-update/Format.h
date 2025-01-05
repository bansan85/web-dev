#pragma once

#include "clang/Basic/OperatorPrecedence.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Debug.h"
#include <set>
#include <string>

namespace clang_vx {

namespace tok = clang::tok;

tok::TokenKind getTokenFromQualifier(const std::string &Qualifier);

} // namespace clang_vx