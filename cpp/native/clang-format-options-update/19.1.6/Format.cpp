//===--- Format.cpp - Format C++ code -------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements functions declared in Format.h. This will be
/// split into separate files as we go.
///
//===----------------------------------------------------------------------===//

#include "Format.h"
#include "clang/Basic/OperatorPrecedence.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Debug.h"
#include <set>

#define DEBUG_TYPE "format-formatter"

using clang_v19::FormatStyle;

namespace prec = clang::prec;
namespace tok = clang::tok;

LLVM_YAML_IS_SEQUENCE_VECTOR(FormatStyle::RawStringFormat)

namespace llvm {
namespace yaml {
template <>
struct ScalarEnumerationTraits<FormatStyle::BreakBeforeNoexceptSpecifierStyle> {
  static void
  enumeration(IO &IO, FormatStyle::BreakBeforeNoexceptSpecifierStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::BBNSS_Never);
    IO.enumCase(Value, "OnlyWithParen", FormatStyle::BBNSS_OnlyWithParen);
    IO.enumCase(Value, "Always", FormatStyle::BBNSS_Always);
  }
};

template <> struct MappingTraits<FormatStyle::AlignConsecutiveStyle> {
  static void enumInput(IO &IO, FormatStyle::AlignConsecutiveStyle &Value) {
    IO.enumCase(Value, "None",
                FormatStyle::AlignConsecutiveStyle(
                    {/*Enabled=*/false, /*AcrossEmptyLines=*/false,
                     /*AcrossComments=*/false, /*AlignCompound=*/false,
                     /*AlignFunctionPointers=*/false, /*PadOperators=*/true}));
    IO.enumCase(Value, "Consecutive",
                FormatStyle::AlignConsecutiveStyle(
                    {/*Enabled=*/true, /*AcrossEmptyLines=*/false,
                     /*AcrossComments=*/false, /*AlignCompound=*/false,
                     /*AlignFunctionPointers=*/false, /*PadOperators=*/true}));
    IO.enumCase(Value, "AcrossEmptyLines",
                FormatStyle::AlignConsecutiveStyle(
                    {/*Enabled=*/true, /*AcrossEmptyLines=*/true,
                     /*AcrossComments=*/false, /*AlignCompound=*/false,
                     /*AlignFunctionPointers=*/false, /*PadOperators=*/true}));
    IO.enumCase(Value, "AcrossComments",
                FormatStyle::AlignConsecutiveStyle(
                    {/*Enabled=*/true, /*AcrossEmptyLines=*/false,
                     /*AcrossComments=*/true, /*AlignCompound=*/false,
                     /*AlignFunctionPointers=*/false, /*PadOperators=*/true}));
    IO.enumCase(Value, "AcrossEmptyLinesAndComments",
                FormatStyle::AlignConsecutiveStyle(
                    {/*Enabled=*/true, /*AcrossEmptyLines=*/true,
                     /*AcrossComments=*/true, /*AlignCompound=*/false,
                     /*AlignFunctionPointers=*/false, /*PadOperators=*/true}));

    // For backward compatibility.
    IO.enumCase(Value, "true",
                FormatStyle::AlignConsecutiveStyle(
                    {/*Enabled=*/true, /*AcrossEmptyLines=*/false,
                     /*AcrossComments=*/false, /*AlignCompound=*/false,
                     /*AlignFunctionPointers=*/false, /*PadOperators=*/true}));
    IO.enumCase(Value, "false",
                FormatStyle::AlignConsecutiveStyle(
                    {/*Enabled=*/false, /*AcrossEmptyLines=*/false,
                     /*AcrossComments=*/false, /*AlignCompound=*/false,
                     /*AlignFunctionPointers=*/false, /*PadOperators=*/true}));
  }

  static void mapping(IO &IO, FormatStyle::AlignConsecutiveStyle &Value) {
    IO.mapOptional("Enabled", Value.Enabled);
    IO.mapOptional("AcrossEmptyLines", Value.AcrossEmptyLines);
    IO.mapOptional("AcrossComments", Value.AcrossComments);
    IO.mapOptional("AlignCompound", Value.AlignCompound);
    IO.mapOptional("AlignFunctionPointers", Value.AlignFunctionPointers);
    IO.mapOptional("PadOperators", Value.PadOperators);
  }
};

template <>
struct MappingTraits<FormatStyle::ShortCaseStatementsAlignmentStyle> {
  static void mapping(IO &IO,
                      FormatStyle::ShortCaseStatementsAlignmentStyle &Value) {
    IO.mapOptional("Enabled", Value.Enabled);
    IO.mapOptional("AcrossEmptyLines", Value.AcrossEmptyLines);
    IO.mapOptional("AcrossComments", Value.AcrossComments);
    IO.mapOptional("AlignCaseArrows", Value.AlignCaseArrows);
    IO.mapOptional("AlignCaseColons", Value.AlignCaseColons);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::AttributeBreakingStyle> {
  static void enumeration(IO &IO, FormatStyle::AttributeBreakingStyle &Value) {
    IO.enumCase(Value, "Always", FormatStyle::ABS_Always);
    IO.enumCase(Value, "Leave", FormatStyle::ABS_Leave);
    IO.enumCase(Value, "Never", FormatStyle::ABS_Never);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::ArrayInitializerAlignmentStyle> {
  static void enumeration(IO &IO,
                          FormatStyle::ArrayInitializerAlignmentStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::AIAS_None);
    IO.enumCase(Value, "Left", FormatStyle::AIAS_Left);
    IO.enumCase(Value, "Right", FormatStyle::AIAS_Right);
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

template <> struct ScalarEnumerationTraits<FormatStyle::BinPackStyle> {
  static void enumeration(IO &IO, FormatStyle::BinPackStyle &Value) {
    IO.enumCase(Value, "Auto", FormatStyle::BPS_Auto);
    IO.enumCase(Value, "Always", FormatStyle::BPS_Always);
    IO.enumCase(Value, "Never", FormatStyle::BPS_Never);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::BitFieldColonSpacingStyle> {
  static void enumeration(IO &IO,
                          FormatStyle::BitFieldColonSpacingStyle &Value) {
    IO.enumCase(Value, "Both", FormatStyle::BFCS_Both);
    IO.enumCase(Value, "None", FormatStyle::BFCS_None);
    IO.enumCase(Value, "Before", FormatStyle::BFCS_Before);
    IO.enumCase(Value, "After", FormatStyle::BFCS_After);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::BraceBreakingStyle> {
  static void enumeration(IO &IO, FormatStyle::BraceBreakingStyle &Value) {
    IO.enumCase(Value, "Attach", FormatStyle::BS_Attach);
    IO.enumCase(Value, "Linux", FormatStyle::BS_Linux);
    IO.enumCase(Value, "Mozilla", FormatStyle::BS_Mozilla);
    IO.enumCase(Value, "Stroustrup", FormatStyle::BS_Stroustrup);
    IO.enumCase(Value, "Allman", FormatStyle::BS_Allman);
    IO.enumCase(Value, "Whitesmiths", FormatStyle::BS_Whitesmiths);
    IO.enumCase(Value, "GNU", FormatStyle::BS_GNU);
    IO.enumCase(Value, "WebKit", FormatStyle::BS_WebKit);
    IO.enumCase(Value, "Custom", FormatStyle::BS_Custom);
  }
};

template <> struct MappingTraits<FormatStyle::BraceWrappingFlags> {
  static void mapping(IO &IO, FormatStyle::BraceWrappingFlags &Wrapping) {
    IO.mapOptional("AfterCaseLabel", Wrapping.AfterCaseLabel);
    IO.mapOptional("AfterClass", Wrapping.AfterClass);
    IO.mapOptional("AfterControlStatement", Wrapping.AfterControlStatement);
    IO.mapOptional("AfterEnum", Wrapping.AfterEnum);
    IO.mapOptional("AfterExternBlock", Wrapping.AfterExternBlock);
    IO.mapOptional("AfterFunction", Wrapping.AfterFunction);
    IO.mapOptional("AfterNamespace", Wrapping.AfterNamespace);
    IO.mapOptional("AfterObjCDeclaration", Wrapping.AfterObjCDeclaration);
    IO.mapOptional("AfterStruct", Wrapping.AfterStruct);
    IO.mapOptional("AfterUnion", Wrapping.AfterUnion);
    IO.mapOptional("BeforeCatch", Wrapping.BeforeCatch);
    IO.mapOptional("BeforeElse", Wrapping.BeforeElse);
    IO.mapOptional("BeforeLambdaBody", Wrapping.BeforeLambdaBody);
    IO.mapOptional("BeforeWhile", Wrapping.BeforeWhile);
    IO.mapOptional("IndentBraces", Wrapping.IndentBraces);
    IO.mapOptional("SplitEmptyFunction", Wrapping.SplitEmptyFunction);
    IO.mapOptional("SplitEmptyRecord", Wrapping.SplitEmptyRecord);
    IO.mapOptional("SplitEmptyNamespace", Wrapping.SplitEmptyNamespace);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::BracketAlignmentStyle> {
  static void enumeration(IO &IO, FormatStyle::BracketAlignmentStyle &Value) {
    IO.enumCase(Value, "Align", FormatStyle::BAS_Align);
    IO.enumCase(Value, "DontAlign", FormatStyle::BAS_DontAlign);
    IO.enumCase(Value, "AlwaysBreak", FormatStyle::BAS_AlwaysBreak);
    IO.enumCase(Value, "BlockIndent", FormatStyle::BAS_BlockIndent);

    // For backward compatibility.
    IO.enumCase(Value, "true", FormatStyle::BAS_Align);
    IO.enumCase(Value, "false", FormatStyle::BAS_DontAlign);
  }
};

template <>
struct ScalarEnumerationTraits<
    FormatStyle::BraceWrappingAfterControlStatementStyle> {
  static void
  enumeration(IO &IO,
              FormatStyle::BraceWrappingAfterControlStatementStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::BWACS_Never);
    IO.enumCase(Value, "MultiLine", FormatStyle::BWACS_MultiLine);
    IO.enumCase(Value, "Always", FormatStyle::BWACS_Always);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::BWACS_Never);
    IO.enumCase(Value, "true", FormatStyle::BWACS_Always);
  }
};

template <>
struct ScalarEnumerationTraits<
    FormatStyle::BreakBeforeConceptDeclarationsStyle> {
  static void
  enumeration(IO &IO, FormatStyle::BreakBeforeConceptDeclarationsStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::BBCDS_Never);
    IO.enumCase(Value, "Allowed", FormatStyle::BBCDS_Allowed);
    IO.enumCase(Value, "Always", FormatStyle::BBCDS_Always);

    // For backward compatibility.
    IO.enumCase(Value, "true", FormatStyle::BBCDS_Always);
    IO.enumCase(Value, "false", FormatStyle::BBCDS_Allowed);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::BreakBeforeInlineASMColonStyle> {
  static void enumeration(IO &IO,
                          FormatStyle::BreakBeforeInlineASMColonStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::BBIAS_Never);
    IO.enumCase(Value, "OnlyMultiline", FormatStyle::BBIAS_OnlyMultiline);
    IO.enumCase(Value, "Always", FormatStyle::BBIAS_Always);
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
    IO.enumCase(Value, "AfterComma", FormatStyle::BILS_AfterComma);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::BreakTemplateDeclarationsStyle> {
  static void enumeration(IO &IO,
                          FormatStyle::BreakTemplateDeclarationsStyle &Value) {
    IO.enumCase(Value, "Leave", FormatStyle::BTDS_Leave);
    IO.enumCase(Value, "No", FormatStyle::BTDS_No);
    IO.enumCase(Value, "MultiLine", FormatStyle::BTDS_MultiLine);
    IO.enumCase(Value, "Yes", FormatStyle::BTDS_Yes);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::BTDS_MultiLine);
    IO.enumCase(Value, "true", FormatStyle::BTDS_Yes);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::DAGArgStyle> {
  static void enumeration(IO &IO, FormatStyle::DAGArgStyle &Value) {
    IO.enumCase(Value, "DontBreak", FormatStyle::DAS_DontBreak);
    IO.enumCase(Value, "BreakElements", FormatStyle::DAS_BreakElements);
    IO.enumCase(Value, "BreakAll", FormatStyle::DAS_BreakAll);
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
struct ScalarEnumerationTraits<FormatStyle::EscapedNewlineAlignmentStyle> {
  static void enumeration(IO &IO,
                          FormatStyle::EscapedNewlineAlignmentStyle &Value) {
    IO.enumCase(Value, "DontAlign", FormatStyle::ENAS_DontAlign);
    IO.enumCase(Value, "Left", FormatStyle::ENAS_Left);
    IO.enumCase(Value, "LeftWithLastLine", FormatStyle::ENAS_LeftWithLastLine);
    IO.enumCase(Value, "Right", FormatStyle::ENAS_Right);

    // For backward compatibility.
    IO.enumCase(Value, "true", FormatStyle::ENAS_Left);
    IO.enumCase(Value, "false", FormatStyle::ENAS_Right);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::EmptyLineAfterAccessModifierStyle> {
  static void
  enumeration(IO &IO, FormatStyle::EmptyLineAfterAccessModifierStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::ELAAMS_Never);
    IO.enumCase(Value, "Leave", FormatStyle::ELAAMS_Leave);
    IO.enumCase(Value, "Always", FormatStyle::ELAAMS_Always);
  }
};

template <>
struct ScalarEnumerationTraits<
    FormatStyle::EmptyLineBeforeAccessModifierStyle> {
  static void
  enumeration(IO &IO, FormatStyle::EmptyLineBeforeAccessModifierStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::ELBAMS_Never);
    IO.enumCase(Value, "Leave", FormatStyle::ELBAMS_Leave);
    IO.enumCase(Value, "LogicalBlock", FormatStyle::ELBAMS_LogicalBlock);
    IO.enumCase(Value, "Always", FormatStyle::ELBAMS_Always);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::IndentExternBlockStyle> {
  static void enumeration(IO &IO, FormatStyle::IndentExternBlockStyle &Value) {
    IO.enumCase(Value, "AfterExternBlock", FormatStyle::IEBS_AfterExternBlock);
    IO.enumCase(Value, "Indent", FormatStyle::IEBS_Indent);
    IO.enumCase(Value, "NoIndent", FormatStyle::IEBS_NoIndent);
    IO.enumCase(Value, "true", FormatStyle::IEBS_Indent);
    IO.enumCase(Value, "false", FormatStyle::IEBS_NoIndent);
  }
};

template <> struct MappingTraits<FormatStyle::IntegerLiteralSeparatorStyle> {
  static void mapping(IO &IO, FormatStyle::IntegerLiteralSeparatorStyle &Base) {
    IO.mapOptional("Binary", Base.Binary);
    IO.mapOptional("BinaryMinDigits", Base.BinaryMinDigits);
    IO.mapOptional("Decimal", Base.Decimal);
    IO.mapOptional("DecimalMinDigits", Base.DecimalMinDigits);
    IO.mapOptional("Hex", Base.Hex);
    IO.mapOptional("HexMinDigits", Base.HexMinDigits);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::JavaScriptQuoteStyle> {
  static void enumeration(IO &IO, FormatStyle::JavaScriptQuoteStyle &Value) {
    IO.enumCase(Value, "Leave", FormatStyle::JSQS_Leave);
    IO.enumCase(Value, "Single", FormatStyle::JSQS_Single);
    IO.enumCase(Value, "Double", FormatStyle::JSQS_Double);
  }
};

template <> struct MappingTraits<FormatStyle::KeepEmptyLinesStyle> {
  static void mapping(IO &IO, FormatStyle::KeepEmptyLinesStyle &Value) {
    IO.mapOptional("AtEndOfFile", Value.AtEndOfFile);
    IO.mapOptional("AtStartOfBlock", Value.AtStartOfBlock);
    IO.mapOptional("AtStartOfFile", Value.AtStartOfFile);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::LanguageKind> {
  static void enumeration(IO &IO, FormatStyle::LanguageKind &Value) {
    IO.enumCase(Value, "Cpp", FormatStyle::LK_Cpp);
    IO.enumCase(Value, "Java", FormatStyle::LK_Java);
    IO.enumCase(Value, "JavaScript", FormatStyle::LK_JavaScript);
    IO.enumCase(Value, "ObjC", FormatStyle::LK_ObjC);
    IO.enumCase(Value, "Proto", FormatStyle::LK_Proto);
    IO.enumCase(Value, "TableGen", FormatStyle::LK_TableGen);
    IO.enumCase(Value, "TextProto", FormatStyle::LK_TextProto);
    IO.enumCase(Value, "CSharp", FormatStyle::LK_CSharp);
    IO.enumCase(Value, "Json", FormatStyle::LK_Json);
    IO.enumCase(Value, "Verilog", FormatStyle::LK_Verilog);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::LanguageStandard> {
  static void enumeration(IO &IO, FormatStyle::LanguageStandard &Value) {
    IO.enumCase(Value, "c++03", FormatStyle::LS_Cpp03);
    IO.enumCase(Value, "C++03", FormatStyle::LS_Cpp03); // Legacy alias
    IO.enumCase(Value, "Cpp03", FormatStyle::LS_Cpp03); // Legacy alias

    IO.enumCase(Value, "c++11", FormatStyle::LS_Cpp11);
    IO.enumCase(Value, "C++11", FormatStyle::LS_Cpp11); // Legacy alias

    IO.enumCase(Value, "c++14", FormatStyle::LS_Cpp14);
    IO.enumCase(Value, "c++17", FormatStyle::LS_Cpp17);
    IO.enumCase(Value, "c++20", FormatStyle::LS_Cpp20);

    IO.enumCase(Value, "Latest", FormatStyle::LS_Latest);
    IO.enumCase(Value, "Cpp11", FormatStyle::LS_Latest); // Legacy alias
    IO.enumCase(Value, "Auto", FormatStyle::LS_Auto);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::LambdaBodyIndentationKind> {
  static void enumeration(IO &IO,
                          FormatStyle::LambdaBodyIndentationKind &Value) {
    IO.enumCase(Value, "Signature", FormatStyle::LBI_Signature);
    IO.enumCase(Value, "OuterScope", FormatStyle::LBI_OuterScope);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::LineEndingStyle> {
  static void enumeration(IO &IO, FormatStyle::LineEndingStyle &Value) {
    IO.enumCase(Value, "LF", FormatStyle::LE_LF);
    IO.enumCase(Value, "CRLF", FormatStyle::LE_CRLF);
    IO.enumCase(Value, "DeriveLF", FormatStyle::LE_DeriveLF);
    IO.enumCase(Value, "DeriveCRLF", FormatStyle::LE_DeriveCRLF);
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

template <> struct ScalarEnumerationTraits<FormatStyle::OperandAlignmentStyle> {
  static void enumeration(IO &IO, FormatStyle::OperandAlignmentStyle &Value) {
    IO.enumCase(Value, "DontAlign", FormatStyle::OAS_DontAlign);
    IO.enumCase(Value, "Align", FormatStyle::OAS_Align);
    IO.enumCase(Value, "AlignAfterOperator",
                FormatStyle::OAS_AlignAfterOperator);

    // For backward compatibility.
    IO.enumCase(Value, "true", FormatStyle::OAS_Align);
    IO.enumCase(Value, "false", FormatStyle::OAS_DontAlign);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::PackConstructorInitializersStyle> {
  static void
  enumeration(IO &IO, FormatStyle::PackConstructorInitializersStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::PCIS_Never);
    IO.enumCase(Value, "BinPack", FormatStyle::PCIS_BinPack);
    IO.enumCase(Value, "CurrentLine", FormatStyle::PCIS_CurrentLine);
    IO.enumCase(Value, "NextLine", FormatStyle::PCIS_NextLine);
    IO.enumCase(Value, "NextLineOnly", FormatStyle::PCIS_NextLineOnly);
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
struct ScalarEnumerationTraits<FormatStyle::PPDirectiveIndentStyle> {
  static void enumeration(IO &IO, FormatStyle::PPDirectiveIndentStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::PPDIS_None);
    IO.enumCase(Value, "AfterHash", FormatStyle::PPDIS_AfterHash);
    IO.enumCase(Value, "BeforeHash", FormatStyle::PPDIS_BeforeHash);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::QualifierAlignmentStyle> {
  static void enumeration(IO &IO, FormatStyle::QualifierAlignmentStyle &Value) {
    IO.enumCase(Value, "Leave", FormatStyle::QAS_Leave);
    IO.enumCase(Value, "Left", FormatStyle::QAS_Left);
    IO.enumCase(Value, "Right", FormatStyle::QAS_Right);
    IO.enumCase(Value, "Custom", FormatStyle::QAS_Custom);
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

template <>
struct ScalarEnumerationTraits<FormatStyle::ReferenceAlignmentStyle> {
  static void enumeration(IO &IO, FormatStyle::ReferenceAlignmentStyle &Value) {
    IO.enumCase(Value, "Pointer", FormatStyle::RAS_Pointer);
    IO.enumCase(Value, "Middle", FormatStyle::RAS_Middle);
    IO.enumCase(Value, "Left", FormatStyle::RAS_Left);
    IO.enumCase(Value, "Right", FormatStyle::RAS_Right);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::RemoveParenthesesStyle> {
  static void enumeration(IO &IO, FormatStyle::RemoveParenthesesStyle &Value) {
    IO.enumCase(Value, "Leave", FormatStyle::RPS_Leave);
    IO.enumCase(Value, "MultipleParentheses",
                FormatStyle::RPS_MultipleParentheses);
    IO.enumCase(Value, "ReturnStatement", FormatStyle::RPS_ReturnStatement);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::RequiresClausePositionStyle> {
  static void enumeration(IO &IO,
                          FormatStyle::RequiresClausePositionStyle &Value) {
    IO.enumCase(Value, "OwnLine", FormatStyle::RCPS_OwnLine);
    IO.enumCase(Value, "WithPreceding", FormatStyle::RCPS_WithPreceding);
    IO.enumCase(Value, "WithFollowing", FormatStyle::RCPS_WithFollowing);
    IO.enumCase(Value, "SingleLine", FormatStyle::RCPS_SingleLine);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::RequiresExpressionIndentationKind> {
  static void
  enumeration(IO &IO, FormatStyle::RequiresExpressionIndentationKind &Value) {
    IO.enumCase(Value, "Keyword", FormatStyle::REI_Keyword);
    IO.enumCase(Value, "OuterScope", FormatStyle::REI_OuterScope);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::ReturnTypeBreakingStyle> {
  static void enumeration(IO &IO, FormatStyle::ReturnTypeBreakingStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::RTBS_None);
    IO.enumCase(Value, "Automatic", FormatStyle::RTBS_Automatic);
    IO.enumCase(Value, "ExceptShortType", FormatStyle::RTBS_ExceptShortType);
    IO.enumCase(Value, "All", FormatStyle::RTBS_All);
    IO.enumCase(Value, "TopLevel", FormatStyle::RTBS_TopLevel);
    IO.enumCase(Value, "TopLevelDefinitions",
                FormatStyle::RTBS_TopLevelDefinitions);
    IO.enumCase(Value, "AllDefinitions", FormatStyle::RTBS_AllDefinitions);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::SeparateDefinitionStyle> {
  static void enumeration(IO &IO, FormatStyle::SeparateDefinitionStyle &Value) {
    IO.enumCase(Value, "Leave", FormatStyle::SDS_Leave);
    IO.enumCase(Value, "Always", FormatStyle::SDS_Always);
    IO.enumCase(Value, "Never", FormatStyle::SDS_Never);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::ShortBlockStyle> {
  static void enumeration(IO &IO, FormatStyle::ShortBlockStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::SBS_Never);
    IO.enumCase(Value, "false", FormatStyle::SBS_Never);
    IO.enumCase(Value, "Always", FormatStyle::SBS_Always);
    IO.enumCase(Value, "true", FormatStyle::SBS_Always);
    IO.enumCase(Value, "Empty", FormatStyle::SBS_Empty);
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

template <> struct ScalarEnumerationTraits<FormatStyle::ShortIfStyle> {
  static void enumeration(IO &IO, FormatStyle::ShortIfStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::SIS_Never);
    IO.enumCase(Value, "WithoutElse", FormatStyle::SIS_WithoutElse);
    IO.enumCase(Value, "OnlyFirstIf", FormatStyle::SIS_OnlyFirstIf);
    IO.enumCase(Value, "AllIfsAndElse", FormatStyle::SIS_AllIfsAndElse);

    // For backward compatibility.
    IO.enumCase(Value, "Always", FormatStyle::SIS_OnlyFirstIf);
    IO.enumCase(Value, "false", FormatStyle::SIS_Never);
    IO.enumCase(Value, "true", FormatStyle::SIS_WithoutElse);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::ShortLambdaStyle> {
  static void enumeration(IO &IO, FormatStyle::ShortLambdaStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::SLS_None);
    IO.enumCase(Value, "false", FormatStyle::SLS_None);
    IO.enumCase(Value, "Empty", FormatStyle::SLS_Empty);
    IO.enumCase(Value, "Inline", FormatStyle::SLS_Inline);
    IO.enumCase(Value, "All", FormatStyle::SLS_All);
    IO.enumCase(Value, "true", FormatStyle::SLS_All);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::SortIncludesOptions> {
  static void enumeration(IO &IO, FormatStyle::SortIncludesOptions &Value) {
    IO.enumCase(Value, "Never", FormatStyle::SI_Never);
    IO.enumCase(Value, "CaseInsensitive", FormatStyle::SI_CaseInsensitive);
    IO.enumCase(Value, "CaseSensitive", FormatStyle::SI_CaseSensitive);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::SI_Never);
    IO.enumCase(Value, "true", FormatStyle::SI_CaseSensitive);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::SortJavaStaticImportOptions> {
  static void enumeration(IO &IO,
                          FormatStyle::SortJavaStaticImportOptions &Value) {
    IO.enumCase(Value, "Before", FormatStyle::SJSIO_Before);
    IO.enumCase(Value, "After", FormatStyle::SJSIO_After);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::SortUsingDeclarationsOptions> {
  static void enumeration(IO &IO,
                          FormatStyle::SortUsingDeclarationsOptions &Value) {
    IO.enumCase(Value, "Never", FormatStyle::SUD_Never);
    IO.enumCase(Value, "Lexicographic", FormatStyle::SUD_Lexicographic);
    IO.enumCase(Value, "LexicographicNumeric",
                FormatStyle::SUD_LexicographicNumeric);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::SUD_Never);
    IO.enumCase(Value, "true", FormatStyle::SUD_LexicographicNumeric);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::SpaceAroundPointerQualifiersStyle> {
  static void
  enumeration(IO &IO, FormatStyle::SpaceAroundPointerQualifiersStyle &Value) {
    IO.enumCase(Value, "Default", FormatStyle::SAPQ_Default);
    IO.enumCase(Value, "Before", FormatStyle::SAPQ_Before);
    IO.enumCase(Value, "After", FormatStyle::SAPQ_After);
    IO.enumCase(Value, "Both", FormatStyle::SAPQ_Both);
  }
};

template <> struct MappingTraits<FormatStyle::SpaceBeforeParensCustom> {
  static void mapping(IO &IO, FormatStyle::SpaceBeforeParensCustom &Spacing) {
    IO.mapOptional("AfterControlStatements", Spacing.AfterControlStatements);
    IO.mapOptional("AfterForeachMacros", Spacing.AfterForeachMacros);
    IO.mapOptional("AfterFunctionDefinitionName",
                   Spacing.AfterFunctionDefinitionName);
    IO.mapOptional("AfterFunctionDeclarationName",
                   Spacing.AfterFunctionDeclarationName);
    IO.mapOptional("AfterIfMacros", Spacing.AfterIfMacros);
    IO.mapOptional("AfterOverloadedOperator", Spacing.AfterOverloadedOperator);
    IO.mapOptional("AfterPlacementOperator", Spacing.AfterPlacementOperator);
    IO.mapOptional("AfterRequiresInClause", Spacing.AfterRequiresInClause);
    IO.mapOptional("AfterRequiresInExpression",
                   Spacing.AfterRequiresInExpression);
    IO.mapOptional("BeforeNonEmptyParentheses",
                   Spacing.BeforeNonEmptyParentheses);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::SpaceBeforeParensStyle> {
  static void enumeration(IO &IO, FormatStyle::SpaceBeforeParensStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::SBPO_Never);
    IO.enumCase(Value, "ControlStatements",
                FormatStyle::SBPO_ControlStatements);
    IO.enumCase(Value, "ControlStatementsExceptControlMacros",
                FormatStyle::SBPO_ControlStatementsExceptControlMacros);
    IO.enumCase(Value, "NonEmptyParentheses",
                FormatStyle::SBPO_NonEmptyParentheses);
    IO.enumCase(Value, "Always", FormatStyle::SBPO_Always);
    IO.enumCase(Value, "Custom", FormatStyle::SBPO_Custom);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::SBPO_Never);
    IO.enumCase(Value, "true", FormatStyle::SBPO_ControlStatements);
    IO.enumCase(Value, "ControlStatementsExceptForEachMacros",
                FormatStyle::SBPO_ControlStatementsExceptControlMacros);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::SpacesInAnglesStyle> {
  static void enumeration(IO &IO, FormatStyle::SpacesInAnglesStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::SIAS_Never);
    IO.enumCase(Value, "Always", FormatStyle::SIAS_Always);
    IO.enumCase(Value, "Leave", FormatStyle::SIAS_Leave);

    // For backward compatibility.
    IO.enumCase(Value, "false", FormatStyle::SIAS_Never);
    IO.enumCase(Value, "true", FormatStyle::SIAS_Always);
  }
};

template <> struct MappingTraits<FormatStyle::SpacesInLineComment> {
  static void mapping(IO &IO, FormatStyle::SpacesInLineComment &Space) {
    // Transform the maximum to signed, to parse "-1" correctly
    int signedMaximum = static_cast<int>(Space.Maximum);
    IO.mapOptional("Minimum", Space.Minimum);
    IO.mapOptional("Maximum", signedMaximum);
    Space.Maximum = static_cast<unsigned>(signedMaximum);

    if (Space.Maximum != -1u)
      Space.Minimum = std::min(Space.Minimum, Space.Maximum);
  }
};

template <> struct MappingTraits<FormatStyle::SpacesInParensCustom> {
  static void mapping(IO &IO, FormatStyle::SpacesInParensCustom &Spaces) {
    IO.mapOptional("ExceptDoubleParentheses", Spaces.ExceptDoubleParentheses);
    IO.mapOptional("InCStyleCasts", Spaces.InCStyleCasts);
    IO.mapOptional("InConditionalStatements", Spaces.InConditionalStatements);
    IO.mapOptional("InEmptyParentheses", Spaces.InEmptyParentheses);
    IO.mapOptional("Other", Spaces.Other);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::SpacesInParensStyle> {
  static void enumeration(IO &IO, FormatStyle::SpacesInParensStyle &Value) {
    IO.enumCase(Value, "Never", FormatStyle::SIPO_Never);
    IO.enumCase(Value, "Custom", FormatStyle::SIPO_Custom);
  }
};

template <> struct ScalarEnumerationTraits<FormatStyle::TrailingCommaStyle> {
  static void enumeration(IO &IO, FormatStyle::TrailingCommaStyle &Value) {
    IO.enumCase(Value, "None", FormatStyle::TCS_None);
    IO.enumCase(Value, "Wrapped", FormatStyle::TCS_Wrapped);
  }
};

template <>
struct ScalarEnumerationTraits<FormatStyle::TrailingCommentsAlignmentKinds> {
  static void enumeration(IO &IO,
                          FormatStyle::TrailingCommentsAlignmentKinds &Value) {
    IO.enumCase(Value, "Leave", FormatStyle::TCAS_Leave);
    IO.enumCase(Value, "Always", FormatStyle::TCAS_Always);
    IO.enumCase(Value, "Never", FormatStyle::TCAS_Never);
  }
};

template <> struct MappingTraits<FormatStyle::TrailingCommentsAlignmentStyle> {
  static void enumInput(IO &IO,
                        FormatStyle::TrailingCommentsAlignmentStyle &Value) {
    IO.enumCase(Value, "Leave",
                FormatStyle::TrailingCommentsAlignmentStyle(
                    {FormatStyle::TCAS_Leave, 0}));

    IO.enumCase(Value, "Always",
                FormatStyle::TrailingCommentsAlignmentStyle(
                    {FormatStyle::TCAS_Always, 0}));

    IO.enumCase(Value, "Never",
                FormatStyle::TrailingCommentsAlignmentStyle(
                    {FormatStyle::TCAS_Never, 0}));

    // For backwards compatibility
    IO.enumCase(Value, "true",
                FormatStyle::TrailingCommentsAlignmentStyle(
                    {FormatStyle::TCAS_Always, 0}));
    IO.enumCase(Value, "false",
                FormatStyle::TrailingCommentsAlignmentStyle(
                    {FormatStyle::TCAS_Never, 0}));
  }

  static void mapping(IO &IO,
                      FormatStyle::TrailingCommentsAlignmentStyle &Value) {
    IO.mapOptional("Kind", Value.Kind);
    IO.mapOptional("OverEmptyLines", Value.OverEmptyLines);
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
    IO.enumCase(Value, "AlignWithSpaces", FormatStyle::UT_AlignWithSpaces);
  }
};

template <> struct MappingTraits<FormatStyle> {
  static void mapping(IO &IO, FormatStyle &Style) {
    // When reading, read the language first, we need it for getPredefinedStyle.
    IO.mapOptional("Language", Style.Language);

    StringRef BasedOnStyle;
    if (IO.outputting()) {
      StringRef Styles[] = {"LLVM",   "Google", "Chromium",  "Mozilla",
                            "WebKit", "GNU",    "Microsoft", "clang-format"};
      for (StringRef StyleName : Styles) {
        FormatStyle PredefinedStyle;
        if (getPredefinedStyle(StyleName, Style.Language, &PredefinedStyle) &&
            Style == PredefinedStyle) {
          BasedOnStyle = StyleName;
          break;
        }
      }
    } else {
      IO.mapOptional("BasedOnStyle", BasedOnStyle);
      if (!BasedOnStyle.empty()) {
        FormatStyle::LanguageKind OldLanguage = Style.Language;
        FormatStyle::LanguageKind Language =
            ((FormatStyle *)IO.getContext())->Language;
        if (!getPredefinedStyle(BasedOnStyle, Language, &Style)) {
          throw std::runtime_error("Unknown value for BasedOnStyle: " +
                                   BasedOnStyle.str());
        }
        Style.Language = OldLanguage;
      }
    }

    // Initialize some variables used in the parsing. The using logic is at the
    // end.

    // For backward compatibility:
    // The default value of ConstructorInitializerAllOnOneLineOrOnePerLine was
    // false unless BasedOnStyle was Google or Chromium whereas that of
    // AllowAllConstructorInitializersOnNextLine was always true, so the
    // equivalent default value of PackConstructorInitializers is PCIS_NextLine
    // for Google/Chromium or PCIS_BinPack otherwise. If the deprecated options
    // had a non-default value while PackConstructorInitializers has a default
    // value, set the latter to an equivalent non-default value if needed.
    const bool IsGoogleOrChromium = BasedOnStyle.equals_insensitive("google") ||
                                    BasedOnStyle.equals_insensitive("chromium");
    bool OnCurrentLine = IsGoogleOrChromium;
    bool OnNextLine = true;

    bool BreakBeforeInheritanceComma = false;
    bool BreakConstructorInitializersBeforeComma = false;

    bool DeriveLineEnding = true;
    bool UseCRLF = false;

    bool SpaceInEmptyParentheses = false;
    bool SpacesInConditionalStatement = false;
    bool SpacesInCStyleCastParentheses = false;
    bool SpacesInParentheses = false;

    // For backward compatibility.
    if (!IO.outputting()) {
      IO.mapOptional("AlignEscapedNewlinesLeft", Style.AlignEscapedNewlines);
      IO.mapOptional("AllowAllConstructorInitializersOnNextLine", OnNextLine);
      IO.mapOptional("AlwaysBreakAfterReturnType", Style.BreakAfterReturnType);
      IO.mapOptional("AlwaysBreakTemplateDeclarations",
                     Style.BreakTemplateDeclarations);
      IO.mapOptional("BreakBeforeInheritanceComma",
                     BreakBeforeInheritanceComma);
      IO.mapOptional("BreakConstructorInitializersBeforeComma",
                     BreakConstructorInitializersBeforeComma);
      IO.mapOptional("ConstructorInitializerAllOnOneLineOrOnePerLine",
                     OnCurrentLine);
      IO.mapOptional("DeriveLineEnding", DeriveLineEnding);
      IO.mapOptional("DerivePointerBinding", Style.DerivePointerAlignment);
      IO.mapOptional("KeepEmptyLinesAtEOF", Style.KeepEmptyLines.AtEndOfFile);
      IO.mapOptional("KeepEmptyLinesAtTheStartOfBlocks",
                     Style.KeepEmptyLines.AtStartOfBlock);
      IO.mapOptional("IndentFunctionDeclarationAfterType",
                     Style.IndentWrappedFunctionNames);
      IO.mapOptional("IndentRequires", Style.IndentRequiresClause);
      IO.mapOptional("PointerBindsToType", Style.PointerAlignment);
      IO.mapOptional("SpaceAfterControlStatementKeyword",
                     Style.SpaceBeforeParens);
      IO.mapOptional("SpaceInEmptyParentheses", SpaceInEmptyParentheses);
      IO.mapOptional("SpacesInConditionalStatement",
                     SpacesInConditionalStatement);
      IO.mapOptional("SpacesInCStyleCastParentheses",
                     SpacesInCStyleCastParentheses);
      IO.mapOptional("SpacesInParentheses", SpacesInParentheses);
      IO.mapOptional("UseCRLF", UseCRLF);
    }

    IO.mapOptional("AccessModifierOffset", Style.AccessModifierOffset);
    IO.mapOptional("AlignAfterOpenBracket", Style.AlignAfterOpenBracket);
    IO.mapOptional("AlignArrayOfStructures", Style.AlignArrayOfStructures);
    IO.mapOptional("AlignConsecutiveAssignments",
                   Style.AlignConsecutiveAssignments);
    IO.mapOptional("AlignConsecutiveBitFields",
                   Style.AlignConsecutiveBitFields);
    IO.mapOptional("AlignConsecutiveDeclarations",
                   Style.AlignConsecutiveDeclarations);
    IO.mapOptional("AlignConsecutiveMacros", Style.AlignConsecutiveMacros);
    IO.mapOptional("AlignConsecutiveShortCaseStatements",
                   Style.AlignConsecutiveShortCaseStatements);
    IO.mapOptional("AlignConsecutiveTableGenBreakingDAGArgColons",
                   Style.AlignConsecutiveTableGenBreakingDAGArgColons);
    IO.mapOptional("AlignConsecutiveTableGenCondOperatorColons",
                   Style.AlignConsecutiveTableGenCondOperatorColons);
    IO.mapOptional("AlignConsecutiveTableGenDefinitionColons",
                   Style.AlignConsecutiveTableGenDefinitionColons);
    IO.mapOptional("AlignEscapedNewlines", Style.AlignEscapedNewlines);
    IO.mapOptional("AlignOperands", Style.AlignOperands);
    IO.mapOptional("AlignTrailingComments", Style.AlignTrailingComments);
    IO.mapOptional("AllowAllArgumentsOnNextLine",
                   Style.AllowAllArgumentsOnNextLine);
    IO.mapOptional("AllowAllParametersOfDeclarationOnNextLine",
                   Style.AllowAllParametersOfDeclarationOnNextLine);
    IO.mapOptional("AllowBreakBeforeNoexceptSpecifier",
                   Style.AllowBreakBeforeNoexceptSpecifier);
    IO.mapOptional("AllowShortBlocksOnASingleLine",
                   Style.AllowShortBlocksOnASingleLine);
    IO.mapOptional("AllowShortCaseExpressionOnASingleLine",
                   Style.AllowShortCaseExpressionOnASingleLine);
    IO.mapOptional("AllowShortCaseLabelsOnASingleLine",
                   Style.AllowShortCaseLabelsOnASingleLine);
    IO.mapOptional("AllowShortCompoundRequirementOnASingleLine",
                   Style.AllowShortCompoundRequirementOnASingleLine);
    IO.mapOptional("AllowShortEnumsOnASingleLine",
                   Style.AllowShortEnumsOnASingleLine);
    IO.mapOptional("AllowShortFunctionsOnASingleLine",
                   Style.AllowShortFunctionsOnASingleLine);
    IO.mapOptional("AllowShortIfStatementsOnASingleLine",
                   Style.AllowShortIfStatementsOnASingleLine);
    IO.mapOptional("AllowShortLambdasOnASingleLine",
                   Style.AllowShortLambdasOnASingleLine);
    IO.mapOptional("AllowShortLoopsOnASingleLine",
                   Style.AllowShortLoopsOnASingleLine);
    IO.mapOptional("AlwaysBreakAfterDefinitionReturnType",
                   Style.AlwaysBreakAfterDefinitionReturnType);
    IO.mapOptional("AlwaysBreakBeforeMultilineStrings",
                   Style.AlwaysBreakBeforeMultilineStrings);
    IO.mapOptional("AttributeMacros", Style.AttributeMacros);
    IO.mapOptional("BinPackArguments", Style.BinPackArguments);
    IO.mapOptional("BinPackParameters", Style.BinPackParameters);
    IO.mapOptional("BitFieldColonSpacing", Style.BitFieldColonSpacing);
    IO.mapOptional("BracedInitializerIndentWidth",
                   Style.BracedInitializerIndentWidth);
    IO.mapOptional("BraceWrapping", Style.BraceWrapping);
    IO.mapOptional("BreakAdjacentStringLiterals",
                   Style.BreakAdjacentStringLiterals);
    IO.mapOptional("BreakAfterAttributes", Style.BreakAfterAttributes);
    IO.mapOptional("BreakAfterJavaFieldAnnotations",
                   Style.BreakAfterJavaFieldAnnotations);
    IO.mapOptional("BreakAfterReturnType", Style.BreakAfterReturnType);
    IO.mapOptional("BreakArrays", Style.BreakArrays);
    IO.mapOptional("BreakBeforeBinaryOperators",
                   Style.BreakBeforeBinaryOperators);
    IO.mapOptional("BreakBeforeConceptDeclarations",
                   Style.BreakBeforeConceptDeclarations);
    IO.mapOptional("BreakBeforeBraces", Style.BreakBeforeBraces);
    IO.mapOptional("BreakBeforeInlineASMColon",
                   Style.BreakBeforeInlineASMColon);
    IO.mapOptional("BreakBeforeTernaryOperators",
                   Style.BreakBeforeTernaryOperators);
    IO.mapOptional("BreakConstructorInitializers",
                   Style.BreakConstructorInitializers);
    IO.mapOptional("BreakFunctionDefinitionParameters",
                   Style.BreakFunctionDefinitionParameters);
    IO.mapOptional("BreakInheritanceList", Style.BreakInheritanceList);
    IO.mapOptional("BreakStringLiterals", Style.BreakStringLiterals);
    IO.mapOptional("BreakTemplateDeclarations",
                   Style.BreakTemplateDeclarations);
    IO.mapOptional("ColumnLimit", Style.ColumnLimit);
    IO.mapOptional("CommentPragmas", Style.CommentPragmas);
    IO.mapOptional("CompactNamespaces", Style.CompactNamespaces);
    IO.mapOptional("ConstructorInitializerIndentWidth",
                   Style.ConstructorInitializerIndentWidth);
    IO.mapOptional("ContinuationIndentWidth", Style.ContinuationIndentWidth);
    IO.mapOptional("Cpp11BracedListStyle", Style.Cpp11BracedListStyle);
    IO.mapOptional("DerivePointerAlignment", Style.DerivePointerAlignment);
    IO.mapOptional("DisableFormat", Style.DisableFormat);
    IO.mapOptional("EmptyLineAfterAccessModifier",
                   Style.EmptyLineAfterAccessModifier);
    IO.mapOptional("EmptyLineBeforeAccessModifier",
                   Style.EmptyLineBeforeAccessModifier);
    IO.mapOptional("ExperimentalAutoDetectBinPacking",
                   Style.ExperimentalAutoDetectBinPacking);
    IO.mapOptional("FixNamespaceComments", Style.FixNamespaceComments);
    IO.mapOptional("ForEachMacros", Style.ForEachMacros);
    IO.mapOptional("IfMacros", Style.IfMacros);
    IO.mapOptional("IncludeBlocks", Style.IncludeStyle.IncludeBlocks);
    IO.mapOptional("IncludeCategories", Style.IncludeStyle.IncludeCategories);
    IO.mapOptional("IncludeIsMainRegex", Style.IncludeStyle.IncludeIsMainRegex);
    IO.mapOptional("IncludeIsMainSourceRegex",
                   Style.IncludeStyle.IncludeIsMainSourceRegex);
    IO.mapOptional("IndentAccessModifiers", Style.IndentAccessModifiers);
    IO.mapOptional("IndentCaseBlocks", Style.IndentCaseBlocks);
    IO.mapOptional("IndentCaseLabels", Style.IndentCaseLabels);
    IO.mapOptional("IndentExternBlock", Style.IndentExternBlock);
    IO.mapOptional("IndentGotoLabels", Style.IndentGotoLabels);
    IO.mapOptional("IndentPPDirectives", Style.IndentPPDirectives);
    IO.mapOptional("IndentRequiresClause", Style.IndentRequiresClause);
    IO.mapOptional("IndentWidth", Style.IndentWidth);
    IO.mapOptional("IndentWrappedFunctionNames",
                   Style.IndentWrappedFunctionNames);
    IO.mapOptional("InsertBraces", Style.InsertBraces);
    IO.mapOptional("InsertNewlineAtEOF", Style.InsertNewlineAtEOF);
    IO.mapOptional("InsertTrailingCommas", Style.InsertTrailingCommas);
    IO.mapOptional("IntegerLiteralSeparator", Style.IntegerLiteralSeparator);
    IO.mapOptional("JavaImportGroups", Style.JavaImportGroups);
    IO.mapOptional("JavaScriptQuotes", Style.JavaScriptQuotes);
    IO.mapOptional("JavaScriptWrapImports", Style.JavaScriptWrapImports);
    IO.mapOptional("KeepEmptyLines", Style.KeepEmptyLines);
    IO.mapOptional("LambdaBodyIndentation", Style.LambdaBodyIndentation);
    IO.mapOptional("LineEnding", Style.LineEnding);
    IO.mapOptional("MacroBlockBegin", Style.MacroBlockBegin);
    IO.mapOptional("MacroBlockEnd", Style.MacroBlockEnd);
    IO.mapOptional("Macros", Style.Macros);
    IO.mapOptional("MainIncludeChar", Style.IncludeStyle.MainIncludeChar);
    IO.mapOptional("MaxEmptyLinesToKeep", Style.MaxEmptyLinesToKeep);
    IO.mapOptional("NamespaceIndentation", Style.NamespaceIndentation);
    IO.mapOptional("NamespaceMacros", Style.NamespaceMacros);
    IO.mapOptional("ObjCBinPackProtocolList", Style.ObjCBinPackProtocolList);
    IO.mapOptional("ObjCBlockIndentWidth", Style.ObjCBlockIndentWidth);
    IO.mapOptional("ObjCBreakBeforeNestedBlockParam",
                   Style.ObjCBreakBeforeNestedBlockParam);
    IO.mapOptional("ObjCPropertyAttributeOrder",
                   Style.ObjCPropertyAttributeOrder);
    IO.mapOptional("ObjCSpaceAfterProperty", Style.ObjCSpaceAfterProperty);
    IO.mapOptional("ObjCSpaceBeforeProtocolList",
                   Style.ObjCSpaceBeforeProtocolList);
    IO.mapOptional("PackConstructorInitializers",
                   Style.PackConstructorInitializers);
    IO.mapOptional("PenaltyBreakAssignment", Style.PenaltyBreakAssignment);
    IO.mapOptional("PenaltyBreakBeforeFirstCallParameter",
                   Style.PenaltyBreakBeforeFirstCallParameter);
    IO.mapOptional("PenaltyBreakComment", Style.PenaltyBreakComment);
    IO.mapOptional("PenaltyBreakFirstLessLess",
                   Style.PenaltyBreakFirstLessLess);
    IO.mapOptional("PenaltyBreakOpenParenthesis",
                   Style.PenaltyBreakOpenParenthesis);
    IO.mapOptional("PenaltyBreakScopeResolution",
                   Style.PenaltyBreakScopeResolution);
    IO.mapOptional("PenaltyBreakString", Style.PenaltyBreakString);
    IO.mapOptional("PenaltyBreakTemplateDeclaration",
                   Style.PenaltyBreakTemplateDeclaration);
    IO.mapOptional("PenaltyExcessCharacter", Style.PenaltyExcessCharacter);
    IO.mapOptional("PenaltyIndentedWhitespace",
                   Style.PenaltyIndentedWhitespace);
    IO.mapOptional("PenaltyReturnTypeOnItsOwnLine",
                   Style.PenaltyReturnTypeOnItsOwnLine);
    IO.mapOptional("PointerAlignment", Style.PointerAlignment);
    IO.mapOptional("PPIndentWidth", Style.PPIndentWidth);
    IO.mapOptional("QualifierAlignment", Style.QualifierAlignment);
    // Default Order for Left/Right based Qualifier alignment.
    if (Style.QualifierAlignment == FormatStyle::QAS_Right)
      Style.QualifierOrder = {"type", "const", "volatile"};
    else if (Style.QualifierAlignment == FormatStyle::QAS_Left)
      Style.QualifierOrder = {"const", "volatile", "type"};
    else if (Style.QualifierAlignment == FormatStyle::QAS_Custom)
      IO.mapOptional("QualifierOrder", Style.QualifierOrder);
    IO.mapOptional("RawStringFormats", Style.RawStringFormats);
    IO.mapOptional("ReferenceAlignment", Style.ReferenceAlignment);
    IO.mapOptional("ReflowComments", Style.ReflowComments);
    IO.mapOptional("RemoveBracesLLVM", Style.RemoveBracesLLVM);
    IO.mapOptional("RemoveParentheses", Style.RemoveParentheses);
    IO.mapOptional("RemoveSemicolon", Style.RemoveSemicolon);
    IO.mapOptional("RequiresClausePosition", Style.RequiresClausePosition);
    IO.mapOptional("RequiresExpressionIndentation",
                   Style.RequiresExpressionIndentation);
    IO.mapOptional("SeparateDefinitionBlocks", Style.SeparateDefinitionBlocks);
    IO.mapOptional("ShortNamespaceLines", Style.ShortNamespaceLines);
    IO.mapOptional("SkipMacroDefinitionBody", Style.SkipMacroDefinitionBody);
    IO.mapOptional("SortIncludes", Style.SortIncludes);
    IO.mapOptional("SortJavaStaticImport", Style.SortJavaStaticImport);
    IO.mapOptional("SortUsingDeclarations", Style.SortUsingDeclarations);
    IO.mapOptional("SpaceAfterCStyleCast", Style.SpaceAfterCStyleCast);
    IO.mapOptional("SpaceAfterLogicalNot", Style.SpaceAfterLogicalNot);
    IO.mapOptional("SpaceAfterTemplateKeyword",
                   Style.SpaceAfterTemplateKeyword);
    IO.mapOptional("SpaceAroundPointerQualifiers",
                   Style.SpaceAroundPointerQualifiers);
    IO.mapOptional("SpaceBeforeAssignmentOperators",
                   Style.SpaceBeforeAssignmentOperators);
    IO.mapOptional("SpaceBeforeCaseColon", Style.SpaceBeforeCaseColon);
    IO.mapOptional("SpaceBeforeCpp11BracedList",
                   Style.SpaceBeforeCpp11BracedList);
    IO.mapOptional("SpaceBeforeCtorInitializerColon",
                   Style.SpaceBeforeCtorInitializerColon);
    IO.mapOptional("SpaceBeforeInheritanceColon",
                   Style.SpaceBeforeInheritanceColon);
    IO.mapOptional("SpaceBeforeJsonColon", Style.SpaceBeforeJsonColon);
    IO.mapOptional("SpaceBeforeParens", Style.SpaceBeforeParens);
    IO.mapOptional("SpaceBeforeParensOptions", Style.SpaceBeforeParensOptions);
    IO.mapOptional("SpaceBeforeRangeBasedForLoopColon",
                   Style.SpaceBeforeRangeBasedForLoopColon);
    IO.mapOptional("SpaceBeforeSquareBrackets",
                   Style.SpaceBeforeSquareBrackets);
    IO.mapOptional("SpaceInEmptyBlock", Style.SpaceInEmptyBlock);
    IO.mapOptional("SpacesBeforeTrailingComments",
                   Style.SpacesBeforeTrailingComments);
    IO.mapOptional("SpacesInAngles", Style.SpacesInAngles);
    IO.mapOptional("SpacesInContainerLiterals",
                   Style.SpacesInContainerLiterals);
    IO.mapOptional("SpacesInLineCommentPrefix",
                   Style.SpacesInLineCommentPrefix);
    IO.mapOptional("SpacesInParens", Style.SpacesInParens);
    IO.mapOptional("SpacesInParensOptions", Style.SpacesInParensOptions);
    IO.mapOptional("SpacesInSquareBrackets", Style.SpacesInSquareBrackets);
    IO.mapOptional("Standard", Style.Standard);
    IO.mapOptional("StatementAttributeLikeMacros",
                   Style.StatementAttributeLikeMacros);
    IO.mapOptional("StatementMacros", Style.StatementMacros);
    IO.mapOptional("TableGenBreakingDAGArgOperators",
                   Style.TableGenBreakingDAGArgOperators);
    IO.mapOptional("TableGenBreakInsideDAGArg",
                   Style.TableGenBreakInsideDAGArg);
    IO.mapOptional("TabWidth", Style.TabWidth);
    IO.mapOptional("TypeNames", Style.TypeNames);
    IO.mapOptional("TypenameMacros", Style.TypenameMacros);
    IO.mapOptional("UseTab", Style.UseTab);
    IO.mapOptional("VerilogBreakBetweenInstancePorts",
                   Style.VerilogBreakBetweenInstancePorts);
    IO.mapOptional("WhitespaceSensitiveMacros",
                   Style.WhitespaceSensitiveMacros);

    // If AlwaysBreakAfterDefinitionReturnType was specified but
    // BreakAfterReturnType was not, initialize the latter from the former for
    // backwards compatibility.
    if (Style.AlwaysBreakAfterDefinitionReturnType != FormatStyle::DRTBS_None &&
        Style.BreakAfterReturnType == FormatStyle::RTBS_None) {
      if (Style.AlwaysBreakAfterDefinitionReturnType ==
          FormatStyle::DRTBS_All) {
        Style.BreakAfterReturnType = FormatStyle::RTBS_AllDefinitions;
      } else if (Style.AlwaysBreakAfterDefinitionReturnType ==
                 FormatStyle::DRTBS_TopLevel) {
        Style.BreakAfterReturnType = FormatStyle::RTBS_TopLevelDefinitions;
      }
    }

    // If BreakBeforeInheritanceComma was specified but BreakInheritance was
    // not, initialize the latter from the former for backwards compatibility.
    if (BreakBeforeInheritanceComma &&
        Style.BreakInheritanceList == FormatStyle::BILS_BeforeColon) {
      Style.BreakInheritanceList = FormatStyle::BILS_BeforeComma;
    }

    // If BreakConstructorInitializersBeforeComma was specified but
    // BreakConstructorInitializers was not, initialize the latter from the
    // former for backwards compatibility.
    if (BreakConstructorInitializersBeforeComma &&
        Style.BreakConstructorInitializers == FormatStyle::BCIS_BeforeColon) {
      Style.BreakConstructorInitializers = FormatStyle::BCIS_BeforeComma;
    }

    if (!IsGoogleOrChromium) {
      if (Style.PackConstructorInitializers == FormatStyle::PCIS_BinPack &&
          OnCurrentLine) {
        Style.PackConstructorInitializers = OnNextLine
                                                ? FormatStyle::PCIS_NextLine
                                                : FormatStyle::PCIS_CurrentLine;
      }
    } else if (Style.PackConstructorInitializers ==
               FormatStyle::PCIS_NextLine) {
      if (!OnCurrentLine)
        Style.PackConstructorInitializers = FormatStyle::PCIS_BinPack;
      else if (!OnNextLine)
        Style.PackConstructorInitializers = FormatStyle::PCIS_CurrentLine;
    }

    if (Style.LineEnding == FormatStyle::LE_DeriveLF) {
      if (!DeriveLineEnding)
        Style.LineEnding = UseCRLF ? FormatStyle::LE_CRLF : FormatStyle::LE_LF;
      else if (UseCRLF)
        Style.LineEnding = FormatStyle::LE_DeriveCRLF;
    }

    if (Style.SpacesInParens != FormatStyle::SIPO_Custom &&
        (SpacesInParentheses || SpaceInEmptyParentheses ||
         SpacesInConditionalStatement || SpacesInCStyleCastParentheses)) {
      if (SpacesInParentheses) {
        // For backward compatibility.
        Style.SpacesInParensOptions.ExceptDoubleParentheses = false;
        Style.SpacesInParensOptions.InConditionalStatements = true;
        Style.SpacesInParensOptions.InCStyleCasts =
            SpacesInCStyleCastParentheses;
        Style.SpacesInParensOptions.InEmptyParentheses =
            SpaceInEmptyParentheses;
        Style.SpacesInParensOptions.Other = true;
      } else {
        Style.SpacesInParensOptions = {};
        Style.SpacesInParensOptions.InConditionalStatements =
            SpacesInConditionalStatement;
        Style.SpacesInParensOptions.InCStyleCasts =
            SpacesInCStyleCastParentheses;
        Style.SpacesInParensOptions.InEmptyParentheses =
            SpaceInEmptyParentheses;
      }
      Style.SpacesInParens = FormatStyle::SIPO_Custom;
    }
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

namespace clang_v19 {

static tok::TokenKind getTokenFromQualifier(const std::string &Qualifier) {
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
  case ParseError::BinPackTrailingCommaConflict:
    return "trailing comma insertion cannot be used with bin packing";
  case ParseError::InvalidQualifierSpecified:
    return "Invalid qualifier specified in QualifierOrder";
  case ParseError::DuplicateQualifierSpecified:
    return "Duplicate qualifier specified in QualifierOrder";
  case ParseError::MissingQualifierType:
    return "Missing type in QualifierOrder";
  case ParseError::MissingQualifierOrder:
    return "Missing QualifierOrder";
  }
  llvm_unreachable("unexpected parse error");
}

static void expandPresetsBraceWrapping(FormatStyle &Expanded) {
  if (Expanded.BreakBeforeBraces == FormatStyle::BS_Custom)
    return;
  Expanded.BraceWrapping = {/*AfterCaseLabel=*/false,
                            /*AfterClass=*/false,
                            /*AfterControlStatement=*/FormatStyle::BWACS_Never,
                            /*AfterEnum=*/false,
                            /*AfterFunction=*/false,
                            /*AfterNamespace=*/false,
                            /*AfterObjCDeclaration=*/false,
                            /*AfterStruct=*/false,
                            /*AfterUnion=*/false,
                            /*AfterExternBlock=*/false,
                            /*BeforeCatch=*/false,
                            /*BeforeElse=*/false,
                            /*BeforeLambdaBody=*/false,
                            /*BeforeWhile=*/false,
                            /*IndentBraces=*/false,
                            /*SplitEmptyFunction=*/true,
                            /*SplitEmptyRecord=*/true,
                            /*SplitEmptyNamespace=*/true};
  switch (Expanded.BreakBeforeBraces) {
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
    Expanded.BraceWrapping.AfterCaseLabel = true;
    Expanded.BraceWrapping.AfterClass = true;
    Expanded.BraceWrapping.AfterControlStatement = FormatStyle::BWACS_Always;
    Expanded.BraceWrapping.AfterEnum = true;
    Expanded.BraceWrapping.AfterFunction = true;
    Expanded.BraceWrapping.AfterNamespace = true;
    Expanded.BraceWrapping.AfterObjCDeclaration = true;
    Expanded.BraceWrapping.AfterStruct = true;
    Expanded.BraceWrapping.AfterUnion = true;
    Expanded.BraceWrapping.AfterExternBlock = true;
    Expanded.BraceWrapping.BeforeCatch = true;
    Expanded.BraceWrapping.BeforeElse = true;
    Expanded.BraceWrapping.BeforeLambdaBody = true;
    break;
  case FormatStyle::BS_Whitesmiths:
    Expanded.BraceWrapping.AfterCaseLabel = true;
    Expanded.BraceWrapping.AfterClass = true;
    Expanded.BraceWrapping.AfterControlStatement = FormatStyle::BWACS_Always;
    Expanded.BraceWrapping.AfterEnum = true;
    Expanded.BraceWrapping.AfterFunction = true;
    Expanded.BraceWrapping.AfterNamespace = true;
    Expanded.BraceWrapping.AfterObjCDeclaration = true;
    Expanded.BraceWrapping.AfterStruct = true;
    Expanded.BraceWrapping.AfterExternBlock = true;
    Expanded.BraceWrapping.BeforeCatch = true;
    Expanded.BraceWrapping.BeforeElse = true;
    Expanded.BraceWrapping.BeforeLambdaBody = true;
    break;
  case FormatStyle::BS_GNU:
    Expanded.BraceWrapping = {
        /*AfterCaseLabel=*/true,
        /*AfterClass=*/true,
        /*AfterControlStatement=*/FormatStyle::BWACS_Always,
        /*AfterEnum=*/true,
        /*AfterFunction=*/true,
        /*AfterNamespace=*/true,
        /*AfterObjCDeclaration=*/true,
        /*AfterStruct=*/true,
        /*AfterUnion=*/true,
        /*AfterExternBlock=*/true,
        /*BeforeCatch=*/true,
        /*BeforeElse=*/true,
        /*BeforeLambdaBody=*/false,
        /*BeforeWhile=*/true,
        /*IndentBraces=*/true,
        /*SplitEmptyFunction=*/true,
        /*SplitEmptyRecord=*/true,
        /*SplitEmptyNamespace=*/true};
    break;
  case FormatStyle::BS_WebKit:
    Expanded.BraceWrapping.AfterFunction = true;
    break;
  default:
    break;
  }
}

static void expandPresetsSpaceBeforeParens(FormatStyle &Expanded) {
  if (Expanded.SpaceBeforeParens == FormatStyle::SBPO_Custom)
    return;
  // Reset all flags
  Expanded.SpaceBeforeParensOptions = {};
  Expanded.SpaceBeforeParensOptions.AfterPlacementOperator = true;

  switch (Expanded.SpaceBeforeParens) {
  case FormatStyle::SBPO_ControlStatements:
    Expanded.SpaceBeforeParensOptions.AfterControlStatements = true;
    Expanded.SpaceBeforeParensOptions.AfterForeachMacros = true;
    Expanded.SpaceBeforeParensOptions.AfterIfMacros = true;
    break;
  case FormatStyle::SBPO_ControlStatementsExceptControlMacros:
    Expanded.SpaceBeforeParensOptions.AfterControlStatements = true;
    break;
  case FormatStyle::SBPO_NonEmptyParentheses:
    Expanded.SpaceBeforeParensOptions.BeforeNonEmptyParentheses = true;
    break;
  default:
    break;
  }
}

static void expandPresetsSpacesInParens(FormatStyle &Expanded) {
  if (Expanded.SpacesInParens == FormatStyle::SIPO_Custom)
    return;
  assert(Expanded.SpacesInParens == FormatStyle::SIPO_Never);
  // Reset all flags
  Expanded.SpacesInParensOptions = {};
}

FormatStyle getLLVMStyle(FormatStyle::LanguageKind Language) {
  FormatStyle LLVMStyle;
  LLVMStyle.AccessModifierOffset = -2;
  LLVMStyle.AlignAfterOpenBracket = FormatStyle::BAS_Align;
  LLVMStyle.AlignArrayOfStructures = FormatStyle::AIAS_None;
  LLVMStyle.AlignConsecutiveAssignments = {};
  LLVMStyle.AlignConsecutiveAssignments.AcrossComments = false;
  LLVMStyle.AlignConsecutiveAssignments.AcrossEmptyLines = false;
  LLVMStyle.AlignConsecutiveAssignments.AlignCompound = false;
  LLVMStyle.AlignConsecutiveAssignments.AlignFunctionPointers = false;
  LLVMStyle.AlignConsecutiveAssignments.Enabled = false;
  LLVMStyle.AlignConsecutiveAssignments.PadOperators = true;
  LLVMStyle.AlignConsecutiveBitFields = {};
  LLVMStyle.AlignConsecutiveDeclarations = {};
  LLVMStyle.AlignConsecutiveMacros = {};
  LLVMStyle.AlignConsecutiveShortCaseStatements = {};
  LLVMStyle.AlignConsecutiveTableGenBreakingDAGArgColons = {};
  LLVMStyle.AlignConsecutiveTableGenCondOperatorColons = {};
  LLVMStyle.AlignConsecutiveTableGenDefinitionColons = {};
  LLVMStyle.AlignEscapedNewlines = FormatStyle::ENAS_Right;
  LLVMStyle.AlignOperands = FormatStyle::OAS_Align;
  LLVMStyle.AlignTrailingComments = {};
  LLVMStyle.AlignTrailingComments.Kind = FormatStyle::TCAS_Always;
  LLVMStyle.AlignTrailingComments.OverEmptyLines = 0;
  LLVMStyle.AllowAllArgumentsOnNextLine = true;
  LLVMStyle.AllowAllParametersOfDeclarationOnNextLine = true;
  LLVMStyle.AllowBreakBeforeNoexceptSpecifier = FormatStyle::BBNSS_Never;
  LLVMStyle.AllowShortBlocksOnASingleLine = FormatStyle::SBS_Never;
  LLVMStyle.AllowShortCaseExpressionOnASingleLine = true;
  LLVMStyle.AllowShortCaseLabelsOnASingleLine = false;
  LLVMStyle.AllowShortCompoundRequirementOnASingleLine = true;
  LLVMStyle.AllowShortEnumsOnASingleLine = true;
  LLVMStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_All;
  LLVMStyle.AllowShortIfStatementsOnASingleLine = FormatStyle::SIS_Never;
  LLVMStyle.AllowShortLambdasOnASingleLine = FormatStyle::SLS_All;
  LLVMStyle.AllowShortLoopsOnASingleLine = false;
  LLVMStyle.AlwaysBreakAfterDefinitionReturnType = FormatStyle::DRTBS_None;
  LLVMStyle.AlwaysBreakBeforeMultilineStrings = false;
  LLVMStyle.AttributeMacros.push_back("__capability");
  LLVMStyle.BinPackArguments = true;
  LLVMStyle.BinPackParameters = true;
  LLVMStyle.BitFieldColonSpacing = FormatStyle::BFCS_Both;
  LLVMStyle.BracedInitializerIndentWidth = std::nullopt;
  LLVMStyle.BraceWrapping = {/*AfterCaseLabel=*/false,
                             /*AfterClass=*/false,
                             /*AfterControlStatement=*/FormatStyle::BWACS_Never,
                             /*AfterEnum=*/false,
                             /*AfterFunction=*/false,
                             /*AfterNamespace=*/false,
                             /*AfterObjCDeclaration=*/false,
                             /*AfterStruct=*/false,
                             /*AfterUnion=*/false,
                             /*AfterExternBlock=*/false,
                             /*BeforeCatch=*/false,
                             /*BeforeElse=*/false,
                             /*BeforeLambdaBody=*/false,
                             /*BeforeWhile=*/false,
                             /*IndentBraces=*/false,
                             /*SplitEmptyFunction=*/true,
                             /*SplitEmptyRecord=*/true,
                             /*SplitEmptyNamespace=*/true};
  LLVMStyle.BreakAdjacentStringLiterals = true;
  LLVMStyle.BreakAfterAttributes = FormatStyle::ABS_Leave;
  LLVMStyle.BreakAfterJavaFieldAnnotations = false;
  LLVMStyle.BreakAfterReturnType = FormatStyle::RTBS_None;
  LLVMStyle.BreakArrays = true;
  LLVMStyle.BreakBeforeBinaryOperators = FormatStyle::BOS_None;
  LLVMStyle.BreakBeforeBraces = FormatStyle::BS_Attach;
  LLVMStyle.BreakBeforeConceptDeclarations = FormatStyle::BBCDS_Always;
  LLVMStyle.BreakBeforeInlineASMColon = FormatStyle::BBIAS_OnlyMultiline;
  LLVMStyle.BreakBeforeTernaryOperators = true;
  LLVMStyle.BreakConstructorInitializers = FormatStyle::BCIS_BeforeColon;
  LLVMStyle.BreakFunctionDefinitionParameters = false;
  LLVMStyle.BreakInheritanceList = FormatStyle::BILS_BeforeColon;
  LLVMStyle.BreakStringLiterals = true;
  LLVMStyle.BreakTemplateDeclarations = FormatStyle::BTDS_MultiLine;
  LLVMStyle.ColumnLimit = 80;
  LLVMStyle.CommentPragmas = "^ IWYU pragma:";
  LLVMStyle.CompactNamespaces = false;
  LLVMStyle.ConstructorInitializerIndentWidth = 4;
  LLVMStyle.ContinuationIndentWidth = 4;
  LLVMStyle.Cpp11BracedListStyle = true;
  LLVMStyle.DerivePointerAlignment = false;
  LLVMStyle.DisableFormat = false;
  LLVMStyle.EmptyLineAfterAccessModifier = FormatStyle::ELAAMS_Never;
  LLVMStyle.EmptyLineBeforeAccessModifier = FormatStyle::ELBAMS_LogicalBlock;
  LLVMStyle.ExperimentalAutoDetectBinPacking = false;
  LLVMStyle.FixNamespaceComments = true;
  LLVMStyle.ForEachMacros.push_back("foreach");
  LLVMStyle.ForEachMacros.push_back("Q_FOREACH");
  LLVMStyle.ForEachMacros.push_back("BOOST_FOREACH");
  LLVMStyle.IfMacros.push_back("KJ_IF_MAYBE");
  LLVMStyle.IncludeStyle.IncludeBlocks = clang_v19::IncludeStyle::IBS_Preserve;
  LLVMStyle.IncludeStyle.IncludeCategories = {
      {"^\"(llvm|llvm-c|clang|clang-c)/", 2, 0, false},
      {"^(<|\"(gtest|gmock|isl|json)/)", 3, 0, false},
      {".*", 1, 0, false}};
  LLVMStyle.IncludeStyle.IncludeIsMainRegex = "(Test)?$";
  LLVMStyle.IncludeStyle.MainIncludeChar = clang_v19::IncludeStyle::MICD_Quote;
  LLVMStyle.IndentAccessModifiers = false;
  LLVMStyle.IndentCaseBlocks = false;
  LLVMStyle.IndentCaseLabels = false;
  LLVMStyle.IndentExternBlock = FormatStyle::IEBS_AfterExternBlock;
  LLVMStyle.IndentGotoLabels = true;
  LLVMStyle.IndentPPDirectives = FormatStyle::PPDIS_None;
  LLVMStyle.IndentRequiresClause = true;
  LLVMStyle.IndentWidth = 2;
  LLVMStyle.IndentWrappedFunctionNames = false;
  LLVMStyle.InheritsParentConfig = false;
  LLVMStyle.InsertBraces = false;
  LLVMStyle.InsertNewlineAtEOF = false;
  LLVMStyle.InsertTrailingCommas = FormatStyle::TCS_None;
  LLVMStyle.IntegerLiteralSeparator = {
      /*Binary=*/0,  /*BinaryMinDigits=*/0,
      /*Decimal=*/0, /*DecimalMinDigits=*/0,
      /*Hex=*/0,     /*HexMinDigits=*/0};
  LLVMStyle.JavaScriptQuotes = FormatStyle::JSQS_Leave;
  LLVMStyle.JavaScriptWrapImports = true;
  LLVMStyle.KeepEmptyLines = {
      /*AtEndOfFile=*/false,
      /*AtStartOfBlock=*/true,
      /*AtStartOfFile=*/true,
  };
  LLVMStyle.LambdaBodyIndentation = FormatStyle::LBI_Signature;
  LLVMStyle.Language = Language;
  LLVMStyle.LineEnding = FormatStyle::LE_DeriveLF;
  LLVMStyle.MaxEmptyLinesToKeep = 1;
  LLVMStyle.NamespaceIndentation = FormatStyle::NI_None;
  LLVMStyle.ObjCBinPackProtocolList = FormatStyle::BPS_Auto;
  LLVMStyle.ObjCBlockIndentWidth = 2;
  LLVMStyle.ObjCBreakBeforeNestedBlockParam = true;
  LLVMStyle.ObjCSpaceAfterProperty = false;
  LLVMStyle.ObjCSpaceBeforeProtocolList = true;
  LLVMStyle.PackConstructorInitializers = FormatStyle::PCIS_BinPack;
  LLVMStyle.PointerAlignment = FormatStyle::PAS_Right;
  LLVMStyle.PPIndentWidth = -1;
  LLVMStyle.QualifierAlignment = FormatStyle::QAS_Leave;
  LLVMStyle.ReferenceAlignment = FormatStyle::RAS_Pointer;
  LLVMStyle.ReflowComments = true;
  LLVMStyle.RemoveBracesLLVM = false;
  LLVMStyle.RemoveParentheses = FormatStyle::RPS_Leave;
  LLVMStyle.RemoveSemicolon = false;
  LLVMStyle.RequiresClausePosition = FormatStyle::RCPS_OwnLine;
  LLVMStyle.RequiresExpressionIndentation = FormatStyle::REI_OuterScope;
  LLVMStyle.SeparateDefinitionBlocks = FormatStyle::SDS_Leave;
  LLVMStyle.ShortNamespaceLines = 1;
  LLVMStyle.SkipMacroDefinitionBody = false;
  LLVMStyle.SortIncludes = FormatStyle::SI_CaseSensitive;
  LLVMStyle.SortJavaStaticImport = FormatStyle::SJSIO_Before;
  LLVMStyle.SortUsingDeclarations = FormatStyle::SUD_LexicographicNumeric;
  LLVMStyle.SpaceAfterCStyleCast = false;
  LLVMStyle.SpaceAfterLogicalNot = false;
  LLVMStyle.SpaceAfterTemplateKeyword = true;
  LLVMStyle.SpaceAroundPointerQualifiers = FormatStyle::SAPQ_Default;
  LLVMStyle.SpaceBeforeAssignmentOperators = true;
  LLVMStyle.SpaceBeforeCaseColon = false;
  LLVMStyle.SpaceBeforeCpp11BracedList = false;
  LLVMStyle.SpaceBeforeCtorInitializerColon = true;
  LLVMStyle.SpaceBeforeInheritanceColon = true;
  LLVMStyle.SpaceBeforeJsonColon = false;
  LLVMStyle.SpaceBeforeParens = FormatStyle::SBPO_ControlStatements;
  LLVMStyle.SpaceBeforeParensOptions = {};
  LLVMStyle.SpaceBeforeParensOptions.AfterControlStatements = true;
  LLVMStyle.SpaceBeforeParensOptions.AfterForeachMacros = true;
  LLVMStyle.SpaceBeforeParensOptions.AfterIfMacros = true;
  LLVMStyle.SpaceBeforeRangeBasedForLoopColon = true;
  LLVMStyle.SpaceBeforeSquareBrackets = false;
  LLVMStyle.SpaceInEmptyBlock = false;
  LLVMStyle.SpacesBeforeTrailingComments = 1;
  LLVMStyle.SpacesInAngles = FormatStyle::SIAS_Never;
  LLVMStyle.SpacesInContainerLiterals = true;
  LLVMStyle.SpacesInLineCommentPrefix = {/*Minimum=*/1, /*Maximum=*/-1u};
  LLVMStyle.SpacesInParens = FormatStyle::SIPO_Never;
  LLVMStyle.SpacesInSquareBrackets = false;
  LLVMStyle.Standard = FormatStyle::LS_Latest;
  LLVMStyle.StatementAttributeLikeMacros.push_back("Q_EMIT");
  LLVMStyle.StatementMacros.push_back("Q_UNUSED");
  LLVMStyle.StatementMacros.push_back("QT_REQUIRE_VERSION");
  LLVMStyle.TableGenBreakingDAGArgOperators = {};
  LLVMStyle.TableGenBreakInsideDAGArg = FormatStyle::DAS_DontBreak;
  LLVMStyle.TabWidth = 8;
  LLVMStyle.UseTab = FormatStyle::UT_Never;
  LLVMStyle.VerilogBreakBetweenInstancePorts = true;
  LLVMStyle.WhitespaceSensitiveMacros.push_back("BOOST_PP_STRINGIZE");
  LLVMStyle.WhitespaceSensitiveMacros.push_back("CF_SWIFT_NAME");
  LLVMStyle.WhitespaceSensitiveMacros.push_back("NS_SWIFT_NAME");
  LLVMStyle.WhitespaceSensitiveMacros.push_back("PP_STRINGIZE");
  LLVMStyle.WhitespaceSensitiveMacros.push_back("STRINGIZE");

  LLVMStyle.PenaltyBreakAssignment = prec::Assignment;
  LLVMStyle.PenaltyBreakBeforeFirstCallParameter = 19;
  LLVMStyle.PenaltyBreakComment = 300;
  LLVMStyle.PenaltyBreakFirstLessLess = 120;
  LLVMStyle.PenaltyBreakOpenParenthesis = 0;
  LLVMStyle.PenaltyBreakScopeResolution = 500;
  LLVMStyle.PenaltyBreakString = 1000;
  LLVMStyle.PenaltyBreakTemplateDeclaration = prec::Relational;
  LLVMStyle.PenaltyExcessCharacter = 1'000'000;
  LLVMStyle.PenaltyIndentedWhitespace = 0;
  LLVMStyle.PenaltyReturnTypeOnItsOwnLine = 60;

  // Defaults that differ when not C++.
  switch (Language) {
  case FormatStyle::LK_TableGen:
    LLVMStyle.SpacesInContainerLiterals = false;
    break;
  case FormatStyle::LK_Json:
    LLVMStyle.ColumnLimit = 0;
    break;
  case FormatStyle::LK_Verilog:
    LLVMStyle.IndentCaseLabels = true;
    LLVMStyle.SpacesInContainerLiterals = false;
    break;
  default:
    break;
  }

  return LLVMStyle;
}

FormatStyle getGoogleStyle(FormatStyle::LanguageKind Language) {
  if (Language == FormatStyle::LK_TextProto) {
    FormatStyle GoogleStyle = getGoogleStyle(FormatStyle::LK_Proto);
    GoogleStyle.Language = FormatStyle::LK_TextProto;

    return GoogleStyle;
  }

  FormatStyle GoogleStyle = getLLVMStyle(Language);

  GoogleStyle.AccessModifierOffset = -1;
  GoogleStyle.AlignEscapedNewlines = FormatStyle::ENAS_Left;
  GoogleStyle.AllowShortIfStatementsOnASingleLine =
      FormatStyle::SIS_WithoutElse;
  GoogleStyle.AllowShortLoopsOnASingleLine = true;
  GoogleStyle.AlwaysBreakBeforeMultilineStrings = true;
  GoogleStyle.BreakTemplateDeclarations = FormatStyle::BTDS_Yes;
  GoogleStyle.DerivePointerAlignment = true;
  GoogleStyle.IncludeStyle.IncludeBlocks = clang_v19::IncludeStyle::IBS_Regroup;
  GoogleStyle.IncludeStyle.IncludeCategories = {{"^<ext/.*\\.h>", 2, 0, false},
                                                {"^<.*\\.h>", 1, 0, false},
                                                {"^<.*", 2, 0, false},
                                                {".*", 3, 0, false}};
  GoogleStyle.IncludeStyle.IncludeIsMainRegex = "([-_](test|unittest))?$";
  GoogleStyle.IndentCaseLabels = true;
  GoogleStyle.KeepEmptyLines.AtStartOfBlock = false;
  GoogleStyle.ObjCBinPackProtocolList = FormatStyle::BPS_Never;
  GoogleStyle.ObjCSpaceAfterProperty = false;
  GoogleStyle.ObjCSpaceBeforeProtocolList = true;
  GoogleStyle.PackConstructorInitializers = FormatStyle::PCIS_NextLine;
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
              "ParseTestProto",
              "ParsePartialTestProto",
          },
          /*CanonicalDelimiter=*/"pb",
          /*BasedOnStyle=*/"google",
      },
  };

  GoogleStyle.SpacesBeforeTrailingComments = 2;
  GoogleStyle.Standard = FormatStyle::LS_Auto;

  GoogleStyle.PenaltyBreakBeforeFirstCallParameter = 1;
  GoogleStyle.PenaltyReturnTypeOnItsOwnLine = 200;

  if (Language == FormatStyle::LK_Java) {
    GoogleStyle.AlignAfterOpenBracket = FormatStyle::BAS_DontAlign;
    GoogleStyle.AlignOperands = FormatStyle::OAS_DontAlign;
    GoogleStyle.AlignTrailingComments = {};
    GoogleStyle.AlignTrailingComments.Kind = FormatStyle::TCAS_Never;
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Empty;
    GoogleStyle.AllowShortIfStatementsOnASingleLine = FormatStyle::SIS_Never;
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
    GoogleStyle.BreakBeforeBinaryOperators = FormatStyle::BOS_NonAssignment;
    GoogleStyle.ColumnLimit = 100;
    GoogleStyle.SpaceAfterCStyleCast = true;
    GoogleStyle.SpacesBeforeTrailingComments = 1;
  } else if (Language == FormatStyle::LK_JavaScript) {
    GoogleStyle.AlignAfterOpenBracket = FormatStyle::BAS_AlwaysBreak;
    GoogleStyle.AlignOperands = FormatStyle::OAS_DontAlign;
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Empty;
    // TODO: still under discussion whether to switch to SLS_All.
    GoogleStyle.AllowShortLambdasOnASingleLine = FormatStyle::SLS_Empty;
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
    GoogleStyle.BreakBeforeTernaryOperators = false;
    // taze:, triple slash directives (`/// <...`), tslint:, and @see, which is
    // commonly followed by overlong URLs.
    GoogleStyle.CommentPragmas = "(taze:|^/[ \t]*<|tslint:|@see)";
    // TODO: enable once decided, in particular re disabling bin packing.
    // https://google.github.io/styleguide/jsguide.html#features-arrays-trailing-comma
    // GoogleStyle.InsertTrailingCommas = FormatStyle::TCS_Wrapped;
    GoogleStyle.JavaScriptQuotes = FormatStyle::JSQS_Single;
    GoogleStyle.JavaScriptWrapImports = false;
    GoogleStyle.MaxEmptyLinesToKeep = 3;
    GoogleStyle.NamespaceIndentation = FormatStyle::NI_All;
    GoogleStyle.SpacesInContainerLiterals = false;
  } else if (Language == FormatStyle::LK_Proto) {
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Empty;
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
    // This affects protocol buffer options specifications and text protos.
    // Text protos are currently mostly formatted inside C++ raw string literals
    // and often the current breaking behavior of string literals is not
    // beneficial there. Investigate turning this on once proper string reflow
    // has been implemented.
    GoogleStyle.BreakStringLiterals = false;
    GoogleStyle.Cpp11BracedListStyle = false;
    GoogleStyle.SpacesInContainerLiterals = false;
  } else if (Language == FormatStyle::LK_ObjC) {
    GoogleStyle.AlwaysBreakBeforeMultilineStrings = false;
    GoogleStyle.ColumnLimit = 100;
    // "Regroup" doesn't work well for ObjC yet (main header heuristic,
    // relationship between ObjC standard library headers and other heades,
    // #imports, etc.)
    GoogleStyle.IncludeStyle.IncludeBlocks =
        clang_v19::IncludeStyle::IBS_Preserve;
  } else if (Language == FormatStyle::LK_CSharp) {
    GoogleStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Empty;
    GoogleStyle.AllowShortIfStatementsOnASingleLine = FormatStyle::SIS_Never;
    GoogleStyle.BreakStringLiterals = false;
    GoogleStyle.ColumnLimit = 100;
    GoogleStyle.NamespaceIndentation = FormatStyle::NI_All;
  }

  return GoogleStyle;
}

FormatStyle getChromiumStyle(FormatStyle::LanguageKind Language) {
  FormatStyle ChromiumStyle = getGoogleStyle(Language);

  // Disable include reordering across blocks in Chromium code.
  // - clang-format tries to detect that foo.h is the "main" header for
  //   foo.cc and foo_unittest.cc via IncludeIsMainRegex. However, Chromium
  //   uses many other suffices (_win.cc, _mac.mm, _posix.cc, _browsertest.cc,
  //   _private.cc, _impl.cc etc) in different permutations
  //   (_win_browsertest.cc) so disable this until IncludeIsMainRegex has a
  //   better default for Chromium code.
  // - The default for .cc and .mm files is different (r357695) for Google style
  //   for the same reason. The plan is to unify this again once the main
  //   header detection works for Google's ObjC code, but this hasn't happened
  //   yet. Since Chromium has some ObjC code, switching Chromium is blocked
  //   on that.
  // - Finally, "If include reordering is harmful, put things in different
  //   blocks to prevent it" has been a recommendation for a long time that
  //   people are used to. We'll need a dev education push to change this to
  //   "If include reordering is harmful, put things in a different block and
  //   _prepend that with a comment_ to prevent it" before changing behavior.
  ChromiumStyle.IncludeStyle.IncludeBlocks =
      clang_v19::IncludeStyle::IBS_Preserve;

  if (Language == FormatStyle::LK_Java) {
    ChromiumStyle.AllowShortIfStatementsOnASingleLine =
        FormatStyle::SIS_WithoutElse;
    ChromiumStyle.BreakAfterJavaFieldAnnotations = true;
    ChromiumStyle.ContinuationIndentWidth = 8;
    ChromiumStyle.IndentWidth = 4;
    // See styleguide for import groups:
    // https://chromium.googlesource.com/chromium/src/+/refs/heads/main/styleguide/java/java.md#Import-Order
    ChromiumStyle.JavaImportGroups = {
        "android",
        "androidx",
        "com",
        "dalvik",
        "junit",
        "org",
        "com.google.android.apps.chrome",
        "org.chromium",
        "java",
        "javax",
    };
    ChromiumStyle.SortIncludes = FormatStyle::SI_CaseSensitive;
  } else if (Language == FormatStyle::LK_JavaScript) {
    ChromiumStyle.AllowShortIfStatementsOnASingleLine = FormatStyle::SIS_Never;
    ChromiumStyle.AllowShortLoopsOnASingleLine = false;
  } else {
    ChromiumStyle.AllowAllParametersOfDeclarationOnNextLine = false;
    ChromiumStyle.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Inline;
    ChromiumStyle.AllowShortIfStatementsOnASingleLine = FormatStyle::SIS_Never;
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
  MozillaStyle.AlwaysBreakAfterDefinitionReturnType =
      FormatStyle::DRTBS_TopLevel;
  MozillaStyle.BinPackArguments = false;
  MozillaStyle.BinPackParameters = false;
  MozillaStyle.BreakAfterReturnType = FormatStyle::RTBS_TopLevel;
  MozillaStyle.BreakBeforeBraces = FormatStyle::BS_Mozilla;
  MozillaStyle.BreakConstructorInitializers = FormatStyle::BCIS_BeforeComma;
  MozillaStyle.BreakInheritanceList = FormatStyle::BILS_BeforeComma;
  MozillaStyle.BreakTemplateDeclarations = FormatStyle::BTDS_Yes;
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
  Style.AlignOperands = FormatStyle::OAS_DontAlign;
  Style.AlignTrailingComments = {};
  Style.AlignTrailingComments.Kind = FormatStyle::TCAS_Never;
  Style.AllowShortBlocksOnASingleLine = FormatStyle::SBS_Empty;
  Style.BreakBeforeBinaryOperators = FormatStyle::BOS_All;
  Style.BreakBeforeBraces = FormatStyle::BS_WebKit;
  Style.BreakConstructorInitializers = FormatStyle::BCIS_BeforeComma;
  Style.ColumnLimit = 0;
  Style.Cpp11BracedListStyle = false;
  Style.FixNamespaceComments = false;
  Style.IndentWidth = 4;
  Style.NamespaceIndentation = FormatStyle::NI_Inner;
  Style.ObjCBlockIndentWidth = 4;
  Style.ObjCSpaceAfterProperty = true;
  Style.PointerAlignment = FormatStyle::PAS_Left;
  Style.SpaceBeforeCpp11BracedList = true;
  Style.SpaceInEmptyBlock = true;
  return Style;
}

FormatStyle getGNUStyle() {
  FormatStyle Style = getLLVMStyle();
  Style.AlwaysBreakAfterDefinitionReturnType = FormatStyle::DRTBS_All;
  Style.BreakAfterReturnType = FormatStyle::RTBS_AllDefinitions;
  Style.BreakBeforeBinaryOperators = FormatStyle::BOS_All;
  Style.BreakBeforeBraces = FormatStyle::BS_GNU;
  Style.BreakBeforeTernaryOperators = true;
  Style.ColumnLimit = 79;
  Style.Cpp11BracedListStyle = false;
  Style.FixNamespaceComments = false;
  Style.SpaceBeforeParens = FormatStyle::SBPO_Always;
  Style.Standard = FormatStyle::LS_Cpp03;
  return Style;
}

FormatStyle getMicrosoftStyle(FormatStyle::LanguageKind Language) {
  FormatStyle Style = getLLVMStyle(Language);
  Style.ColumnLimit = 120;
  Style.TabWidth = 4;
  Style.IndentWidth = 4;
  Style.UseTab = FormatStyle::UT_Never;
  Style.BreakBeforeBraces = FormatStyle::BS_Custom;
  Style.BraceWrapping.AfterClass = true;
  Style.BraceWrapping.AfterControlStatement = FormatStyle::BWACS_Always;
  Style.BraceWrapping.AfterEnum = true;
  Style.BraceWrapping.AfterFunction = true;
  Style.BraceWrapping.AfterNamespace = true;
  Style.BraceWrapping.AfterObjCDeclaration = true;
  Style.BraceWrapping.AfterStruct = true;
  Style.BraceWrapping.AfterExternBlock = true;
  Style.BraceWrapping.BeforeCatch = true;
  Style.BraceWrapping.BeforeElse = true;
  Style.BraceWrapping.BeforeWhile = false;
  Style.PenaltyReturnTypeOnItsOwnLine = 1000;
  Style.AllowShortEnumsOnASingleLine = false;
  Style.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_None;
  Style.AllowShortCaseLabelsOnASingleLine = false;
  Style.AllowShortIfStatementsOnASingleLine = FormatStyle::SIS_Never;
  Style.AllowShortLoopsOnASingleLine = false;
  Style.AlwaysBreakAfterDefinitionReturnType = FormatStyle::DRTBS_None;
  Style.BreakAfterReturnType = FormatStyle::RTBS_None;
  return Style;
}

FormatStyle getClangFormatStyle() {
  FormatStyle Style = getLLVMStyle();
  Style.InsertBraces = true;
  Style.InsertNewlineAtEOF = true;
  Style.IntegerLiteralSeparator.Decimal = 3;
  Style.IntegerLiteralSeparator.DecimalMinDigits = 5;
  Style.LineEnding = FormatStyle::LE_LF;
  Style.RemoveBracesLLVM = true;
  Style.RemoveParentheses = FormatStyle::RPS_ReturnStatement;
  Style.RemoveSemicolon = true;
  return Style;
}

FormatStyle getNoStyle() {
  FormatStyle NoStyle = getLLVMStyle();
  NoStyle.DisableFormat = true;
  NoStyle.SortIncludes = FormatStyle::SI_Never;
  NoStyle.SortUsingDeclarations = FormatStyle::SUD_Never;
  return NoStyle;
}

bool getPredefinedStyle(StringRef Name, FormatStyle::LanguageKind Language,
                        FormatStyle *Style) {
  if (Name.equals_insensitive("llvm"))
    *Style = getLLVMStyle(Language);
  else if (Name.equals_insensitive("chromium"))
    *Style = getChromiumStyle(Language);
  else if (Name.equals_insensitive("mozilla"))
    *Style = getMozillaStyle();
  else if (Name.equals_insensitive("google"))
    *Style = getGoogleStyle(Language);
  else if (Name.equals_insensitive("webkit"))
    *Style = getWebKitStyle();
  else if (Name.equals_insensitive("gnu"))
    *Style = getGNUStyle();
  else if (Name.equals_insensitive("microsoft"))
    *Style = getMicrosoftStyle(Language);
  else if (Name.equals_insensitive("clang-format"))
    *Style = getClangFormatStyle();
  else if (Name.equals_insensitive("none"))
    *Style = getNoStyle();
  else if (Name.equals_insensitive("inheritparentconfig"))
    Style->InheritsParentConfig = true;
  else
    return false;

  Style->Language = Language;
  return true;
}

ParseError validateQualifierOrder(FormatStyle *Style) {
  // If its empty then it means don't do anything.
  if (Style->QualifierOrder.empty())
    return ParseError::MissingQualifierOrder;

  // Ensure the list contains only currently valid qualifiers.
  for (const auto &Qualifier : Style->QualifierOrder) {
    if (Qualifier == "type")
      continue;
    auto token = getTokenFromQualifier(Qualifier);
    if (token == tok::identifier)
      return ParseError::InvalidQualifierSpecified;
  }

  // Ensure the list is unique (no duplicates).
  std::set<std::string> UniqueQualifiers(Style->QualifierOrder.begin(),
                                         Style->QualifierOrder.end());
  if (Style->QualifierOrder.size() != UniqueQualifiers.size()) {
    LLVM_DEBUG(llvm::dbgs()
               << "Duplicate Qualifiers " << Style->QualifierOrder.size()
               << " vs " << UniqueQualifiers.size() << "\n");
    return ParseError::DuplicateQualifierSpecified;
  }

  // Ensure the list has 'type' in it.
  if (!llvm::is_contained(Style->QualifierOrder, "type"))
    return ParseError::MissingQualifierType;

  return ParseError::Success;
}

std::error_code parseConfiguration(llvm::MemoryBufferRef Config,
                                   FormatStyle *Style) {
  assert(Style);
  FormatStyle::LanguageKind Language = Style->Language;
  assert(Language != FormatStyle::LK_None);
  if (Config.getBuffer().trim().empty())
    return make_error_code(ParseError::Success);
  Style->StyleSet.Clear();
  std::vector<FormatStyle> Styles;
  llvm::yaml::Input Input(Config, /*Ctxt=*/nullptr, nullptr, nullptr);
  // DocumentListTraits<vector<FormatStyle>> uses the context to get default
  // values for the fields, keys for which are missing from the configuration.
  // Mapping also uses the context to get the language to find the correct
  // base style.
  Input.setContext(Style);
  Input.setAllowUnknownKeys(false);
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
  for (const FormatStyle &Style : llvm::reverse(Styles)) {
    if (Style.Language != FormatStyle::LK_None)
      StyleSet.Add(Style);
    if (Style.Language == Language)
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
  if (Style->InsertTrailingCommas != FormatStyle::TCS_None &&
      Style->BinPackArguments) {
    // See comment on FormatStyle::TSC_Wrapped.
    return make_error_code(ParseError::BinPackTrailingCommaConflict);
  }
  if (Style->QualifierAlignment != FormatStyle::QAS_Leave)
    return make_error_code(validateQualifierOrder(Style));
  return make_error_code(ParseError::Success);
}

std::string configurationAsText(const FormatStyle &Style) {
  std::string Text;
  llvm::raw_string_ostream Stream(Text);
  llvm::yaml::Output Output(Stream);
  // We use the same mapping method for input and output, so we need a non-const
  // reference here.
  FormatStyle NonConstStyle = Style;
  expandPresetsBraceWrapping(NonConstStyle);
  expandPresetsSpaceBeforeParens(NonConstStyle);
  expandPresetsSpacesInParens(NonConstStyle);
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

} // namespace clang_v19
