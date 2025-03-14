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

using clang_v4::FormatStyle;

LLVM_YAML_IS_SEQUENCE_VECTOR(clang_v4::FormatStyle::IncludeCategory)

namespace llvm {
namespace yaml {
template <> struct ScalarEnumerationTraits<FormatStyle::LanguageKind> {
  static void enumeration(IO &IO, FormatStyle::LanguageKind &Value) {
    IO.enumCase(Value, "Cpp", FormatStyle::LK_Cpp);
    IO.enumCase(Value, "Java", FormatStyle::LK_Java);
    IO.enumCase(Value, "JavaScript", FormatStyle::LK_JavaScript);
    IO.enumCase(Value, "ObjC", FormatStyle::LK_ObjC);
    IO.enumCase(Value, "Proto", FormatStyle::LK_Proto);
    IO.enumCase(Value, "TableGen", FormatStyle::LK_TableGen);
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
    IO.enumCase(Value, "ForContinuationAndIndentation",
                FormatStyle::UT_ForContinuationAndIndentation);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::JavaScriptQuoteStyle> {
  static void enumeration(IO &IO, FormatStyle::JavaScriptQuoteStyle &Value) {
    IO.enumCase(Value, "Leave", FormatStyle::JSQS_Leave);
    IO.enumCase(Value, "Single", FormatStyle::JSQS_Single);
    IO.enumCase(Value, "Double", FormatStyle::JSQS_Double);
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
    IO.enumCase(Value, "Mozilla", FormatStyle::BS_Mozilla);
    IO.enumCase(Value, "Stroustrup", FormatStyle::BS_Stroustrup);
    IO.enumCase(Value, "Allman", FormatStyle::BS_Allman);
    IO.enumCase(Value, "GNU", FormatStyle::BS_GNU);
    IO.enumCase(Value, "WebKit", FormatStyle::BS_WebKit);
    IO.enumCase(Value, "Custom", FormatStyle::BS_Custom);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::ReturnTypeBreakingStyle> {
  static void enumeration(IO &IO, FormatStyle::ReturnTypeBreakingStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::RTBS_None);
    IO.enumCase(Value, "All", FormatStyle::RTBS_All);
    IO.enumCase(Value, "TopLevel", FormatStyle::RTBS_TopLevel);
    IO.enumCase(Value, "TopLevelDefinitions",
                FormatStyle::RTBS_TopLevelDefinitions);
    IO.enumCase(Value, "AllDefinitions", FormatStyle::RTBS_AllDefinitions);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::DefinitionReturnTypeBreakingStyle> {
  static void
  enumeration(IO &IO, FormatStyle::DefinitionReturnTypeBreakingStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::DRTBS_None);
    IO.enumCase(Value, "All", FormatStyle::DRTBS_All);
    IO.enumCase(Value, "TopLevel", FormatStyle::DRTBS_TopLevel);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::DRTBS_None);
    IO.enumCase(Value, "true", FormatStyle::DRTBS_All);
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

template <> struct ScalarEnumerationTraits<FormatStyle::BracketAlignmentStyle> {
  static void enumeration(IO &IO, FormatStyle::BracketAlignmentStyle &Value) {
    IO.enumCase(Value, "Align", FormatStyle::BAS_Align);
    IO.enumCase(Value, "DontAlign", FormatStyle::BAS_DontAlign);
    IO.enumCase(Value, "AlwaysBreak", FormatStyle::BAS_AlwaysBreak);

    // For backward compatibility.
    IO.enumCase(Value, "true", FormatStyle::BAS_Align);
    IO.enumCase(Value, "false", FormatStyle::BAS_DontAlign);
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
      clang_vx::OutputDiffOnly<clang_v4::FormatStyle> &out =
          *static_cast<clang_vx::OutputDiffOnly<clang_v4::FormatStyle> *>(
              IO.getContext());
      if (out.getDefaultStyle()) {
        for (const std::string &StyleName : clang_v4::getStyleNames()) {
          clang_v4::FormatStyle PredefinedStyle;
          if (clang_v4::getPredefinedStyle(StyleName, Style.Language,
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
      for (const std::string &StyleName : clang_v4::getStyleNames()) {
        clang_v4::FormatStyle PredefinedStyle;
        if (clang_v4::getPredefinedStyle(StyleName, Style.Language,
                                         &PredefinedStyle) &&
            Style == PredefinedStyle) {
          BasedOnStyle = StyleName;
          break;
        }
      }
    }

    // For backward compatibility.
    if (!IO.outputting()) {
      IO.mapOptional("DerivePointerBinding", Style.DerivePointerAlignment);
      IO.mapOptional("IndentFunctionDeclarationAfterType",
                     Style.IndentWrappedFunctionNames);
      IO.mapOptional("PointerBindsToType", Style.PointerAlignment);
      IO.mapOptional("SpaceAfterControlStatementKeyword",
                     Style.SpaceBeforeParens);
    }

    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "AccessModifierOffset",
                                                   Style.AccessModifierOffset);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "AlignAfterOpenBracket",
                                                   Style.AlignAfterOpenBracket);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AlignConsecutiveAssignments", Style.AlignConsecutiveAssignments);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AlignConsecutiveDeclarations", Style.AlignConsecutiveDeclarations);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AlignEscapedNewlinesLeft", Style.AlignEscapedNewlinesLeft);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "AlignOperands",
                                                   Style.AlignOperands);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "AlignTrailingComments",
                                                   Style.AlignTrailingComments);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AllowAllParametersOfDeclarationOnNextLine",
        Style.AllowAllParametersOfDeclarationOnNextLine);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AllowShortBlocksOnASingleLine",
        Style.AllowShortBlocksOnASingleLine);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AllowShortCaseLabelsOnASingleLine",
        Style.AllowShortCaseLabelsOnASingleLine);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AllowShortFunctionsOnASingleLine",
        Style.AllowShortFunctionsOnASingleLine);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AllowShortIfStatementsOnASingleLine",
        Style.AllowShortIfStatementsOnASingleLine);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AllowShortLoopsOnASingleLine", Style.AllowShortLoopsOnASingleLine);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AlwaysBreakAfterDefinitionReturnType",
        Style.AlwaysBreakAfterDefinitionReturnType);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AlwaysBreakAfterReturnType", Style.AlwaysBreakAfterReturnType);
    // If AlwaysBreakAfterDefinitionReturnType was specified but
    // AlwaysBreakAfterReturnType was not, initialize the latter from the
    // former for backwards compatibility.
    if (Style.AlwaysBreakAfterDefinitionReturnType != FormatStyle::DRTBS_None &&
        Style.AlwaysBreakAfterReturnType == FormatStyle::RTBS_None) {
      if (Style.AlwaysBreakAfterDefinitionReturnType == FormatStyle::DRTBS_All)
        Style.AlwaysBreakAfterReturnType = FormatStyle::RTBS_AllDefinitions;
      else if (Style.AlwaysBreakAfterDefinitionReturnType ==
               FormatStyle::DRTBS_TopLevel)
        Style.AlwaysBreakAfterReturnType =
            FormatStyle::RTBS_TopLevelDefinitions;
    }

    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AlwaysBreakBeforeMultilineStrings",
        Style.AlwaysBreakBeforeMultilineStrings);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AlwaysBreakTemplateDeclarations",
        Style.AlwaysBreakTemplateDeclarations);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "BinPackArguments",
                                                   Style.BinPackArguments);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "BinPackParameters",
                                                   Style.BinPackParameters);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "BraceWrapping",
                                                   Style.BraceWrapping);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "BreakBeforeBinaryOperators", Style.BreakBeforeBinaryOperators);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "BreakBeforeBraces",
                                                   Style.BreakBeforeBraces);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "BreakBeforeTernaryOperators", Style.BreakBeforeTernaryOperators);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "BreakConstructorInitializersBeforeComma",
        Style.BreakConstructorInitializersBeforeComma);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "BreakAfterJavaFieldAnnotations",
        Style.BreakAfterJavaFieldAnnotations);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "BreakStringLiterals",
                                                   Style.BreakStringLiterals);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "ColumnLimit",
                                                   Style.ColumnLimit);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "CommentPragmas",
                                                   Style.CommentPragmas);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "ConstructorInitializerAllOnOneLineOrOnePerLine",
        Style.ConstructorInitializerAllOnOneLineOrOnePerLine);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "ConstructorInitializerIndentWidth",
        Style.ConstructorInitializerIndentWidth);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "ContinuationIndentWidth", Style.ContinuationIndentWidth);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "Cpp11BracedListStyle",
                                                   Style.Cpp11BracedListStyle);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "DerivePointerAlignment", Style.DerivePointerAlignment);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "DisableFormat",
                                                   Style.DisableFormat);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "ExperimentalAutoDetectBinPacking",
        Style.ExperimentalAutoDetectBinPacking);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "ForEachMacros",
                                                   Style.ForEachMacros);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "IncludeCategories",
                                                   Style.IncludeCategories);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "IncludeIsMainRegex",
                                                   Style.IncludeIsMainRegex);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "IndentCaseLabels",
                                                   Style.IndentCaseLabels);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "IndentWidth",
                                                   Style.IndentWidth);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "IndentWrappedFunctionNames", Style.IndentWrappedFunctionNames);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "JavaScriptQuotes",
                                                   Style.JavaScriptQuotes);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "JavaScriptWrapImports",
                                                   Style.JavaScriptWrapImports);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "KeepEmptyLinesAtTheStartOfBlocks",
        Style.KeepEmptyLinesAtTheStartOfBlocks);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "MacroBlockBegin",
                                                   Style.MacroBlockBegin);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "MacroBlockEnd",
                                                   Style.MacroBlockEnd);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "MaxEmptyLinesToKeep",
                                                   Style.MaxEmptyLinesToKeep);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "NamespaceIndentation",
                                                   Style.NamespaceIndentation);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "ObjCBlockIndentWidth",
                                                   Style.ObjCBlockIndentWidth);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "ObjCSpaceAfterProperty", Style.ObjCSpaceAfterProperty);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "ObjCSpaceBeforeProtocolList", Style.ObjCSpaceBeforeProtocolList);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "PenaltyBreakBeforeFirstCallParameter",
        Style.PenaltyBreakBeforeFirstCallParameter);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "PenaltyBreakComment",
                                                   Style.PenaltyBreakComment);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "PenaltyBreakFirstLessLess", Style.PenaltyBreakFirstLessLess);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "PenaltyBreakString",
                                                   Style.PenaltyBreakString);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "PenaltyExcessCharacter", Style.PenaltyExcessCharacter);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "PenaltyReturnTypeOnItsOwnLine",
        Style.PenaltyReturnTypeOnItsOwnLine);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "PointerAlignment",
                                                   Style.PointerAlignment);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "ReflowComments",
                                                   Style.ReflowComments);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "SortIncludes",
                                                   Style.SortIncludes);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "SpaceAfterCStyleCast",
                                                   Style.SpaceAfterCStyleCast);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "SpaceAfterTemplateKeyword", Style.SpaceAfterTemplateKeyword);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "SpaceBeforeAssignmentOperators",
        Style.SpaceBeforeAssignmentOperators);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "SpaceBeforeParens",
                                                   Style.SpaceBeforeParens);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "SpaceInEmptyParentheses", Style.SpaceInEmptyParentheses);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "SpacesBeforeTrailingComments", Style.SpacesBeforeTrailingComments);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "SpacesInAngles",
                                                   Style.SpacesInAngles);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "SpacesInContainerLiterals", Style.SpacesInContainerLiterals);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "SpacesInCStyleCastParentheses",
        Style.SpacesInCStyleCastParentheses);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "SpacesInParentheses",
                                                   Style.SpacesInParentheses);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "SpacesInSquareBrackets", Style.SpacesInSquareBrackets);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "Standard",
                                                   Style.Standard);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "TabWidth",
                                                   Style.TabWidth);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "UseTab", Style.UseTab);
  }
};

template <> struct MappingTraits<FormatStyle::BraceWrappingFlags> {
  static void mapping(IO &IO, FormatStyle::BraceWrappingFlags &Wrapping) {
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "AfterClass",
                                                   Wrapping.AfterClass);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AfterControlStatement", Wrapping.AfterControlStatement);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "AfterEnum",
                                                   Wrapping.AfterEnum);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "AfterFunction",
                                                   Wrapping.AfterFunction);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "AfterNamespace",
                                                   Wrapping.AfterNamespace);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(
        IO, "AfterObjCDeclaration", Wrapping.AfterObjCDeclaration);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "AfterStruct",
                                                   Wrapping.AfterStruct);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "AfterUnion",
                                                   Wrapping.AfterUnion);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "BeforeCatch",
                                                   Wrapping.BeforeCatch);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "BeforeElse",
                                                   Wrapping.BeforeElse);
    clang_vx::IoMapOptional<clang_v4::FormatStyle>(IO, "IndentBraces",
                                                   Wrapping.IndentBraces);
  }
};

template <> struct MappingTraits<FormatStyle::IncludeCategory> {
  static void mapping(IO &IO, FormatStyle::IncludeCategory &Category) {
    IO.mapOptional("Regex", Category.Regex);
    IO.mapOptional("Priority", Category.Priority);
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

namespace clang_v4 {

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

static FormatStyle expandPresets(const FormatStyle &Style) {
  if (Style.BreakBeforeBraces == FormatStyle::BS_Custom)
    return Style;
  FormatStyle Expanded = Style;
  Expanded.BraceWrapping = {false, false, false, false, false, false,
                            false, false, false, false, false};
  switch (Style.BreakBeforeBraces) {
  case FormatStyle::BS_Linux:
    Expanded.BraceWrapping.AfterClass = true;
    Expanded.BraceWrapping.AfterFunction = true;
    Expanded.BraceWrapping.AfterNamespace = true;
    break;
  case FormatStyle::BS_Mozilla:
    Expanded.BraceWrapping.AfterClass = true;
    Expanded.BraceWrapping.AfterEnum = true;
    Expanded.BraceWrapping.AfterFunction = true;
    Expanded.BraceWrapping.AfterStruct = true;
    Expanded.BraceWrapping.AfterUnion = true;
    break;
  case FormatStyle::BS_Stroustrup:
    Expanded.BraceWrapping.AfterFunction = true;
    Expanded.BraceWrapping.BeforeCatch = true;
    Expanded.BraceWrapping.BeforeElse = true;
    break;
  case FormatStyle::BS_Allman:
    Expanded.BraceWrapping.AfterClass = true;
    Expanded.BraceWrapping.AfterControlStatement = true;
    Expanded.BraceWrapping.AfterEnum = true;
    Expanded.BraceWrapping.AfterFunction = true;
    Expanded.BraceWrapping.AfterNamespace = true;
    Expanded.BraceWrapping.AfterObjCDeclaration = true;
    Expanded.BraceWrapping.AfterStruct = true;
    Expanded.BraceWrapping.BeforeCatch = true;
    Expanded.BraceWrapping.BeforeElse = true;
    break;
  case FormatStyle::BS_GNU:
    Expanded.BraceWrapping = {true, true, true, true, true, true,
                              true, true, true, true, true};
    break;
  case FormatStyle::BS_WebKit:
    Expanded.BraceWrapping.AfterFunction = true;
    break;
  default:
    break;
  }
  return Expanded;
}

FormatStyle getLLVMStyle() {
  FormatStyle LLVMStyle;
  LLVMStyle.Language = FormatStyle::LK_Cpp;
  LLVMStyle.AccessModifierOffset = -2;
  LLVMStyle.AlignEscapedNewlinesLeft = false;
  LLVMStyle.AlignAfterOpenBracket = FormatStyle::BAS_Align;
  LLVMStyle.AlignOperands = true;
  LLVMStyle.AlignTrailingComments = true;
  LLVMStyle.AlignConsecutiveAssignments = false;
  LLVMStyle.AlignConsecutiveDeclarations = false;
  LLVMStyle.AllowAllParametersOfDeclarationOnNextLine = true;
  LLVMStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_All;
  LLVMStyle.AllowShortBlocksOnASingleLine = false;
  LLVMStyle.AllowShortCaseLabelsOnASingleLine = false;
  LLVMStyle.AllowShortIfStatementsOnASingleLine = false;
  LLVMStyle.AllowShortLoopsOnASingleLine = false;
  LLVMStyle.AlwaysBreakAfterReturnType = FormatStyle::RTBS_None;
  LLVMStyle.AlwaysBreakAfterDefinitionReturnType = FormatStyle::DRTBS_None;
  LLVMStyle.AlwaysBreakBeforeMultilineStrings = false;
  LLVMStyle.AlwaysBreakTemplateDeclarations = false;
  LLVMStyle.BinPackParameters = true;
  LLVMStyle.BinPackArguments = true;
  LLVMStyle.BreakBeforeBinaryOperators = FormatStyle::BOS_None;
  LLVMStyle.BreakBeforeTernaryOperators = true;
  LLVMStyle.BreakBeforeBraces = FormatStyle::BS_Attach;
  LLVMStyle.BraceWrapping = {false, false, false, false, false, false,
                             false, false, false, false, false};
  LLVMStyle.BreakAfterJavaFieldAnnotations = false;
  LLVMStyle.BreakConstructorInitializersBeforeComma = false;
  LLVMStyle.BreakStringLiterals = true;
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
  LLVMStyle.IncludeCategories = {{"^\"(llvm|llvm-c|clang|clang-c)/", 2},
                                 {"^(<|\"(gtest|isl|json)/)", 3},
                                 {".*", 1}};
  LLVMStyle.IncludeIsMainRegex = "$";
  LLVMStyle.IndentCaseLabels = false;
  LLVMStyle.IndentWrappedFunctionNames = false;
  LLVMStyle.IndentWidth = 2;
  LLVMStyle.JavaScriptQuotes = FormatStyle::JSQS_Leave;
  LLVMStyle.JavaScriptWrapImports = true;
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
  LLVMStyle.JavaScriptQuotes = FormatStyle::JSQS_Leave;
  LLVMStyle.ReflowComments = true;
  LLVMStyle.SpacesInParentheses = false;
  LLVMStyle.SpacesInSquareBrackets = false;
  LLVMStyle.SpaceInEmptyParentheses = false;
  LLVMStyle.SpacesInContainerLiterals = true;
  LLVMStyle.SpacesInCStyleCastParentheses = false;
  LLVMStyle.SpaceAfterCStyleCast = false;
  LLVMStyle.SpaceAfterTemplateKeyword = true;
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
  LLVMStyle.SortIncludes = true;

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
  GoogleStyle.IncludeCategories = {{"^<.*\\.h>", 1}, {"^<.*", 2}, {".*", 3}};
  GoogleStyle.IncludeIsMainRegex = "([-_](test|unittest))?$";
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
    GoogleStyle.AlignAfterOpenBracket = FormatStyle::BAS_DontAlign;
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
    GoogleStyle.AlignAfterOpenBracket = FormatStyle::BAS_AlwaysBreak;
    GoogleStyle.AlignOperands = false;
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Empty;
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
    GoogleStyle.BreakBeforeTernaryOperators = false;
    GoogleStyle.CommentPragmas =
        "(taze:|@(export|requirecss|return|returns|see|visibility)) ";
    GoogleStyle.MaxEmptyLinesToKeep = 3;
    GoogleStyle.NamespaceIndentation = FormatStyle::NI_All;
    GoogleStyle.SpacesInContainerLiterals = false;
    GoogleStyle.JavaScriptQuotes = FormatStyle::JSQS_Single;
    GoogleStyle.JavaScriptWrapImports = false;
  } else if (Language == FormatStyle::LK_Proto) {
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_None;
    GoogleStyle.SpacesInContainerLiterals = false;
  } else if (Language == FormatStyle::LK_ObjC) {
    GoogleStyle.ColumnLimit = 100;
  }

  return GoogleStyle;
}

FormatStyle getChromiumStyle(FormatStyle::LanguageKind Language) {
  FormatStyle ChromiumStyle = getGoogleStyle(Language);
  if (Language == FormatStyle::LK_Java) {
    ChromiumStyle.AllowShortIfStatementsOnASingleLine = true;
    ChromiumStyle.BreakAfterJavaFieldAnnotations = true;
    ChromiumStyle.ContinuationIndentWidth = 8;
    ChromiumStyle.IndentWidth = 4;
  } else if (Language == FormatStyle::LK_JavaScript) {
    ChromiumStyle.AllowShortIfStatementsOnASingleLine = false;
    ChromiumStyle.AllowShortLoopsOnASingleLine = false;
  } else {
    ChromiumStyle.AllowAllParametersOfDeclarationOnNextLine = false;
    ChromiumStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Inline;
    ChromiumStyle.AllowShortIfStatementsOnASingleLine = false;
    ChromiumStyle.AllowShortLoopsOnASingleLine = false;
    ChromiumStyle.BinPackParameters = false;
    ChromiumStyle.DerivePointerAlignment = false;
  }
  ChromiumStyle.SortIncludes = false;
  return ChromiumStyle;
}

FormatStyle getMozillaStyle() {
  FormatStyle MozillaStyle = getLLVMStyle();
  MozillaStyle.AllowAllParametersOfDeclarationOnNextLine = false;
  MozillaStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Inline;
  MozillaStyle.AlwaysBreakAfterReturnType = FormatStyle::RTBS_TopLevel;
  MozillaStyle.AlwaysBreakAfterDefinitionReturnType =
      FormatStyle::DRTBS_TopLevel;
  MozillaStyle.AlwaysBreakTemplateDeclarations = true;
  MozillaStyle.BinPackParameters = false;
  MozillaStyle.BinPackArguments = false;
  MozillaStyle.BreakBeforeBraces = FormatStyle::BS_Mozilla;
  MozillaStyle.BreakConstructorInitializersBeforeComma = true;
  MozillaStyle.ConstructorInitializerIndentWidth = 2;
  MozillaStyle.ContinuationIndentWidth = 2;
  MozillaStyle.Cpp11BracedListStyle = false;
  MozillaStyle.IndentCaseLabels = true;
  MozillaStyle.ObjCSpaceAfterProperty = true;
  MozillaStyle.ObjCSpaceBeforeProtocolList = false;
  MozillaStyle.PenaltyReturnTypeOnItsOwnLine = 200;
  MozillaStyle.PointerAlignment = FormatStyle::PAS_Left;
  MozillaStyle.SpaceAfterTemplateKeyword = false;
  return MozillaStyle;
}

FormatStyle getWebKitStyle() {
  FormatStyle Style = getLLVMStyle();
  Style.AccessModifierOffset = -4;
  Style.AlignAfterOpenBracket = FormatStyle::BAS_DontAlign;
  Style.AlignOperands = false;
  Style.AlignTrailingComments = false;
  Style.BreakBeforeBinaryOperators = FormatStyle::BOS_All;
  Style.BreakBeforeBraces = FormatStyle::BS_WebKit;
  Style.BreakConstructorInitializersBeforeComma = true;
  Style.Cpp11BracedListStyle = false;
  Style.ColumnLimit = 0;
  Style.IndentWidth = 4;
  Style.NamespaceIndentation = FormatStyle::NI_Inner;
  Style.ObjCBlockIndentWidth = 4;
  Style.ObjCSpaceAfterProperty = true;
  Style.PointerAlignment = FormatStyle::PAS_Left;
  return Style;
}

FormatStyle getGNUStyle() {
  FormatStyle Style = getLLVMStyle();
  Style.AlwaysBreakAfterDefinitionReturnType = FormatStyle::DRTBS_All;
  Style.AlwaysBreakAfterReturnType = FormatStyle::RTBS_AllDefinitions;
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
  NoStyle.SortIncludes = false;
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
  FormatStyle NonConstStyle = expandPresets(Style);
  std::optional<clang_vx::OutputDiffOnly<FormatStyle>> ctxt;
  if (!SkipSameValue ||
      !getPredefinedStyle(DefaultStyleName, Style.Language, &DefaultStyle)) {
    ctxt.emplace(nullptr, NonConstStyle, false);
  } else {
    ctxt.emplace(&DefaultStyle, NonConstStyle, SkipSameValue);
  }
  llvm::yaml::Output Output(Stream, &*ctxt);
  Output << NonConstStyle;

  return Stream.str();
}

} // namespace clang_v4
