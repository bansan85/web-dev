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

#include "ContinuationIndenter.h"
#include "TokenAnnotator.h"
#include "UnwrappedLineParser.h"
#include "WhitespaceManager.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Format/Format.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Support/Path.h"
#include <queue>
#include <string>

namespace llvm {
namespace yaml {
template <>
struct ScalarEnumerationTraits<clang::format::FormatStyle::LanguageStandard> {
  static void enumeration(IO &IO,
                          clang::format::FormatStyle::LanguageStandard &Value) {
    IO.enumCase(Value, "Cpp03", clang::format::FormatStyle::LS_Cpp03);
    IO.enumCase(Value, "C++03", clang::format::FormatStyle::LS_Cpp03);
    IO.enumCase(Value, "Cpp11", clang::format::FormatStyle::LS_Cpp11);
    IO.enumCase(Value, "C++11", clang::format::FormatStyle::LS_Cpp11);
    IO.enumCase(Value, "Auto", clang::format::FormatStyle::LS_Auto);
  }
};

template <>
struct ScalarEnumerationTraits<clang::format::FormatStyle::UseTabStyle> {
  static void enumeration(IO &IO,
                          clang::format::FormatStyle::UseTabStyle &Value) {
    IO.enumCase(Value, "Never", clang::format::FormatStyle::UT_Never);
    IO.enumCase(Value, "false", clang::format::FormatStyle::UT_Never);
    IO.enumCase(Value, "Always", clang::format::FormatStyle::UT_Always);
    IO.enumCase(Value, "true", clang::format::FormatStyle::UT_Always);
    IO.enumCase(Value, "ForIndentation",
                clang::format::FormatStyle::UT_ForIndentation);
  }
};

template <>
struct ScalarEnumerationTraits<clang::format::FormatStyle::BraceBreakingStyle> {
  static void
  enumeration(IO &IO, clang::format::FormatStyle::BraceBreakingStyle &Value) {
    IO.enumCase(Value, "Attach", clang::format::FormatStyle::BS_Attach);
    IO.enumCase(Value, "Linux", clang::format::FormatStyle::BS_Linux);
    IO.enumCase(Value, "Stroustrup", clang::format::FormatStyle::BS_Stroustrup);
    IO.enumCase(Value, "Allman", clang::format::FormatStyle::BS_Allman);
  }
};

template <>
struct ScalarEnumerationTraits<
    clang::format::FormatStyle::NamespaceIndentationKind> {
  static void
  enumeration(IO &IO,
              clang::format::FormatStyle::NamespaceIndentationKind &Value) {
    IO.enumCase(Value, "None", clang::format::FormatStyle::NI_None);
    IO.enumCase(Value, "Inner", clang::format::FormatStyle::NI_Inner);
    IO.enumCase(Value, "All", clang::format::FormatStyle::NI_All);
  }
};

template <> struct MappingTraits<clang::format::FormatStyle> {
  static void mapping(llvm::yaml::IO &IO, clang::format::FormatStyle &Style) {
    if (IO.outputting()) {
      StringRef StylesArray[] = { "LLVM",    "Google", "Chromium",
                                  "Mozilla", "WebKit" };
      ArrayRef<StringRef> Styles(StylesArray);
      for (size_t i = 0, e = Styles.size(); i < e; ++i) {
        StringRef StyleName(Styles[i]);
        clang::format::FormatStyle PredefinedStyle;
        if (clang::format::getPredefinedStyle(StyleName, &PredefinedStyle) &&
            Style == PredefinedStyle) {
          IO.mapOptional("# BasedOnStyle", StyleName);
          break;
        }
      }
    } else {
      StringRef BasedOnStyle;
      IO.mapOptional("BasedOnStyle", BasedOnStyle);
      if (!BasedOnStyle.empty())
        if (!clang::format::getPredefinedStyle(BasedOnStyle, &Style)) {
          IO.setError(Twine("Unknown value for BasedOnStyle: ", BasedOnStyle));
          return;
        }
    }

    IO.mapOptional("AccessModifierOffset", Style.AccessModifierOffset);
    IO.mapOptional("ConstructorInitializerIndentWidth",
                   Style.ConstructorInitializerIndentWidth);
    IO.mapOptional("AlignEscapedNewlinesLeft", Style.AlignEscapedNewlinesLeft);
    IO.mapOptional("AlignTrailingComments", Style.AlignTrailingComments);
    IO.mapOptional("AllowAllParametersOfDeclarationOnNextLine",
                   Style.AllowAllParametersOfDeclarationOnNextLine);
    IO.mapOptional("AllowShortIfStatementsOnASingleLine",
                   Style.AllowShortIfStatementsOnASingleLine);
    IO.mapOptional("AllowShortLoopsOnASingleLine",
                   Style.AllowShortLoopsOnASingleLine);
    IO.mapOptional("AlwaysBreakTemplateDeclarations",
                   Style.AlwaysBreakTemplateDeclarations);
    IO.mapOptional("AlwaysBreakBeforeMultilineStrings",
                   Style.AlwaysBreakBeforeMultilineStrings);
    IO.mapOptional("BreakBeforeBinaryOperators",
                   Style.BreakBeforeBinaryOperators);
    IO.mapOptional("BreakBeforeTernaryOperators",
                   Style.BreakBeforeTernaryOperators);
    IO.mapOptional("BreakConstructorInitializersBeforeComma",
                   Style.BreakConstructorInitializersBeforeComma);
    IO.mapOptional("BinPackParameters", Style.BinPackParameters);
    IO.mapOptional("ColumnLimit", Style.ColumnLimit);
    IO.mapOptional("ConstructorInitializerAllOnOneLineOrOnePerLine",
                   Style.ConstructorInitializerAllOnOneLineOrOnePerLine);
    IO.mapOptional("DerivePointerBinding", Style.DerivePointerBinding);
    IO.mapOptional("ExperimentalAutoDetectBinPacking",
                   Style.ExperimentalAutoDetectBinPacking);
    IO.mapOptional("IndentCaseLabels", Style.IndentCaseLabels);
    IO.mapOptional("MaxEmptyLinesToKeep", Style.MaxEmptyLinesToKeep);
    IO.mapOptional("NamespaceIndentation", Style.NamespaceIndentation);
    IO.mapOptional("ObjCSpaceBeforeProtocolList",
                   Style.ObjCSpaceBeforeProtocolList);
    IO.mapOptional("PenaltyBreakBeforeFirstCallParameter",
                   Style.PenaltyBreakBeforeFirstCallParameter);
    IO.mapOptional("PenaltyBreakComment", Style.PenaltyBreakComment);
    IO.mapOptional("PenaltyBreakString", Style.PenaltyBreakString);
    IO.mapOptional("PenaltyBreakFirstLessLess",
                   Style.PenaltyBreakFirstLessLess);
    IO.mapOptional("PenaltyExcessCharacter", Style.PenaltyExcessCharacter);
    IO.mapOptional("PenaltyReturnTypeOnItsOwnLine",
                   Style.PenaltyReturnTypeOnItsOwnLine);
    IO.mapOptional("PointerBindsToType", Style.PointerBindsToType);
    IO.mapOptional("SpacesBeforeTrailingComments",
                   Style.SpacesBeforeTrailingComments);
    IO.mapOptional("Cpp11BracedListStyle", Style.Cpp11BracedListStyle);
    IO.mapOptional("Standard", Style.Standard);
    IO.mapOptional("IndentWidth", Style.IndentWidth);
    IO.mapOptional("TabWidth", Style.TabWidth);
    IO.mapOptional("UseTab", Style.UseTab);
    IO.mapOptional("BreakBeforeBraces", Style.BreakBeforeBraces);
    IO.mapOptional("IndentFunctionDeclarationAfterType",
                   Style.IndentFunctionDeclarationAfterType);
    IO.mapOptional("SpacesInParentheses", Style.SpacesInParentheses);
    IO.mapOptional("SpacesInAngles", Style.SpacesInAngles);
    IO.mapOptional("SpaceInEmptyParentheses", Style.SpaceInEmptyParentheses);
    IO.mapOptional("SpacesInCStyleCastParentheses",
                   Style.SpacesInCStyleCastParentheses);
    IO.mapOptional("SpaceAfterControlStatementKeyword",
                   Style.SpaceAfterControlStatementKeyword);
    IO.mapOptional("SpaceBeforeAssignmentOperators",
                   Style.SpaceBeforeAssignmentOperators);
    IO.mapOptional("ContinuationIndentWidth", Style.ContinuationIndentWidth);
  }
};
}
}

namespace clang {
namespace format {

void setDefaultPenalties(FormatStyle &Style) {
  Style.PenaltyBreakComment = 60;
  Style.PenaltyBreakFirstLessLess = 120;
  Style.PenaltyBreakString = 1000;
  Style.PenaltyExcessCharacter = 1000000;
}

FormatStyle getLLVMStyle() {
  FormatStyle LLVMStyle;
  LLVMStyle.AccessModifierOffset = -2;
  LLVMStyle.AlignEscapedNewlinesLeft = false;
  LLVMStyle.AlignTrailingComments = true;
  LLVMStyle.AllowAllParametersOfDeclarationOnNextLine = true;
  LLVMStyle.AllowShortIfStatementsOnASingleLine = false;
  LLVMStyle.AllowShortLoopsOnASingleLine = false;
  LLVMStyle.AlwaysBreakBeforeMultilineStrings = false;
  LLVMStyle.AlwaysBreakTemplateDeclarations = false;
  LLVMStyle.BinPackParameters = true;
  LLVMStyle.BreakBeforeBinaryOperators = false;
  LLVMStyle.BreakBeforeTernaryOperators = true;
  LLVMStyle.BreakBeforeBraces = FormatStyle::BS_Attach;
  LLVMStyle.BreakConstructorInitializersBeforeComma = false;
  LLVMStyle.ColumnLimit = 80;
  LLVMStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
  LLVMStyle.ConstructorInitializerIndentWidth = 4;
  LLVMStyle.Cpp11BracedListStyle = false;
  LLVMStyle.DerivePointerBinding = false;
  LLVMStyle.ExperimentalAutoDetectBinPacking = false;
  LLVMStyle.IndentCaseLabels = false;
  LLVMStyle.IndentFunctionDeclarationAfterType = false;
  LLVMStyle.IndentWidth = 2;
  LLVMStyle.TabWidth = 8;
  LLVMStyle.MaxEmptyLinesToKeep = 1;
  LLVMStyle.NamespaceIndentation = FormatStyle::NI_None;
  LLVMStyle.ObjCSpaceBeforeProtocolList = true;
  LLVMStyle.PointerBindsToType = false;
  LLVMStyle.SpacesBeforeTrailingComments = 1;
  LLVMStyle.Standard = FormatStyle::LS_Cpp03;
  LLVMStyle.UseTab = FormatStyle::UT_Never;
  LLVMStyle.SpacesInParentheses = false;
  LLVMStyle.SpaceInEmptyParentheses = false;
  LLVMStyle.SpacesInCStyleCastParentheses = false;
  LLVMStyle.SpaceAfterControlStatementKeyword = true;
  LLVMStyle.SpaceBeforeAssignmentOperators = true;
  LLVMStyle.ContinuationIndentWidth = 4;
  LLVMStyle.SpacesInAngles = false;

  setDefaultPenalties(LLVMStyle);
  LLVMStyle.PenaltyReturnTypeOnItsOwnLine = 60;
  LLVMStyle.PenaltyBreakBeforeFirstCallParameter = 19;

  return LLVMStyle;
}

FormatStyle getGoogleStyle() {
  FormatStyle GoogleStyle;
  GoogleStyle.AccessModifierOffset = -1;
  GoogleStyle.AlignEscapedNewlinesLeft = true;
  GoogleStyle.AlignTrailingComments = true;
  GoogleStyle.AllowAllParametersOfDeclarationOnNextLine = true;
  GoogleStyle.AllowShortIfStatementsOnASingleLine = true;
  GoogleStyle.AllowShortLoopsOnASingleLine = true;
  GoogleStyle.AlwaysBreakBeforeMultilineStrings = true;
  GoogleStyle.AlwaysBreakTemplateDeclarations = true;
  GoogleStyle.BinPackParameters = true;
  GoogleStyle.BreakBeforeBinaryOperators = false;
  GoogleStyle.BreakBeforeTernaryOperators = true;
  GoogleStyle.BreakBeforeBraces = FormatStyle::BS_Attach;
  GoogleStyle.BreakConstructorInitializersBeforeComma = false;
  GoogleStyle.ColumnLimit = 80;
  GoogleStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
  GoogleStyle.ConstructorInitializerIndentWidth = 4;
  GoogleStyle.Cpp11BracedListStyle = true;
  GoogleStyle.DerivePointerBinding = true;
  GoogleStyle.ExperimentalAutoDetectBinPacking = false;
  GoogleStyle.IndentCaseLabels = true;
  GoogleStyle.IndentFunctionDeclarationAfterType = true;
  GoogleStyle.IndentWidth = 2;
  GoogleStyle.TabWidth = 8;
  GoogleStyle.MaxEmptyLinesToKeep = 1;
  GoogleStyle.NamespaceIndentation = FormatStyle::NI_None;
  GoogleStyle.ObjCSpaceBeforeProtocolList = false;
  GoogleStyle.PointerBindsToType = true;
  GoogleStyle.SpacesBeforeTrailingComments = 2;
  GoogleStyle.Standard = FormatStyle::LS_Auto;
  GoogleStyle.UseTab = FormatStyle::UT_Never;
  GoogleStyle.SpacesInParentheses = false;
  GoogleStyle.SpaceInEmptyParentheses = false;
  GoogleStyle.SpacesInCStyleCastParentheses = false;
  GoogleStyle.SpaceAfterControlStatementKeyword = true;
  GoogleStyle.SpaceBeforeAssignmentOperators = true;
  GoogleStyle.ContinuationIndentWidth = 4;
  GoogleStyle.SpacesInAngles = false;

  setDefaultPenalties(GoogleStyle);
  GoogleStyle.PenaltyReturnTypeOnItsOwnLine = 200;
  GoogleStyle.PenaltyBreakBeforeFirstCallParameter = 1;

  return GoogleStyle;
}

FormatStyle getChromiumStyle() {
  FormatStyle ChromiumStyle = getGoogleStyle();
  ChromiumStyle.AllowAllParametersOfDeclarationOnNextLine = false;
  ChromiumStyle.AllowShortIfStatementsOnASingleLine = false;
  ChromiumStyle.AllowShortLoopsOnASingleLine = false;
  ChromiumStyle.BinPackParameters = false;
  ChromiumStyle.DerivePointerBinding = false;
  ChromiumStyle.Standard = FormatStyle::LS_Cpp03;
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

FormatStyle getWebKitStyle() {
  FormatStyle Style = getLLVMStyle();
  Style.AccessModifierOffset = -4;
  Style.AlignTrailingComments = false;
  Style.BreakBeforeBinaryOperators = true;
  Style.BreakBeforeBraces = FormatStyle::BS_Stroustrup;
  Style.BreakConstructorInitializersBeforeComma = true;
  Style.ColumnLimit = 0;
  Style.IndentWidth = 4;
  Style.NamespaceIndentation = FormatStyle::NI_Inner;
  Style.PointerBindsToType = true;
  return Style;
}

bool getPredefinedStyle(StringRef Name, FormatStyle *Style) {
  if (Name.equals_lower("llvm"))
    *Style = getLLVMStyle();
  else if (Name.equals_lower("chromium"))
    *Style = getChromiumStyle();
  else if (Name.equals_lower("mozilla"))
    *Style = getMozillaStyle();
  else if (Name.equals_lower("google"))
    *Style = getGoogleStyle();
  else if (Name.equals_lower("webkit"))
    *Style = getWebKitStyle();
  else
    return false;

  return true;
}

llvm::error_code parseConfiguration(StringRef Text, FormatStyle *Style) {
  if (Text.trim().empty())
    return llvm::make_error_code(llvm::errc::invalid_argument);
  llvm::yaml::Input Input(Text);
  Input >> *Style;
  return Input.error();
}

std::string configurationAsText(const FormatStyle &Style) {
  std::string Text;
  llvm::raw_string_ostream Stream(Text);
  llvm::yaml::Output Output(Stream);
  // We use the same mapping method for input and output, so we need a non-const
  // reference here.
  FormatStyle NonConstStyle = Style;
  Output << NonConstStyle;
  return Stream.str();
}

} // namespace format
} // namespace clang
