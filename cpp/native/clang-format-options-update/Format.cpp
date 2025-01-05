#include "Format.h"

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

} // namespace clang_vx