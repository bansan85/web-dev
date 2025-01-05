//===--- Format.cpp - Format C++ code -------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file implements functions declared in Format.h. This will be
/// split into separate files as we go.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "format-formatter"

#include "BreakableToken.h"
#include "TokenAnnotator.h"
#include "UnwrappedLineParser.h"
#include "WhitespaceManager.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/OperatorPrecedence.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Format/Format.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Debug.h"
#include <queue>
#include <string>

namespace clang {
namespace format {

FormatStyle getLLVMStyle() {
  FormatStyle LLVMStyle;
  LLVMStyle.AccessModifierOffset = -2;
  LLVMStyle.AlignEscapedNewlinesLeft = false;
  LLVMStyle.AllowAllParametersOfDeclarationOnNextLine = true;
  LLVMStyle.AllowShortIfStatementsOnASingleLine = false;
  LLVMStyle.BinPackParameters = true;
  LLVMStyle.ColumnLimit = 80;
  LLVMStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
  LLVMStyle.DerivePointerBinding = false;
  LLVMStyle.IndentCaseLabels = false;
  LLVMStyle.MaxEmptyLinesToKeep = 1;
  LLVMStyle.ObjCSpaceBeforeProtocolList = true;
  LLVMStyle.PenaltyExcessCharacter = 1000000;
  LLVMStyle.PenaltyReturnTypeOnItsOwnLine = 75;
  LLVMStyle.PointerBindsToType = false;
  LLVMStyle.SpacesBeforeTrailingComments = 1;
  LLVMStyle.Standard = FormatStyle::LS_Cpp03;
  return LLVMStyle;
}

FormatStyle getGoogleStyle() {
  FormatStyle GoogleStyle;
  GoogleStyle.AccessModifierOffset = -1;
  GoogleStyle.AlignEscapedNewlinesLeft = true;
  GoogleStyle.AllowAllParametersOfDeclarationOnNextLine = true;
  GoogleStyle.AllowShortIfStatementsOnASingleLine = true;
  GoogleStyle.BinPackParameters = true;
  GoogleStyle.ColumnLimit = 80;
  GoogleStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
  GoogleStyle.DerivePointerBinding = true;
  GoogleStyle.IndentCaseLabels = true;
  GoogleStyle.MaxEmptyLinesToKeep = 1;
  GoogleStyle.ObjCSpaceBeforeProtocolList = false;
  GoogleStyle.PenaltyExcessCharacter = 1000000;
  GoogleStyle.PenaltyReturnTypeOnItsOwnLine = 200;
  GoogleStyle.PointerBindsToType = true;
  GoogleStyle.SpacesBeforeTrailingComments = 2;
  GoogleStyle.Standard = FormatStyle::LS_Auto;
  return GoogleStyle;
}

FormatStyle getChromiumStyle() {
  FormatStyle ChromiumStyle = getGoogleStyle();
  ChromiumStyle.AllowAllParametersOfDeclarationOnNextLine = false;
  ChromiumStyle.AllowShortIfStatementsOnASingleLine = false;
  ChromiumStyle.BinPackParameters = false;
  ChromiumStyle.Standard = FormatStyle::LS_Cpp03;
  ChromiumStyle.DerivePointerBinding = false;
  return ChromiumStyle;
}

FormatStyle getMozillaStyle() {
  FormatStyle MozillaStyle = getLLVMStyle();
  MozillaStyle.AllowAllParametersOfDeclarationOnNextLine = false;
  MozillaStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
  MozillaStyle.DerivePointerBinding = true;
  MozillaStyle.IndentCaseLabels = true;
  MozillaStyle.ObjCSpaceBeforeProtocolList = false;
  MozillaStyle.PenaltyReturnTypeOnItsOwnLine = 200;
  MozillaStyle.PointerBindsToType = true;
  return MozillaStyle;
}

// Returns the length of everything up to the first possible line break after
// the ), ], } or > matching \c Tok.
static unsigned getLengthToMatchingParen(const AnnotatedToken &Tok) {
  if (Tok.MatchingParen == NULL)
    return 0;
  AnnotatedToken *End = Tok.MatchingParen;
  while (!End->Children.empty() && !End->Children[0].CanBreakBefore) {
    End = &End->Children[0];
  }
  return End->TotalLength - Tok.TotalLength + 1;
}

LangOptions getFormattingLangOpts() {
  LangOptions LangOpts;
  LangOpts.CPlusPlus = 1;
  LangOpts.CPlusPlus11 = 1;
  LangOpts.LineComment = 1;
  LangOpts.Bool = 1;
  LangOpts.ObjC1 = 1;
  LangOpts.ObjC2 = 1;
  return LangOpts;
}

} // namespace format
} // namespace clang
