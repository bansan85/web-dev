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

#include "Format.h"
#include "../Format.h"
#include "clang/Basic/OperatorPrecedence.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Debug.h"
#include <set>

#define DEBUG_TYPE "format-formatter"

using clang_v3_6::FormatStyle;

namespace llvm {
namespace yaml {
template <> struct ScalarEnumerationTraits<FormatStyle::LanguageKind> {
  static void enumeration(IO &IO, FormatStyle::LanguageKind &Value) {
    IO.enumCase(Value, "Cpp", FormatStyle::LK_Cpp);
    IO.enumCase(Value, "Java", FormatStyle::LK_Java);
    IO.enumCase(Value, "JavaScript", FormatStyle::LK_JavaScript);
    IO.enumCase(Value, "Proto", FormatStyle::LK_Proto);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::LanguageStandard> {
  static void enumeration(IO &IO, FormatStyle::LanguageStandard &Value) {
    IO.enumCase(Value, "Cpp03", FormatStyle::LS_Cpp03);
    IO.enumCase(Value, "C++03", FormatStyle::LS_Cpp03);
    IO.enumCase(Value, "Cpp11", FormatStyle::LS_Cpp11);
    IO.enumCase(Value, "C++11", FormatStyle::LS_Cpp11);
    IO.enumCase(Value, "Auto", FormatStyle::LS_Auto);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::UseTabStyle> {
  static void enumeration(IO &IO, FormatStyle::UseTabStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::UT_Never);
    IO.enumCase(Value, "false", FormatStyle::UT_Never);
    IO.enumCase(Value, "Always", FormatStyle::UT_Always);
    IO.enumCase(Value, "true", FormatStyle::UT_Always);
    IO.enumCase(Value, "ForIndentation", FormatStyle::UT_ForIndentation);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::ShortFunctionStyle> {
  static void enumeration(IO &IO, FormatStyle::ShortFunctionStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::SFS_None);
    IO.enumCase(Value, "false", FormatStyle::SFS_None);
    IO.enumCase(Value, "All", FormatStyle::SFS_All);
    IO.enumCase(Value, "true", FormatStyle::SFS_All);
    IO.enumCase(Value, "Inline", FormatStyle::SFS_Inline);
    IO.enumCase(Value, "Empty", FormatStyle::SFS_Empty);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::BinaryOperatorStyle> {
  static void enumeration(IO &IO, FormatStyle::BinaryOperatorStyle &Value) {
    IO.enumCase(Value, "All", FormatStyle::BOS_All);
    IO.enumCase(Value, "true", FormatStyle::BOS_All);
    IO.enumCase(Value, "None", FormatStyle::BOS_None);
    IO.enumCase(Value, "false", FormatStyle::BOS_None);
    IO.enumCase(Value, "NonAssignment", FormatStyle::BOS_NonAssignment);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::BraceBreakingStyle> {
  static void enumeration(IO &IO, FormatStyle::BraceBreakingStyle &Value) {
    IO.enumCase(Value, "Attach", FormatStyle::BS_Attach);
    IO.enumCase(Value, "Linux", FormatStyle::BS_Linux);
    IO.enumCase(Value, "Stroustrup", FormatStyle::BS_Stroustrup);
    IO.enumCase(Value, "Allman", FormatStyle::BS_Allman);
    IO.enumCase(Value, "GNU", FormatStyle::BS_GNU);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::NamespaceIndentationKind> {
  static void enumeration(IO &IO,
                          FormatStyle::NamespaceIndentationKind &Value) {
    IO.enumCase(Value, "None", FormatStyle::NI_None);
    IO.enumCase(Value, "Inner", FormatStyle::NI_Inner);
    IO.enumCase(Value, "All", FormatStyle::NI_All);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::PointerAlignmentStyle> {
  static void enumeration(IO &IO, FormatStyle::PointerAlignmentStyle &Value) {
    IO.enumCase(Value, "Middle", FormatStyle::PAS_Middle);
    IO.enumCase(Value, "Left", FormatStyle::PAS_Left);
    IO.enumCase(Value, "Right", FormatStyle::PAS_Right);

    // For backward compatibility.
    IO.enumCase(Value, "true", FormatStyle::PAS_Left);
    IO.enumCase(Value, "false", FormatStyle::PAS_Right);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::SpaceBeforeParensOptions> {
  static void enumeration(IO &IO,
                          FormatStyle::SpaceBeforeParensOptions &Value) {
    IO.enumCase(Value, "Never", FormatStyle::SBPO_Never);
    IO.enumCase(Value, "ControlStatements",
                FormatStyle::SBPO_ControlStatements);
    IO.enumCase(Value, "Always", FormatStyle::SBPO_Always);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::SBPO_Never);
    IO.enumCase(Value, "true", FormatStyle::SBPO_ControlStatements);
  }
};

template <> struct MappingTraits<FormatStyle> {
  static void mapping(IO &IO, FormatStyle &Style) {
    // When reading, read the language first, we need it for getPredefinedStyle.
    IO.mapOptional("Language", Style.Language);

    std::string BasedOnStyle;
    if (IO.outputting()) {
      clang_vx::OutputDiffOnly<FormatStyle> &out =
          *static_cast<clang_vx::OutputDiffOnly<FormatStyle> *>(
              IO.getContext());
      if (out.getDefaultStyle()) {
        for (const std::string &StyleName : clang_v3_6::getStyleNames()) {
          FormatStyle PredefinedStyle;
          if (clang_v3_6::getPredefinedStyle(StyleName, Style.Language,
                                             &PredefinedStyle) &&
              *out.getDefaultStyle() == PredefinedStyle) {
            BasedOnStyle = StyleName;
            break;
          }
        }
        IO.mapOptional("BasedOnStyle", BasedOnStyle);
      }
    } else {
      IO.mapOptional("BasedOnStyle", BasedOnStyle);
      for (const std::string &StyleName : clang_v3_6::getStyleNames()) {
        FormatStyle PredefinedStyle;
        if (clang_v3_6::getPredefinedStyle(StyleName, Style.Language,
                                           &PredefinedStyle) &&
            Style == PredefinedStyle) {
          BasedOnStyle = StyleName;
          break;
        }
      }
    }

    clang_vx::IoMapOptional<FormatStyle>(IO, "AccessModifierOffset",
                                         Style.AccessModifierOffset);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlignAfterOpenBracket",
                                         Style.AlignAfterOpenBracket);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlignEscapedNewlinesLeft",
                                         Style.AlignEscapedNewlinesLeft);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlignOperands",
                                         Style.AlignOperands);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlignTrailingComments",
                                         Style.AlignTrailingComments);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "AllowAllParametersOfDeclarationOnNextLine",
        Style.AllowAllParametersOfDeclarationOnNextLine);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AllowShortBlocksOnASingleLine",
                                         Style.AllowShortBlocksOnASingleLine);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "AllowShortCaseLabelsOnASingleLine",
        Style.AllowShortCaseLabelsOnASingleLine);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "AllowShortIfStatementsOnASingleLine",
        Style.AllowShortIfStatementsOnASingleLine);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AllowShortLoopsOnASingleLine",
                                         Style.AllowShortLoopsOnASingleLine);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "AllowShortFunctionsOnASingleLine",
        Style.AllowShortFunctionsOnASingleLine);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "AlwaysBreakAfterDefinitionReturnType",
        Style.AlwaysBreakAfterDefinitionReturnType);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlwaysBreakTemplateDeclarations",
                                         Style.AlwaysBreakTemplateDeclarations);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "AlwaysBreakBeforeMultilineStrings",
        Style.AlwaysBreakBeforeMultilineStrings);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BreakBeforeBinaryOperators",
                                         Style.BreakBeforeBinaryOperators);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BreakBeforeTernaryOperators",
                                         Style.BreakBeforeTernaryOperators);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "BreakConstructorInitializersBeforeComma",
        Style.BreakConstructorInitializersBeforeComma);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BinPackParameters",
                                         Style.BinPackParameters);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BinPackArguments",
                                         Style.BinPackArguments);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ColumnLimit", Style.ColumnLimit);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "ConstructorInitializerAllOnOneLineOrOnePerLine",
        Style.ConstructorInitializerAllOnOneLineOrOnePerLine);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "ConstructorInitializerIndentWidth",
        Style.ConstructorInitializerIndentWidth);
    clang_vx::IoMapOptional<FormatStyle>(IO, "DerivePointerAlignment",
                                         Style.DerivePointerAlignment);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "ExperimentalAutoDetectBinPacking",
        Style.ExperimentalAutoDetectBinPacking);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IndentCaseLabels",
                                         Style.IndentCaseLabels);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IndentWrappedFunctionNames",
                                         Style.IndentWrappedFunctionNames);
    clang_vx::IoMapOptional<FormatStyle>(IO,
                                         "IndentFunctionDeclarationAfterType",
                                         Style.IndentWrappedFunctionNames);
    clang_vx::IoMapOptional<FormatStyle>(IO, "MaxEmptyLinesToKeep",
                                         Style.MaxEmptyLinesToKeep);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "KeepEmptyLinesAtTheStartOfBlocks",
        Style.KeepEmptyLinesAtTheStartOfBlocks);
    clang_vx::IoMapOptional<FormatStyle>(IO, "NamespaceIndentation",
                                         Style.NamespaceIndentation);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ObjCBlockIndentWidth",
                                         Style.ObjCBlockIndentWidth);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ObjCSpaceAfterProperty",
                                         Style.ObjCSpaceAfterProperty);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ObjCSpaceBeforeProtocolList",
                                         Style.ObjCSpaceBeforeProtocolList);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "PenaltyBreakBeforeFirstCallParameter",
        Style.PenaltyBreakBeforeFirstCallParameter);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyBreakComment",
                                         Style.PenaltyBreakComment);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyBreakString",
                                         Style.PenaltyBreakString);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyBreakFirstLessLess",
                                         Style.PenaltyBreakFirstLessLess);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyExcessCharacter",
                                         Style.PenaltyExcessCharacter);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyReturnTypeOnItsOwnLine",
                                         Style.PenaltyReturnTypeOnItsOwnLine);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PointerAlignment",
                                         Style.PointerAlignment);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesBeforeTrailingComments",
                                         Style.SpacesBeforeTrailingComments);
    clang_vx::IoMapOptional<FormatStyle>(IO, "Cpp11BracedListStyle",
                                         Style.Cpp11BracedListStyle);
    clang_vx::IoMapOptional<FormatStyle>(IO, "Standard", Style.Standard);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IndentWidth", Style.IndentWidth);
    clang_vx::IoMapOptional<FormatStyle>(IO, "TabWidth", Style.TabWidth);
    clang_vx::IoMapOptional<FormatStyle>(IO, "UseTab", Style.UseTab);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BreakBeforeBraces",
                                         Style.BreakBeforeBraces);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesInParentheses",
                                         Style.SpacesInParentheses);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesInSquareBrackets",
                                         Style.SpacesInSquareBrackets);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesInAngles",
                                         Style.SpacesInAngles);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceInEmptyParentheses",
                                         Style.SpaceInEmptyParentheses);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesInCStyleCastParentheses",
                                         Style.SpacesInCStyleCastParentheses);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceAfterCStyleCast",
                                         Style.SpaceAfterCStyleCast);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesInContainerLiterals",
                                         Style.SpacesInContainerLiterals);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceBeforeAssignmentOperators",
                                         Style.SpaceBeforeAssignmentOperators);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ContinuationIndentWidth",
                                         Style.ContinuationIndentWidth);
    clang_vx::IoMapOptional<FormatStyle>(IO, "CommentPragmas",
                                         Style.CommentPragmas);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ForEachMacros",
                                         Style.ForEachMacros);

    // For backward compatibility.
    if (!IO.outputting()) {
      IO.mapOptional("SpaceAfterControlStatementKeyword",
                     Style.SpaceBeforeParens);
      IO.mapOptional("PointerBindsToType", Style.PointerAlignment);
      IO.mapOptional("DerivePointerBinding", Style.DerivePointerAlignment);
    }
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceBeforeParens",
                                         Style.SpaceBeforeParens);
    clang_vx::IoMapOptional<FormatStyle>(IO, "DisableFormat",
                                         Style.DisableFormat);
  }
};

// Allows to read vector<FormatStyle> while keeping default values.
// IO.getContext() should contain a pointer to the FormatStyle structure, that
// will be used to get default values for missing keys.
// If the first element has no Language specified, it will be treated as the
// default one for the following elements.
template <> struct DocumentListTraits<std::vector<FormatStyle>> {
  static size_t size(IO &IO, std::vector<FormatStyle> &Seq) {
    return Seq.size();
  }
  static FormatStyle &element(IO &IO, std::vector<FormatStyle> &Seq,
                              size_t Index) {
    if (Index >= Seq.size()) {
      assert(Index == Seq.size());
      FormatStyle Template;
      if (Seq.size() > 0 && Seq[0].Language == FormatStyle::LK_None) {
        Template = Seq[0];
      } else {
        Template = *((const FormatStyle *)IO.getContext());
        Template.Language = FormatStyle::LK_None;
      }
      Seq.resize(Index + 1, Template);
    }
    return Seq[Index];
  }
};
} // namespace yaml
} // namespace llvm

namespace clang_v3_6 {

const std::error_category &getParseCategory() {
  static ParseErrorCategory C;
  return C;
}
std::error_code make_error_code(ParseError e) {
  return std::error_code(static_cast<int>(e), getParseCategory());
}

const char *ParseErrorCategory::name() const noexcept {
  return "clang-format.parse_error";
}

std::string ParseErrorCategory::message(int EV) const {
  switch (static_cast<ParseError>(EV)) {
  case ParseError::Success:
    return "Success";
  case ParseError::Error:
    return "Invalid argument";
  case ParseError::Unsuitable:
    return "Unsuitable";
  }
  llvm_unreachable("unexpected parse error");
}

FormatStyle getLLVMStyle() {
  FormatStyle LLVMStyle;
  LLVMStyle.Language = FormatStyle::LK_Cpp;
  LLVMStyle.AccessModifierOffset = -2;
  LLVMStyle.AlignEscapedNewlinesLeft = false;
  LLVMStyle.AlignAfterOpenBracket = true;
  LLVMStyle.AlignOperands = true;
  LLVMStyle.AlignTrailingComments = true;
  LLVMStyle.AllowAllParametersOfDeclarationOnNextLine = true;
  LLVMStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_All;
  LLVMStyle.AllowShortBlocksOnASingleLine = false;
  LLVMStyle.AllowShortCaseLabelsOnASingleLine = false;
  LLVMStyle.AllowShortIfStatementsOnASingleLine = false;
  LLVMStyle.AllowShortLoopsOnASingleLine = false;
  LLVMStyle.AlwaysBreakAfterDefinitionReturnType = false;
  LLVMStyle.AlwaysBreakBeforeMultilineStrings = false;
  LLVMStyle.AlwaysBreakTemplateDeclarations = false;
  LLVMStyle.BinPackParameters = true;
  LLVMStyle.BinPackArguments = true;
  LLVMStyle.BreakBeforeBinaryOperators = FormatStyle::BOS_None;
  LLVMStyle.BreakBeforeTernaryOperators = true;
  LLVMStyle.BreakBeforeBraces = FormatStyle::BS_Attach;
  LLVMStyle.BreakConstructorInitializersBeforeComma = false;
  LLVMStyle.ColumnLimit = 80;
  LLVMStyle.CommentPragmas = "^ IWYU pragma:";
  LLVMStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
  LLVMStyle.ConstructorInitializerIndentWidth = 4;
  LLVMStyle.ContinuationIndentWidth = 4;
  LLVMStyle.Cpp11BracedListStyle = true;
  LLVMStyle.DerivePointerAlignment = false;
  LLVMStyle.ExperimentalAutoDetectBinPacking = false;
  LLVMStyle.ForEachMacros.push_back("foreach");
  LLVMStyle.ForEachMacros.push_back("Q_FOREACH");
  LLVMStyle.ForEachMacros.push_back("BOOST_FOREACH");
  LLVMStyle.IndentCaseLabels = false;
  LLVMStyle.IndentWrappedFunctionNames = false;
  LLVMStyle.IndentWidth = 2;
  LLVMStyle.TabWidth = 8;
  LLVMStyle.MaxEmptyLinesToKeep = 1;
  LLVMStyle.KeepEmptyLinesAtTheStartOfBlocks = true;
  LLVMStyle.NamespaceIndentation = FormatStyle::NI_None;
  LLVMStyle.ObjCBlockIndentWidth = 2;
  LLVMStyle.ObjCSpaceAfterProperty = false;
  LLVMStyle.ObjCSpaceBeforeProtocolList = true;
  LLVMStyle.PointerAlignment = FormatStyle::PAS_Right;
  LLVMStyle.SpacesBeforeTrailingComments = 1;
  LLVMStyle.Standard = FormatStyle::LS_Cpp11;
  LLVMStyle.UseTab = FormatStyle::UT_Never;
  LLVMStyle.SpacesInParentheses = false;
  LLVMStyle.SpacesInSquareBrackets = false;
  LLVMStyle.SpaceInEmptyParentheses = false;
  LLVMStyle.SpacesInContainerLiterals = true;
  LLVMStyle.SpacesInCStyleCastParentheses = false;
  LLVMStyle.SpaceAfterCStyleCast = false;
  LLVMStyle.SpaceBeforeParens = FormatStyle::SBPO_ControlStatements;
  LLVMStyle.SpaceBeforeAssignmentOperators = true;
  LLVMStyle.SpacesInAngles = false;

  LLVMStyle.PenaltyBreakComment = 300;
  LLVMStyle.PenaltyBreakFirstLessLess = 120;
  LLVMStyle.PenaltyBreakString = 1000;
  LLVMStyle.PenaltyExcessCharacter = 1000000;
  LLVMStyle.PenaltyReturnTypeOnItsOwnLine = 60;
  LLVMStyle.PenaltyBreakBeforeFirstCallParameter = 19;

  LLVMStyle.DisableFormat = false;

  return LLVMStyle;
}

FormatStyle getGoogleStyle(FormatStyle::LanguageKind Language) {
  FormatStyle GoogleStyle = getLLVMStyle();
  GoogleStyle.Language = Language;

  GoogleStyle.AccessModifierOffset = -1;
  GoogleStyle.AlignEscapedNewlinesLeft = true;
  GoogleStyle.AllowShortIfStatementsOnASingleLine = true;
  GoogleStyle.AllowShortLoopsOnASingleLine = true;
  GoogleStyle.AlwaysBreakBeforeMultilineStrings = true;
  GoogleStyle.AlwaysBreakTemplateDeclarations = true;
  GoogleStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
  GoogleStyle.DerivePointerAlignment = true;
  GoogleStyle.IndentCaseLabels = true;
  GoogleStyle.KeepEmptyLinesAtTheStartOfBlocks = false;
  GoogleStyle.ObjCSpaceAfterProperty = false;
  GoogleStyle.ObjCSpaceBeforeProtocolList = false;
  GoogleStyle.PointerAlignment = FormatStyle::PAS_Left;
  GoogleStyle.SpacesBeforeTrailingComments = 2;
  GoogleStyle.Standard = FormatStyle::LS_Auto;

  GoogleStyle.PenaltyReturnTypeOnItsOwnLine = 200;
  GoogleStyle.PenaltyBreakBeforeFirstCallParameter = 1;

  if (Language == FormatStyle::LK_Java) {
    GoogleStyle.AlignAfterOpenBracket = false;
    GoogleStyle.AlignOperands = false;
    GoogleStyle.AlignTrailingComments = false;
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Empty;
    GoogleStyle.AllowShortIfStatementsOnASingleLine = false;
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
    GoogleStyle.BreakBeforeBinaryOperators = FormatStyle::BOS_NonAssignment;
    GoogleStyle.ColumnLimit = 100;
    GoogleStyle.SpaceAfterCStyleCast = true;
    GoogleStyle.SpacesBeforeTrailingComments = 1;
  } else if (Language == FormatStyle::LK_JavaScript) {
    GoogleStyle.BreakBeforeTernaryOperators = false;
    GoogleStyle.MaxEmptyLinesToKeep = 3;
    GoogleStyle.SpacesInContainerLiterals = false;
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Inline;
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
  } else if (Language == FormatStyle::LK_Proto) {
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_None;
    GoogleStyle.SpacesInContainerLiterals = false;
  }

  return GoogleStyle;
}

FormatStyle getChromiumStyle(FormatStyle::LanguageKind Language) {
  FormatStyle ChromiumStyle = getGoogleStyle(Language);
  if (Language == FormatStyle::LK_Java) {
    ChromiumStyle.AllowShortIfStatementsOnASingleLine = true;
    ChromiumStyle.IndentWidth = 4;
    ChromiumStyle.ContinuationIndentWidth = 8;
  } else {
    ChromiumStyle.AllowAllParametersOfDeclarationOnNextLine = false;
    ChromiumStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Inline;
    ChromiumStyle.AllowShortIfStatementsOnASingleLine = false;
    ChromiumStyle.AllowShortLoopsOnASingleLine = false;
    ChromiumStyle.BinPackParameters = false;
    ChromiumStyle.DerivePointerAlignment = false;
  }
  return ChromiumStyle;
}

FormatStyle getMozillaStyle() {
  FormatStyle MozillaStyle = getLLVMStyle();
  MozillaStyle.AllowAllParametersOfDeclarationOnNextLine = false;
  MozillaStyle.Cpp11BracedListStyle = false;
  MozillaStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
  MozillaStyle.DerivePointerAlignment = true;
  MozillaStyle.IndentCaseLabels = true;
  MozillaStyle.ObjCSpaceAfterProperty = true;
  MozillaStyle.ObjCSpaceBeforeProtocolList = false;
  MozillaStyle.PenaltyReturnTypeOnItsOwnLine = 200;
  MozillaStyle.PointerAlignment = FormatStyle::PAS_Left;
  MozillaStyle.Standard = FormatStyle::LS_Cpp03;
  return MozillaStyle;
}

FormatStyle getWebKitStyle() {
  FormatStyle Style = getLLVMStyle();
  Style.AccessModifierOffset = -4;
  Style.AlignAfterOpenBracket = false;
  Style.AlignOperands = false;
  Style.AlignTrailingComments = false;
  Style.BreakBeforeBinaryOperators = FormatStyle::BOS_All;
  Style.BreakBeforeBraces = FormatStyle::BS_Stroustrup;
  Style.BreakConstructorInitializersBeforeComma = true;
  Style.Cpp11BracedListStyle = false;
  Style.ColumnLimit = 0;
  Style.IndentWidth = 4;
  Style.NamespaceIndentation = FormatStyle::NI_Inner;
  Style.ObjCBlockIndentWidth = 4;
  Style.ObjCSpaceAfterProperty = true;
  Style.PointerAlignment = FormatStyle::PAS_Left;
  Style.Standard = FormatStyle::LS_Cpp03;
  return Style;
}

FormatStyle getGNUStyle() {
  FormatStyle Style = getLLVMStyle();
  Style.AlwaysBreakAfterDefinitionReturnType = true;
  Style.BreakBeforeBinaryOperators = FormatStyle::BOS_All;
  Style.BreakBeforeBraces = FormatStyle::BS_GNU;
  Style.BreakBeforeTernaryOperators = true;
  Style.Cpp11BracedListStyle = false;
  Style.ColumnLimit = 79;
  Style.SpaceBeforeParens = FormatStyle::SBPO_Always;
  Style.Standard = FormatStyle::LS_Cpp03;
  return Style;
}

FormatStyle getNoStyle() {
  FormatStyle NoStyle = getLLVMStyle();
  NoStyle.DisableFormat = true;
  return NoStyle;
}

bool getPredefinedStyle(llvm::StringRef Name,
                        FormatStyle::LanguageKind Language,
                        FormatStyle *Style) {
  if (Name.equals_insensitive("llvm")) {
    *Style = getLLVMStyle();
  } else if (Name.equals_insensitive("chromium")) {
    *Style = getChromiumStyle(Language);
  } else if (Name.equals_insensitive("mozilla")) {
    *Style = getMozillaStyle();
  } else if (Name.equals_insensitive("google")) {
    *Style = getGoogleStyle(Language);
  } else if (Name.equals_insensitive("webkit")) {
    *Style = getWebKitStyle();
  } else if (Name.equals_insensitive("gnu")) {
    *Style = getGNUStyle();
  } else if (Name.equals_insensitive("none")) {
    *Style = getNoStyle();
  } else {
    return false;
  }

  Style->Language = Language;
  return true;
}

std::vector<std::string> getStyleNames() {
  return {"chromium", "gnu", "google", "llvm", "mozilla", "none", "webkit"};
}

std::error_code parseConfiguration(const std::string &Text,
                                   FormatStyle *Style) {
  assert(Style);
  FormatStyle::LanguageKind Language = Style->Language;
  assert(Language != FormatStyle::LK_None);

  std::vector<FormatStyle> Styles;
  llvm::yaml::Input Input(Text);
  // DocumentListTraits<vector<FormatStyle>> uses the context to get default
  // values for the fields, keys for which are missing from the configuration.
  // Mapping also uses the context to get the language to find the correct
  // base style.
  Input.setContext(Style);
  Input >> Styles;
  if (Input.error())
    return Input.error();

  for (unsigned i = 0; i < Styles.size(); ++i) {
    // Ensures that only the first configuration can skip the Language option.
    if (Styles[i].Language == FormatStyle::LK_None && i != 0)
      return make_error_code(ParseError::Error);
    // Ensure that each language is configured at most once.
    for (unsigned j = 0; j < i; ++j) {
      if (Styles[i].Language == Styles[j].Language) {
        LLVM_DEBUG(llvm::dbgs()
                   << "Duplicate languages in the config file on positions "
                   << j << " and " << i << "\n");
        return make_error_code(ParseError::Error);
      }
    }
  }
  // Look for a suitable configuration starting from the end, so we can
  // find the configuration for the specific language first, and the default
  // configuration (which can only be at slot 0) after it.
  for (int i = Styles.size() - 1; i >= 0; --i) {
    if (Styles[i].Language == Language ||
        Styles[i].Language == FormatStyle::LK_None) {
      *Style = Styles[i];
      Style->Language = Language;
      return make_error_code(ParseError::Success);
    }
  }
  return make_error_code(ParseError::Unsuitable);
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
  std::optional<clang_vx::OutputDiffOnly<FormatStyle>> ctxt;
  if (!getPredefinedStyle(DefaultStyleName, Style.Language, &DefaultStyle)) {
    ctxt.emplace(nullptr, NonConstStyle, false);
  } else {
    ctxt.emplace(&DefaultStyle, NonConstStyle, SkipSameValue);
  }
  llvm::yaml::Output Output(Stream, &*ctxt);
  Output << NonConstStyle;

  return Stream.str();
}

} // namespace clang_v3_6
