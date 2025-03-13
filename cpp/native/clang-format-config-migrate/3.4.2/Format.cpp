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

#include "Format.h"
#include "../Format.h"
#include "clang/Basic/OperatorPrecedence.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Debug.h"
#include <set>

namespace llvm {
namespace yaml {
template <>
struct ScalarEnumerationTraits<clang_v3_4::FormatStyle::LanguageStandard> {
  static void enumeration(IO &IO,
                          clang_v3_4::FormatStyle::LanguageStandard &Value) {
    IO.enumCase(Value, "Cpp03", clang_v3_4::FormatStyle::LS_Cpp03);
    IO.enumCase(Value, "C++03", clang_v3_4::FormatStyle::LS_Cpp03);
    IO.enumCase(Value, "Cpp11", clang_v3_4::FormatStyle::LS_Cpp11);
    IO.enumCase(Value, "C++11", clang_v3_4::FormatStyle::LS_Cpp11);
    IO.enumCase(Value, "Auto", clang_v3_4::FormatStyle::LS_Auto);
  }
};

template <>
struct ScalarEnumerationTraits<clang_v3_4::FormatStyle::UseTabStyle> {
  static void enumeration(IO &IO, clang_v3_4::FormatStyle::UseTabStyle &Value) {
    IO.enumCase(Value, "Never", clang_v3_4::FormatStyle::UT_Never);
    IO.enumCase(Value, "false", clang_v3_4::FormatStyle::UT_Never);
    IO.enumCase(Value, "Always", clang_v3_4::FormatStyle::UT_Always);
    IO.enumCase(Value, "true", clang_v3_4::FormatStyle::UT_Always);
    IO.enumCase(Value, "ForIndentation",
                clang_v3_4::FormatStyle::UT_ForIndentation);
  }
};

template <>
struct ScalarEnumerationTraits<clang_v3_4::FormatStyle::BraceBreakingStyle> {
  static void enumeration(IO &IO,
                          clang_v3_4::FormatStyle::BraceBreakingStyle &Value) {
    IO.enumCase(Value, "Attach", clang_v3_4::FormatStyle::BS_Attach);
    IO.enumCase(Value, "Linux", clang_v3_4::FormatStyle::BS_Linux);
    IO.enumCase(Value, "Stroustrup", clang_v3_4::FormatStyle::BS_Stroustrup);
    IO.enumCase(Value, "Allman", clang_v3_4::FormatStyle::BS_Allman);
  }
};

template <>
struct ScalarEnumerationTraits<
    clang_v3_4::FormatStyle::NamespaceIndentationKind> {
  static void
  enumeration(IO &IO,
              clang_v3_4::FormatStyle::NamespaceIndentationKind &Value) {
    IO.enumCase(Value, "None", clang_v3_4::FormatStyle::NI_None);
    IO.enumCase(Value, "Inner", clang_v3_4::FormatStyle::NI_Inner);
    IO.enumCase(Value, "All", clang_v3_4::FormatStyle::NI_All);
  }
};

template <> struct MappingTraits<clang_v3_4::FormatStyle> {
  static void mapping(llvm::yaml::IO &IO, clang_v3_4::FormatStyle &Style) {
    std::string BasedOnStyle;
    if (IO.outputting()) {
      clang_vx::OutputDiffOnly<clang_v3_4::FormatStyle> &out =
          static_cast<clang_vx::OutputDiffOnly<clang_v3_4::FormatStyle> &>(IO);
      if (out.getDefaultStyle()) {
        for (const std::string &StyleName : clang_v3_4::getStyleNames()) {
          clang_v3_4::FormatStyle PredefinedStyle;
          if (clang_v3_4::getPredefinedStyle(StyleName, &PredefinedStyle) &&
              *out.getDefaultStyle() == PredefinedStyle) {
            BasedOnStyle = StyleName;
            break;
          }
        }
        IO.mapOptional("BasedOnStyle", BasedOnStyle);
      }
    } else {
      IO.mapOptional("BasedOnStyle", BasedOnStyle);
      for (const std::string &StyleName : clang_v3_4::getStyleNames()) {
        clang_v3_4::FormatStyle PredefinedStyle;
        if (clang_v3_4::getPredefinedStyle(StyleName, &PredefinedStyle) &&
            Style == PredefinedStyle) {
          BasedOnStyle = StyleName;
          break;
        }
      }
    }

    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "AccessModifierOffset", Style.AccessModifierOffset);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "ConstructorInitializerIndentWidth",
        Style.ConstructorInitializerIndentWidth);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "AlignEscapedNewlinesLeft", Style.AlignEscapedNewlinesLeft);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "AlignTrailingComments", Style.AlignTrailingComments);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "AllowAllParametersOfDeclarationOnNextLine",
        Style.AllowAllParametersOfDeclarationOnNextLine);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "AllowShortIfStatementsOnASingleLine",
        Style.AllowShortIfStatementsOnASingleLine);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "AllowShortLoopsOnASingleLine", Style.AllowShortLoopsOnASingleLine);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "AlwaysBreakTemplateDeclarations",
        Style.AlwaysBreakTemplateDeclarations);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "AlwaysBreakBeforeMultilineStrings",
        Style.AlwaysBreakBeforeMultilineStrings);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "BreakBeforeBinaryOperators", Style.BreakBeforeBinaryOperators);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "BreakBeforeTernaryOperators", Style.BreakBeforeTernaryOperators);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "BreakConstructorInitializersBeforeComma",
        Style.BreakConstructorInitializersBeforeComma);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "BinPackParameters",
                                                     Style.BinPackParameters);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "ColumnLimit",
                                                     Style.ColumnLimit);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "ConstructorInitializerAllOnOneLineOrOnePerLine",
        Style.ConstructorInitializerAllOnOneLineOrOnePerLine);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "DerivePointerBinding", Style.DerivePointerBinding);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "ExperimentalAutoDetectBinPacking",
        Style.ExperimentalAutoDetectBinPacking);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "IndentCaseLabels",
                                                     Style.IndentCaseLabels);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "MaxEmptyLinesToKeep",
                                                     Style.MaxEmptyLinesToKeep);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "NamespaceIndentation", Style.NamespaceIndentation);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "ObjCSpaceBeforeProtocolList", Style.ObjCSpaceBeforeProtocolList);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "PenaltyBreakBeforeFirstCallParameter",
        Style.PenaltyBreakBeforeFirstCallParameter);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "PenaltyBreakComment",
                                                     Style.PenaltyBreakComment);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "PenaltyBreakString",
                                                     Style.PenaltyBreakString);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "PenaltyBreakFirstLessLess", Style.PenaltyBreakFirstLessLess);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "PenaltyExcessCharacter", Style.PenaltyExcessCharacter);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "PenaltyReturnTypeOnItsOwnLine",
        Style.PenaltyReturnTypeOnItsOwnLine);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "PointerBindsToType",
                                                     Style.PointerBindsToType);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "SpacesBeforeTrailingComments", Style.SpacesBeforeTrailingComments);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "Cpp11BracedListStyle", Style.Cpp11BracedListStyle);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "Standard",
                                                     Style.Standard);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "IndentWidth",
                                                     Style.IndentWidth);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "TabWidth",
                                                     Style.TabWidth);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "UseTab",
                                                     Style.UseTab);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "BreakBeforeBraces",
                                                     Style.BreakBeforeBraces);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "IndentFunctionDeclarationAfterType",
        Style.IndentFunctionDeclarationAfterType);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "SpacesInParentheses",
                                                     Style.SpacesInParentheses);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(IO, "SpacesInAngles",
                                                     Style.SpacesInAngles);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "SpaceInEmptyParentheses", Style.SpaceInEmptyParentheses);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "SpacesInCStyleCastParentheses",
        Style.SpacesInCStyleCastParentheses);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "SpaceAfterControlStatementKeyword",
        Style.SpaceAfterControlStatementKeyword);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "SpaceBeforeAssignmentOperators",
        Style.SpaceBeforeAssignmentOperators);
    clang_vx::IoMapOptional<clang_v3_4::FormatStyle>(
        IO, "ContinuationIndentWidth", Style.ContinuationIndentWidth);
  }
};
} // namespace yaml
} // namespace llvm

namespace clang_v3_4 {

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

bool getPredefinedStyle(llvm::StringRef Name, FormatStyle *Style) {
  if (Name.equals_insensitive("llvm"))
    *Style = getLLVMStyle();
  else if (Name.equals_insensitive("chromium"))
    *Style = getChromiumStyle();
  else if (Name.equals_insensitive("mozilla"))
    *Style = getMozillaStyle();
  else if (Name.equals_insensitive("google"))
    *Style = getGoogleStyle();
  else if (Name.equals_insensitive("webkit"))
    *Style = getWebKitStyle();
  else
    return false;

  return true;
}

std::vector<std::string> getStyleNames() {
  return {"chromium", "google", "llvm", "mozilla", "webkit"};
}

std::error_code parseConfiguration(const std::string &Text,
                                   FormatStyle *Style) {
  llvm::yaml::Input Input(Text);
  Input >> *Style;
  return Input.error();
}

std::string configurationAsText(const FormatStyle &Style,
                                const std::string &DefaultStyleName,
                                bool SkipSameValue) {
  std::string Text;
  llvm::raw_string_ostream Stream(Text);
  FormatStyle DefaultStyle;
  // We use the same mapping method for input and output, so we need a
  // non-const reference here.
  FormatStyle NonConstStyle = Style;
  if (!SkipSameValue || !getPredefinedStyle(DefaultStyleName, &DefaultStyle)) {
    clang_vx::OutputDiffOnly<FormatStyle> Output(nullptr, NonConstStyle, false,
                                                 Stream);
    Output << NonConstStyle;
  } else {
    clang_vx::OutputDiffOnly<FormatStyle> Output(&DefaultStyle, NonConstStyle,
                                                 SkipSameValue, Stream);
    Output << NonConstStyle;
  }
  return Stream.str();
}

} // namespace clang_v3_4
