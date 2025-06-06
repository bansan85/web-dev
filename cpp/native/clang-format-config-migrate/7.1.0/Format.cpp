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
/// This file implements functions declared in Format.h. This will be
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

using clang_v7::FormatStyle;

namespace prec = clang::prec;

LLVM_YAML_IS_SEQUENCE_VECTOR(FormatStyle::RawStringFormat)

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
    IO.enumCase(Value, "TextProto", FormatStyle::LK_TextProto);
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
    IO.enumCase(Value, "InlineOnly", FormatStyle::SFS_InlineOnly);
    IO.enumCase(Value, "Empty", FormatStyle::SFS_Empty);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::BinPackStyle> {
  static void enumeration(IO &IO, FormatStyle::BinPackStyle &Value) {
    IO.enumCase(Value, "Auto", FormatStyle::BPS_Auto);
    IO.enumCase(Value, "Always", FormatStyle::BPS_Always);
    IO.enumCase(Value, "Never", FormatStyle::BPS_Never);
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
struct ScalarEnumerationTraits<FormatStyle::BreakConstructorInitializersStyle> {
  static void
  enumeration(IO &IO, FormatStyle::BreakConstructorInitializersStyle &Value) {
    IO.enumCase(Value, "BeforeColon", FormatStyle::BCIS_BeforeColon);
    IO.enumCase(Value, "BeforeComma", FormatStyle::BCIS_BeforeComma);
    IO.enumCase(Value, "AfterColon", FormatStyle::BCIS_AfterColon);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::BreakInheritanceListStyle> {
  static void enumeration(IO &IO,
                          FormatStyle::BreakInheritanceListStyle &Value) {
    IO.enumCase(Value, "BeforeColon", FormatStyle::BILS_BeforeColon);
    IO.enumCase(Value, "BeforeComma", FormatStyle::BILS_BeforeComma);
    IO.enumCase(Value, "AfterColon", FormatStyle::BILS_AfterColon);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::PPDirectiveIndentStyle> {
  static void enumeration(IO &IO, FormatStyle::PPDirectiveIndentStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::PPDIS_None);
    IO.enumCase(Value, "AfterHash", FormatStyle::PPDIS_AfterHash);
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
struct ScalarEnumerationTraits<FormatStyle::BreakTemplateDeclarationsStyle> {
  static void enumeration(IO &IO,
                          FormatStyle::BreakTemplateDeclarationsStyle &Value) {
    IO.enumCase(Value, "No", FormatStyle::BTDS_No);
    IO.enumCase(Value, "MultiLine", FormatStyle::BTDS_MultiLine);
    IO.enumCase(Value, "Yes", FormatStyle::BTDS_Yes);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::BTDS_MultiLine);
    IO.enumCase(Value, "true", FormatStyle::BTDS_Yes);
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

template <>
struct ScalarEnumerationTraits<FormatStyle::EscapedNewlineAlignmentStyle> {
  static void enumeration(IO &IO,
                          FormatStyle::EscapedNewlineAlignmentStyle &Value) {
    IO.enumCase(Value, "DontAlign", FormatStyle::ENAS_DontAlign);
    IO.enumCase(Value, "Left", FormatStyle::ENAS_Left);
    IO.enumCase(Value, "Right", FormatStyle::ENAS_Right);

    // For backward compatibility.
    IO.enumCase(Value, "true", FormatStyle::ENAS_Left);
    IO.enumCase(Value, "false", FormatStyle::ENAS_Right);
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
        for (const std::string &StyleName : clang_v7::getStyleNames()) {
          FormatStyle PredefinedStyle;
          if (clang_v7::getPredefinedStyle(StyleName, Style.Language,
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
      for (const std::string &StyleName : clang_v7::getStyleNames()) {
        FormatStyle PredefinedStyle;
        if (clang_v7::getPredefinedStyle(StyleName, Style.Language,
                                         &PredefinedStyle) &&
            Style == PredefinedStyle) {
          BasedOnStyle = StyleName;
          break;
        }
      }
    }

    // For backward compatibility.
    if (!IO.outputting()) {
      IO.mapOptional("AlignEscapedNewlinesLeft", Style.AlignEscapedNewlines);
      IO.mapOptional("DerivePointerBinding", Style.DerivePointerAlignment);
      IO.mapOptional("IndentFunctionDeclarationAfterType",
                     Style.IndentWrappedFunctionNames);
      IO.mapOptional("PointerBindsToType", Style.PointerAlignment);
      IO.mapOptional("SpaceAfterControlStatementKeyword",
                     Style.SpaceBeforeParens);
    }

    clang_vx::IoMapOptional<FormatStyle>(IO, "AccessModifierOffset",
                                         Style.AccessModifierOffset);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlignAfterOpenBracket",
                                         Style.AlignAfterOpenBracket);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlignConsecutiveAssignments",
                                         Style.AlignConsecutiveAssignments);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlignConsecutiveDeclarations",
                                         Style.AlignConsecutiveDeclarations);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlignEscapedNewlines",
                                         Style.AlignEscapedNewlines);
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
        IO, "AllowShortFunctionsOnASingleLine",
        Style.AllowShortFunctionsOnASingleLine);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "AllowShortIfStatementsOnASingleLine",
        Style.AllowShortIfStatementsOnASingleLine);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AllowShortLoopsOnASingleLine",
                                         Style.AllowShortLoopsOnASingleLine);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "AlwaysBreakAfterDefinitionReturnType",
        Style.AlwaysBreakAfterDefinitionReturnType);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlwaysBreakAfterReturnType",
                                         Style.AlwaysBreakAfterReturnType);
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

    clang_vx::IoMapOptional<FormatStyle>(
        IO, "AlwaysBreakBeforeMultilineStrings",
        Style.AlwaysBreakBeforeMultilineStrings);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AlwaysBreakTemplateDeclarations",
                                         Style.AlwaysBreakTemplateDeclarations);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BinPackArguments",
                                         Style.BinPackArguments);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BinPackParameters",
                                         Style.BinPackParameters);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BraceWrapping",
                                         Style.BraceWrapping);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BreakBeforeBinaryOperators",
                                         Style.BreakBeforeBinaryOperators);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BreakBeforeBraces",
                                         Style.BreakBeforeBraces);

    bool BreakBeforeInheritanceComma = false;
    clang_vx::IoMapOptionalHardcodedValue<FormatStyle>(
        IO, "BreakBeforeInheritanceComma", BreakBeforeInheritanceComma);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BreakInheritanceList",
                                         Style.BreakInheritanceList);
    // If BreakBeforeInheritanceComma was specified but
    // BreakInheritance was not, initialize the latter from the
    // former for backwards compatibility.
    if (BreakBeforeInheritanceComma &&
        Style.BreakInheritanceList == FormatStyle::BILS_BeforeColon)
      Style.BreakInheritanceList = FormatStyle::BILS_BeforeComma;

    clang_vx::IoMapOptional<FormatStyle>(IO, "BreakBeforeTernaryOperators",
                                         Style.BreakBeforeTernaryOperators);

    bool BreakConstructorInitializersBeforeComma = false;
    clang_vx::IoMapOptionalHardcodedValue<FormatStyle>(
        IO, "BreakConstructorInitializersBeforeComma",
        BreakConstructorInitializersBeforeComma);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BreakConstructorInitializers",
                                         Style.BreakConstructorInitializers);
    // If BreakConstructorInitializersBeforeComma was specified but
    // BreakConstructorInitializers was not, initialize the latter from the
    // former for backwards compatibility.
    if (BreakConstructorInitializersBeforeComma &&
        Style.BreakConstructorInitializers == FormatStyle::BCIS_BeforeColon)
      Style.BreakConstructorInitializers = FormatStyle::BCIS_BeforeComma;

    clang_vx::IoMapOptional<FormatStyle>(IO, "BreakAfterJavaFieldAnnotations",
                                         Style.BreakAfterJavaFieldAnnotations);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BreakStringLiterals",
                                         Style.BreakStringLiterals);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ColumnLimit", Style.ColumnLimit);
    clang_vx::IoMapOptional<FormatStyle>(IO, "CommentPragmas",
                                         Style.CommentPragmas);
    clang_vx::IoMapOptional<FormatStyle>(IO, "CompactNamespaces",
                                         Style.CompactNamespaces);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "ConstructorInitializerAllOnOneLineOrOnePerLine",
        Style.ConstructorInitializerAllOnOneLineOrOnePerLine);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "ConstructorInitializerIndentWidth",
        Style.ConstructorInitializerIndentWidth);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ContinuationIndentWidth",
                                         Style.ContinuationIndentWidth);
    clang_vx::IoMapOptional<FormatStyle>(IO, "Cpp11BracedListStyle",
                                         Style.Cpp11BracedListStyle);
    clang_vx::IoMapOptional<FormatStyle>(IO, "DerivePointerAlignment",
                                         Style.DerivePointerAlignment);
    clang_vx::IoMapOptional<FormatStyle>(IO, "DisableFormat",
                                         Style.DisableFormat);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "ExperimentalAutoDetectBinPacking",
        Style.ExperimentalAutoDetectBinPacking);
    clang_vx::IoMapOptional<FormatStyle>(IO, "FixNamespaceComments",
                                         Style.FixNamespaceComments);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ForEachMacros",
                                         Style.ForEachMacros);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IncludeBlocks",
                                         Style.IncludeStyle.IncludeBlocks);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IncludeCategories",
                                         Style.IncludeStyle.IncludeCategories);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IncludeIsMainRegex",
                                         Style.IncludeStyle.IncludeIsMainRegex);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IndentCaseLabels",
                                         Style.IndentCaseLabels);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IndentPPDirectives",
                                         Style.IndentPPDirectives);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IndentWidth", Style.IndentWidth);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IndentWrappedFunctionNames",
                                         Style.IndentWrappedFunctionNames);
    clang_vx::IoMapOptional<FormatStyle>(IO, "JavaScriptQuotes",
                                         Style.JavaScriptQuotes);
    clang_vx::IoMapOptional<FormatStyle>(IO, "JavaScriptWrapImports",
                                         Style.JavaScriptWrapImports);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "KeepEmptyLinesAtTheStartOfBlocks",
        Style.KeepEmptyLinesAtTheStartOfBlocks);
    clang_vx::IoMapOptional<FormatStyle>(IO, "MacroBlockBegin",
                                         Style.MacroBlockBegin);
    clang_vx::IoMapOptional<FormatStyle>(IO, "MacroBlockEnd",
                                         Style.MacroBlockEnd);
    clang_vx::IoMapOptional<FormatStyle>(IO, "MaxEmptyLinesToKeep",
                                         Style.MaxEmptyLinesToKeep);
    clang_vx::IoMapOptional<FormatStyle>(IO, "NamespaceIndentation",
                                         Style.NamespaceIndentation);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ObjCBinPackProtocolList",
                                         Style.ObjCBinPackProtocolList);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ObjCBlockIndentWidth",
                                         Style.ObjCBlockIndentWidth);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ObjCSpaceAfterProperty",
                                         Style.ObjCSpaceAfterProperty);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ObjCSpaceBeforeProtocolList",
                                         Style.ObjCSpaceBeforeProtocolList);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyBreakAssignment",
                                         Style.PenaltyBreakAssignment);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "PenaltyBreakBeforeFirstCallParameter",
        Style.PenaltyBreakBeforeFirstCallParameter);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyBreakComment",
                                         Style.PenaltyBreakComment);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyBreakFirstLessLess",
                                         Style.PenaltyBreakFirstLessLess);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyBreakString",
                                         Style.PenaltyBreakString);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyBreakTemplateDeclaration",
                                         Style.PenaltyBreakTemplateDeclaration);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyExcessCharacter",
                                         Style.PenaltyExcessCharacter);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PenaltyReturnTypeOnItsOwnLine",
                                         Style.PenaltyReturnTypeOnItsOwnLine);
    clang_vx::IoMapOptional<FormatStyle>(IO, "PointerAlignment",
                                         Style.PointerAlignment);
    clang_vx::IoMapOptional<FormatStyle>(IO, "RawStringFormats",
                                         Style.RawStringFormats);
    clang_vx::IoMapOptional<FormatStyle>(IO, "ReflowComments",
                                         Style.ReflowComments);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SortIncludes",
                                         Style.SortIncludes);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SortUsingDeclarations",
                                         Style.SortUsingDeclarations);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceAfterCStyleCast",
                                         Style.SpaceAfterCStyleCast);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceAfterTemplateKeyword",
                                         Style.SpaceAfterTemplateKeyword);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceBeforeAssignmentOperators",
                                         Style.SpaceBeforeAssignmentOperators);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceBeforeCpp11BracedList",
                                         Style.SpaceBeforeCpp11BracedList);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceBeforeCtorInitializerColon",
                                         Style.SpaceBeforeCtorInitializerColon);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceBeforeInheritanceColon",
                                         Style.SpaceBeforeInheritanceColon);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceBeforeParens",
                                         Style.SpaceBeforeParens);
    clang_vx::IoMapOptional<FormatStyle>(
        IO, "SpaceBeforeRangeBasedForLoopColon",
        Style.SpaceBeforeRangeBasedForLoopColon);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpaceInEmptyParentheses",
                                         Style.SpaceInEmptyParentheses);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesBeforeTrailingComments",
                                         Style.SpacesBeforeTrailingComments);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesInAngles",
                                         Style.SpacesInAngles);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesInContainerLiterals",
                                         Style.SpacesInContainerLiterals);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesInCStyleCastParentheses",
                                         Style.SpacesInCStyleCastParentheses);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesInParentheses",
                                         Style.SpacesInParentheses);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SpacesInSquareBrackets",
                                         Style.SpacesInSquareBrackets);
    clang_vx::IoMapOptional<FormatStyle>(IO, "Standard", Style.Standard);
    clang_vx::IoMapOptional<FormatStyle>(IO, "TabWidth", Style.TabWidth);
    clang_vx::IoMapOptional<FormatStyle>(IO, "UseTab", Style.UseTab);
  }
};

template <> struct MappingTraits<FormatStyle::BraceWrappingFlags> {
  static void mapping(IO &IO, FormatStyle::BraceWrappingFlags &Wrapping) {
    clang_vx::IoMapOptional<FormatStyle>(IO, "AfterClass", Wrapping.AfterClass);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AfterControlStatement",
                                         Wrapping.AfterControlStatement);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AfterEnum", Wrapping.AfterEnum);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AfterFunction",
                                         Wrapping.AfterFunction);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AfterNamespace",
                                         Wrapping.AfterNamespace);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AfterObjCDeclaration",
                                         Wrapping.AfterObjCDeclaration);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AfterStruct",
                                         Wrapping.AfterStruct);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AfterUnion", Wrapping.AfterUnion);
    clang_vx::IoMapOptional<FormatStyle>(IO, "AfterExternBlock",
                                         Wrapping.AfterExternBlock);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BeforeCatch",
                                         Wrapping.BeforeCatch);
    clang_vx::IoMapOptional<FormatStyle>(IO, "BeforeElse", Wrapping.BeforeElse);
    clang_vx::IoMapOptional<FormatStyle>(IO, "IndentBraces",
                                         Wrapping.IndentBraces);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SplitEmptyFunction",
                                         Wrapping.SplitEmptyFunction);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SplitEmptyRecord",
                                         Wrapping.SplitEmptyRecord);
    clang_vx::IoMapOptional<FormatStyle>(IO, "SplitEmptyNamespace",
                                         Wrapping.SplitEmptyNamespace);
  }
};

template <> struct MappingTraits<FormatStyle::RawStringFormat> {
  static void mapping(IO &IO, FormatStyle::RawStringFormat &Format) {
    IO.mapOptional("Language", Format.Language);
    IO.mapOptional("Delimiters", Format.Delimiters);
    IO.mapOptional("EnclosingFunctions", Format.EnclosingFunctions);
    IO.mapOptional("CanonicalDelimiter", Format.CanonicalDelimiter);
    IO.mapOptional("BasedOnStyle", Format.BasedOnStyle);
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
      if (!Seq.empty() && Seq[0].Language == FormatStyle::LK_None) {
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

namespace clang_v7 {

const std::error_category &getParseCategory() {
  static const ParseErrorCategory C{};
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
  Expanded.BraceWrapping = {false, false, false, false, false,
                            false, false, false, false, false,
                            false, false, true,  true,  true};
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
    Expanded.BraceWrapping.AfterExternBlock = true;
    Expanded.BraceWrapping.SplitEmptyFunction = true;
    Expanded.BraceWrapping.SplitEmptyRecord = false;
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
    Expanded.BraceWrapping.AfterExternBlock = true;
    Expanded.BraceWrapping.BeforeCatch = true;
    Expanded.BraceWrapping.BeforeElse = true;
    break;
  case FormatStyle::BS_GNU:
    Expanded.BraceWrapping = {true, true, true, true, true, true, true, true,
                              true, true, true, true, true, true, true};
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
  LLVMStyle.AlignEscapedNewlines = FormatStyle::ENAS_Right;
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
  LLVMStyle.AlwaysBreakTemplateDeclarations = FormatStyle::BTDS_MultiLine;
  LLVMStyle.BinPackArguments = true;
  LLVMStyle.BinPackParameters = true;
  LLVMStyle.BreakBeforeBinaryOperators = FormatStyle::BOS_None;
  LLVMStyle.BreakBeforeTernaryOperators = true;
  LLVMStyle.BreakBeforeBraces = FormatStyle::BS_Attach;
  LLVMStyle.BraceWrapping = {false, false, false, false, false,
                             false, false, false, false, false,
                             false, false, true,  true,  true};
  LLVMStyle.BreakAfterJavaFieldAnnotations = false;
  LLVMStyle.BreakConstructorInitializers = FormatStyle::BCIS_BeforeColon;
  LLVMStyle.BreakInheritanceList = FormatStyle::BILS_BeforeColon;
  LLVMStyle.BreakStringLiterals = true;
  LLVMStyle.ColumnLimit = 80;
  LLVMStyle.CommentPragmas = "^ IWYU pragma:";
  LLVMStyle.CompactNamespaces = false;
  LLVMStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
  LLVMStyle.ConstructorInitializerIndentWidth = 4;
  LLVMStyle.ContinuationIndentWidth = 4;
  LLVMStyle.Cpp11BracedListStyle = true;
  LLVMStyle.DerivePointerAlignment = false;
  LLVMStyle.ExperimentalAutoDetectBinPacking = false;
  LLVMStyle.FixNamespaceComments = true;
  LLVMStyle.ForEachMacros.push_back("foreach");
  LLVMStyle.ForEachMacros.push_back("Q_FOREACH");
  LLVMStyle.ForEachMacros.push_back("BOOST_FOREACH");
  LLVMStyle.IncludeStyle.IncludeCategories = {
      {"^\"(llvm|llvm-c|clang|clang-c)/", 2},
      {"^(<|\"(gtest|gmock|isl|json)/)", 3},
      {".*", 1}};
  LLVMStyle.IncludeStyle.IncludeIsMainRegex = "(Test)?$";
  LLVMStyle.IncludeStyle.IncludeBlocks = clang_v7::IncludeStyle::IBS_Preserve;
  LLVMStyle.IndentCaseLabels = false;
  LLVMStyle.IndentPPDirectives = FormatStyle::PPDIS_None;
  LLVMStyle.IndentWrappedFunctionNames = false;
  LLVMStyle.IndentWidth = 2;
  LLVMStyle.JavaScriptQuotes = FormatStyle::JSQS_Leave;
  LLVMStyle.JavaScriptWrapImports = true;
  LLVMStyle.TabWidth = 8;
  LLVMStyle.MaxEmptyLinesToKeep = 1;
  LLVMStyle.KeepEmptyLinesAtTheStartOfBlocks = true;
  LLVMStyle.NamespaceIndentation = FormatStyle::NI_None;
  LLVMStyle.ObjCBinPackProtocolList = FormatStyle::BPS_Auto;
  LLVMStyle.ObjCBlockIndentWidth = 2;
  LLVMStyle.ObjCSpaceAfterProperty = false;
  LLVMStyle.ObjCSpaceBeforeProtocolList = true;
  LLVMStyle.PointerAlignment = FormatStyle::PAS_Right;
  LLVMStyle.SpacesBeforeTrailingComments = 1;
  LLVMStyle.Standard = FormatStyle::LS_Cpp11;
  LLVMStyle.UseTab = FormatStyle::UT_Never;
  LLVMStyle.ReflowComments = true;
  LLVMStyle.SpacesInParentheses = false;
  LLVMStyle.SpacesInSquareBrackets = false;
  LLVMStyle.SpaceInEmptyParentheses = false;
  LLVMStyle.SpacesInContainerLiterals = true;
  LLVMStyle.SpacesInCStyleCastParentheses = false;
  LLVMStyle.SpaceAfterCStyleCast = false;
  LLVMStyle.SpaceAfterTemplateKeyword = true;
  LLVMStyle.SpaceBeforeCtorInitializerColon = true;
  LLVMStyle.SpaceBeforeInheritanceColon = true;
  LLVMStyle.SpaceBeforeParens = FormatStyle::SBPO_ControlStatements;
  LLVMStyle.SpaceBeforeRangeBasedForLoopColon = true;
  LLVMStyle.SpaceBeforeAssignmentOperators = true;
  LLVMStyle.SpaceBeforeCpp11BracedList = false;
  LLVMStyle.SpacesInAngles = false;

  LLVMStyle.PenaltyBreakAssignment = prec::Assignment;
  LLVMStyle.PenaltyBreakComment = 300;
  LLVMStyle.PenaltyBreakFirstLessLess = 120;
  LLVMStyle.PenaltyBreakString = 1000;
  LLVMStyle.PenaltyExcessCharacter = 1000000;
  LLVMStyle.PenaltyReturnTypeOnItsOwnLine = 60;
  LLVMStyle.PenaltyBreakBeforeFirstCallParameter = 19;
  LLVMStyle.PenaltyBreakTemplateDeclaration = prec::Relational;

  LLVMStyle.DisableFormat = false;
  LLVMStyle.SortIncludes = true;
  LLVMStyle.SortUsingDeclarations = true;

  return LLVMStyle;
}

FormatStyle getGoogleStyle(FormatStyle::LanguageKind Language) {
  if (Language == FormatStyle::LK_TextProto) {
    FormatStyle GoogleStyle = getGoogleStyle(FormatStyle::LK_Proto);
    GoogleStyle.Language = FormatStyle::LK_TextProto;

    return GoogleStyle;
  }

  FormatStyle GoogleStyle = getLLVMStyle();
  GoogleStyle.Language = Language;

  GoogleStyle.AccessModifierOffset = -1;
  GoogleStyle.AlignEscapedNewlines = FormatStyle::ENAS_Left;
  GoogleStyle.AllowShortIfStatementsOnASingleLine = true;
  GoogleStyle.AllowShortLoopsOnASingleLine = true;
  GoogleStyle.AlwaysBreakBeforeMultilineStrings = true;
  GoogleStyle.AlwaysBreakTemplateDeclarations = FormatStyle::BTDS_Yes;
  GoogleStyle.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
  GoogleStyle.DerivePointerAlignment = true;
  GoogleStyle.IncludeStyle.IncludeCategories = {
      {"^<ext/.*\\.h>", 2}, {"^<.*\\.h>", 1}, {"^<.*", 2}, {".*", 3}};
  GoogleStyle.IncludeStyle.IncludeIsMainRegex = "([-_](test|unittest))?$";
  GoogleStyle.IndentCaseLabels = true;
  GoogleStyle.KeepEmptyLinesAtTheStartOfBlocks = false;
  GoogleStyle.ObjCBinPackProtocolList = FormatStyle::BPS_Never;
  GoogleStyle.ObjCSpaceAfterProperty = false;
  GoogleStyle.ObjCSpaceBeforeProtocolList = true;
  GoogleStyle.PointerAlignment = FormatStyle::PAS_Left;
  GoogleStyle.RawStringFormats = {
      {
          FormatStyle::LK_Cpp,
          /*Delimiters=*/
          {
              "cc",
              "CC",
              "cpp",
              "Cpp",
              "CPP",
              "c++",
              "C++",
          },
          /*EnclosingFunctionNames=*/
          {},
          /*CanonicalDelimiter=*/"",
          /*BasedOnStyle=*/"google",
      },
      {
          FormatStyle::LK_TextProto,
          /*Delimiters=*/
          {
              "pb",
              "PB",
              "proto",
              "PROTO",
          },
          /*EnclosingFunctionNames=*/
          {
              "EqualsProto",
              "EquivToProto",
              "PARSE_PARTIAL_TEXT_PROTO",
              "PARSE_TEST_PROTO",
              "PARSE_TEXT_PROTO",
              "ParseTextOrDie",
              "ParseTextProtoOrDie",
          },
          /*CanonicalDelimiter=*/"",
          /*BasedOnStyle=*/"google",
      },
  };
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
    // taze:, triple slash directives (`/// <...`), @see, which is commonly
    // followed by overlong URLs.
    GoogleStyle.CommentPragmas = "(taze:|^/[ \t]*<|@see)";
    GoogleStyle.MaxEmptyLinesToKeep = 3;
    GoogleStyle.NamespaceIndentation = FormatStyle::NI_All;
    GoogleStyle.SpacesInContainerLiterals = false;
    GoogleStyle.JavaScriptQuotes = FormatStyle::JSQS_Single;
    GoogleStyle.JavaScriptWrapImports = false;
  } else if (Language == FormatStyle::LK_Proto) {
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_None;
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
    GoogleStyle.SpacesInContainerLiterals = false;
    GoogleStyle.Cpp11BracedListStyle = false;
    // This affects protocol buffer options specifications and text protos.
    // Text protos are currently mostly formatted inside C++ raw string literals
    // and often the current breaking behavior of string literals is not
    // beneficial there. Investigate turning this on once proper string reflow
    // has been implemented.
    GoogleStyle.BreakStringLiterals = false;
  } else if (Language == FormatStyle::LK_ObjC) {
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
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
    if (Language == FormatStyle::LK_ObjC)
      ChromiumStyle.ColumnLimit = 80;
  }
  return ChromiumStyle;
}

FormatStyle getMozillaStyle() {
  FormatStyle MozillaStyle = getLLVMStyle();
  MozillaStyle.AllowAllParametersOfDeclarationOnNextLine = false;
  MozillaStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Inline;
  MozillaStyle.AlwaysBreakAfterReturnType = FormatStyle::RTBS_TopLevel;
  MozillaStyle.AlwaysBreakAfterDefinitionReturnType =
      FormatStyle::DRTBS_TopLevel;
  MozillaStyle.AlwaysBreakTemplateDeclarations = FormatStyle::BTDS_Yes;
  MozillaStyle.BinPackParameters = false;
  MozillaStyle.BinPackArguments = false;
  MozillaStyle.BreakBeforeBraces = FormatStyle::BS_Mozilla;
  MozillaStyle.BreakConstructorInitializers = FormatStyle::BCIS_BeforeComma;
  MozillaStyle.BreakInheritanceList = FormatStyle::BILS_BeforeComma;
  MozillaStyle.ConstructorInitializerIndentWidth = 2;
  MozillaStyle.ContinuationIndentWidth = 2;
  MozillaStyle.Cpp11BracedListStyle = false;
  MozillaStyle.FixNamespaceComments = false;
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
  Style.BreakConstructorInitializers = FormatStyle::BCIS_BeforeComma;
  Style.Cpp11BracedListStyle = false;
  Style.ColumnLimit = 0;
  Style.FixNamespaceComments = false;
  Style.IndentWidth = 4;
  Style.NamespaceIndentation = FormatStyle::NI_Inner;
  Style.ObjCBlockIndentWidth = 4;
  Style.ObjCSpaceAfterProperty = true;
  Style.PointerAlignment = FormatStyle::PAS_Left;
  Style.SpaceBeforeCpp11BracedList = true;
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
  Style.FixNamespaceComments = false;
  Style.SpaceBeforeParens = FormatStyle::SBPO_Always;
  Style.Standard = FormatStyle::LS_Cpp03;
  return Style;
}

FormatStyle getNoStyle() {
  FormatStyle NoStyle = getLLVMStyle();
  NoStyle.DisableFormat = true;
  NoStyle.SortIncludes = false;
  NoStyle.SortUsingDeclarations = false;
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
  Style->StyleSet.Clear();
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
  FormatStyle::FormatStyleSet StyleSet;
  bool LanguageFound = false;
  for (int i = Styles.size() - 1; i >= 0; --i) {
    if (Styles[i].Language != FormatStyle::LK_None)
      StyleSet.Add(Styles[i]);
    if (Styles[i].Language == Language)
      LanguageFound = true;
  }
  if (!LanguageFound) {
    if (Styles.empty() || Styles[0].Language != FormatStyle::LK_None)
      return make_error_code(ParseError::Unsuitable);
    FormatStyle DefaultStyle = Styles[0];
    DefaultStyle.Language = Language;
    StyleSet.Add(std::move(DefaultStyle));
  }
  *Style = *StyleSet.Get(Language);
  return make_error_code(ParseError::Success);
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
  if (!getPredefinedStyle(DefaultStyleName, Style.Language, &DefaultStyle)) {
    ctxt.emplace(nullptr, NonConstStyle, false);
  } else {
    ctxt.emplace(&DefaultStyle, NonConstStyle, SkipSameValue);
  }
  llvm::yaml::Output Output(Stream, &*ctxt);
  Output << NonConstStyle;

  return Stream.str();
}

std::optional<FormatStyle>
FormatStyle::FormatStyleSet::Get(FormatStyle::LanguageKind Language) const {
  if (!Styles)
    return std::nullopt;
  auto It = Styles->find(Language);
  if (It == Styles->end())
    return std::nullopt;
  FormatStyle Style = It->second;
  Style.StyleSet = *this;
  return Style;
}

void FormatStyle::FormatStyleSet::Add(FormatStyle Style) {
  assert(Style.Language != LK_None &&
         "Cannot add a style for LK_None to a StyleSet");
  assert(
      !Style.StyleSet.Styles &&
      "Cannot add a style associated with an existing StyleSet to a StyleSet");
  if (!Styles)
    Styles = std::make_shared<MapType>();
  (*Styles)[Style.Language] = std::move(Style);
}

void FormatStyle::FormatStyleSet::Clear() { Styles.reset(); }

std::optional<FormatStyle>
FormatStyle::GetLanguageStyle(FormatStyle::LanguageKind Language) const {
  return StyleSet.Get(Language);
}

} // namespace clang_v7
