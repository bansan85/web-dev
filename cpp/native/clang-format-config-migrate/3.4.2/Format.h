//===--- Format.h - Format C++ code -----------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Various functions to configurably format source code.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "llvm/Support/YAMLTraits.h"

namespace clang_v3_4 {

/// \brief The \c FormatStyle is used to configure the formatting to follow
/// specific guidelines.
struct FormatStyle {
  /// \brief The column limit.
  ///
  /// A column limit of \c 0 means that there is no column limit. In this case,
  /// clang-format will respect the input's line breaking decisions within
  /// statements.
  unsigned ColumnLimit;

  /// \brief The maximum number of consecutive empty lines to keep.
  unsigned MaxEmptyLinesToKeep;

  /// \brief The penalty for each line break introduced inside a comment.
  unsigned PenaltyBreakComment;

  /// \brief The penalty for each line break introduced inside a string literal.
  unsigned PenaltyBreakString;

  /// \brief The penalty for each character outside of the column limit.
  unsigned PenaltyExcessCharacter;

  /// \brief The penalty for breaking before the first \c <<.
  unsigned PenaltyBreakFirstLessLess;

  /// \brief The penalty for breaking a function call after "call(".
  unsigned PenaltyBreakBeforeFirstCallParameter;

  /// \brief Set whether & and * bind to the type as opposed to the variable.
  bool PointerBindsToType;

  /// \brief If \c true, analyze the formatted file for the most common binding.
  bool DerivePointerBinding;

  /// \brief The extra indent or outdent of access modifiers, e.g. \c public:.
  int AccessModifierOffset;

  /// \brief Supported language standards.
  enum LanguageStandard {
    /// Use C++03-compatible syntax.
    LS_Cpp03,
    /// Use features of C++11 (e.g. \c A<A<int>> instead of
    /// <tt>A<A<int> ></tt>).
    LS_Cpp11,
    /// Automatic detection based on the input.
    LS_Auto
  };

  /// \brief Format compatible with this standard, e.g. use
  /// <tt>A<A<int> ></tt> instead of \c A<A<int>> for LS_Cpp03.
  LanguageStandard Standard;

  /// \brief Indent case labels one level from the switch statement.
  ///
  /// When \c false, use the same indentation level as for the switch statement.
  /// Switch statement body is always indented one level more than case labels.
  bool IndentCaseLabels;

  /// \brief Different ways to indent namespace contents.
  enum NamespaceIndentationKind {
    /// Don't indent in namespaces.
    NI_None,
    /// Indent only in inner namespaces (nested in other namespaces).
    NI_Inner,
    /// Indent in all namespaces.
    NI_All
  };

  /// \brief The indentation used for namespaces.
  NamespaceIndentationKind NamespaceIndentation;

  /// \brief The number of spaces to before trailing line comments.
  unsigned SpacesBeforeTrailingComments;

  /// \brief If \c false, a function call's or function definition's parameters
  /// will either all be on the same line or will have one line each.
  bool BinPackParameters;

  /// \brief If \c true, clang-format detects whether function calls and
  /// definitions are formatted with one parameter per line.
  ///
  /// Each call can be bin-packed, one-per-line or inconclusive. If it is
  /// inconclusive, e.g. completely on one line, but a decision needs to be
  /// made, clang-format analyzes whether there are other bin-packed cases in
  /// the input file and act accordingly.
  ///
  /// NOTE: This is an experimental flag, that might go away or be renamed. Do
  /// not use this in config files, etc. Use at your own risk.
  bool ExperimentalAutoDetectBinPacking;

  /// \brief Allow putting all parameters of a function declaration onto
  /// the next line even if \c BinPackParameters is \c false.
  bool AllowAllParametersOfDeclarationOnNextLine;

  /// \brief Penalty for putting the return type of a function onto its own
  /// line.
  unsigned PenaltyReturnTypeOnItsOwnLine;

  /// \brief If the constructor initializers don't fit on a line, put each
  /// initializer on its own line.
  bool ConstructorInitializerAllOnOneLineOrOnePerLine;

  /// \brief Always break constructor initializers before commas and align
  /// the commas with the colon.
  bool BreakConstructorInitializersBeforeComma;

  /// \brief If \c true, <tt>if (a) return;</tt> can be put on a single
  /// line.
  bool AllowShortIfStatementsOnASingleLine;

  /// \brief If \c true, <tt>while (true) continue;</tt> can be put on a
  /// single line.
  bool AllowShortLoopsOnASingleLine;

  /// \brief Add a space in front of an Objective-C protocol list, i.e. use
  /// <tt>Foo <Protocol></tt> instead of \c Foo<Protocol>.
  bool ObjCSpaceBeforeProtocolList;

  /// \brief If \c true, aligns trailing comments.
  bool AlignTrailingComments;

  /// \brief If \c true, aligns escaped newlines as far left as possible.
  /// Otherwise puts them into the right-most column.
  bool AlignEscapedNewlinesLeft;

  /// \brief The number of columns to use for indentation.
  unsigned IndentWidth;

  /// \brief The number of columns used for tab stops.
  unsigned TabWidth;

  /// \brief The number of characters to use for indentation of constructor
  /// initializer lists.
  unsigned ConstructorInitializerIndentWidth;

  /// \brief If \c true, always break after the <tt>template<...></tt> of a
  /// template declaration.
  bool AlwaysBreakTemplateDeclarations;

  /// \brief If \c true, always break before multiline string literals.
  bool AlwaysBreakBeforeMultilineStrings;

  /// \brief Different ways to use tab in formatting.
  enum UseTabStyle {
    /// Never use tab.
    UT_Never,
    /// Use tabs only for indentation.
    UT_ForIndentation,
    /// Use tabs whenever we need to fill whitespace that spans at least from
    /// one tab stop to the next one.
    UT_Always
  };

  /// \brief The way to use tab characters in the resulting file.
  UseTabStyle UseTab;

  /// \brief If \c true, binary operators will be placed after line breaks.
  bool BreakBeforeBinaryOperators;

  /// \brief If \c true, ternary operators will be placed after line breaks.
  bool BreakBeforeTernaryOperators;

  /// \brief Different ways to attach braces to their surrounding context.
  enum BraceBreakingStyle {
    /// Always attach braces to surrounding context.
    BS_Attach,
    /// Like \c Attach, but break before braces on function, namespace and
    /// class definitions.
    BS_Linux,
    /// Like \c Attach, but break before function definitions.
    BS_Stroustrup,
    /// Always break before braces.
    BS_Allman
  };

  /// \brief The brace breaking style to use.
  BraceBreakingStyle BreakBeforeBraces;

  /// \brief If \c true, format braced lists as best suited for C++11 braced
  /// lists.
  ///
  /// Important differences:
  /// - No spaces inside the braced list.
  /// - No line break before the closing brace.
  /// - Indentation with the continuation indent, not with the block indent.
  ///
  /// Fundamentally, C++11 braced lists are formatted exactly like function
  /// calls would be formatted in their place. If the braced list follows a name
  /// (e.g. a type or variable name), clang-format formats as if the \c {} were
  /// the parentheses of a function call with that name. If there is no name,
  /// a zero-length name is assumed.
  bool Cpp11BracedListStyle;

  /// \brief If \c true, indent when breaking function declarations which
  /// are not also definitions after the type.
  bool IndentFunctionDeclarationAfterType;

  /// \brief If \c true, spaces will be inserted after '(' and before ')'.
  bool SpacesInParentheses;

  /// \brief If \c true, spaces will be inserted after '<' and before '>' in
  /// template argument lists
  bool SpacesInAngles;

  /// \brief If \c false, spaces may be inserted into '()'.
  bool SpaceInEmptyParentheses;

  /// \brief If \c false, spaces may be inserted into C style casts.
  bool SpacesInCStyleCastParentheses;

  /// \brief If \c true, spaces will be inserted between 'for'/'if'/'while'/...
  /// and '('.
  bool SpaceAfterControlStatementKeyword;

  /// \brief If \c false, spaces will be removed before assignment operators.
  bool SpaceBeforeAssignmentOperators;

  /// \brief Indent width for line continuations.
  unsigned ContinuationIndentWidth;

  bool operator==(const FormatStyle &R) const {
    return AccessModifierOffset == R.AccessModifierOffset &&
           ConstructorInitializerIndentWidth ==
               R.ConstructorInitializerIndentWidth &&
           AlignEscapedNewlinesLeft == R.AlignEscapedNewlinesLeft &&
           AlignTrailingComments == R.AlignTrailingComments &&
           AllowAllParametersOfDeclarationOnNextLine ==
               R.AllowAllParametersOfDeclarationOnNextLine &&
           AllowShortIfStatementsOnASingleLine ==
               R.AllowShortIfStatementsOnASingleLine &&
           AllowShortLoopsOnASingleLine == R.AllowShortLoopsOnASingleLine &&
           AlwaysBreakTemplateDeclarations ==
               R.AlwaysBreakTemplateDeclarations &&
           AlwaysBreakBeforeMultilineStrings ==
               R.AlwaysBreakBeforeMultilineStrings &&
           BinPackParameters == R.BinPackParameters &&
           BreakBeforeBinaryOperators == R.BreakBeforeBinaryOperators &&
           BreakBeforeTernaryOperators == R.BreakBeforeTernaryOperators &&
           BreakBeforeBraces == R.BreakBeforeBraces &&
           BreakConstructorInitializersBeforeComma ==
               R.BreakConstructorInitializersBeforeComma &&
           ColumnLimit == R.ColumnLimit &&
           ConstructorInitializerAllOnOneLineOrOnePerLine ==
               R.ConstructorInitializerAllOnOneLineOrOnePerLine &&
           DerivePointerBinding == R.DerivePointerBinding &&
           ExperimentalAutoDetectBinPacking ==
               R.ExperimentalAutoDetectBinPacking &&
           IndentCaseLabels == R.IndentCaseLabels &&
           IndentFunctionDeclarationAfterType ==
               R.IndentFunctionDeclarationAfterType &&
           IndentWidth == R.IndentWidth &&
           MaxEmptyLinesToKeep == R.MaxEmptyLinesToKeep &&
           NamespaceIndentation == R.NamespaceIndentation &&
           ObjCSpaceBeforeProtocolList == R.ObjCSpaceBeforeProtocolList &&
           PenaltyBreakComment == R.PenaltyBreakComment &&
           PenaltyBreakFirstLessLess == R.PenaltyBreakFirstLessLess &&
           PenaltyBreakString == R.PenaltyBreakString &&
           PenaltyExcessCharacter == R.PenaltyExcessCharacter &&
           PenaltyReturnTypeOnItsOwnLine == R.PenaltyReturnTypeOnItsOwnLine &&
           PointerBindsToType == R.PointerBindsToType &&
           SpacesBeforeTrailingComments == R.SpacesBeforeTrailingComments &&
           Cpp11BracedListStyle == R.Cpp11BracedListStyle &&
           Standard == R.Standard && TabWidth == R.TabWidth &&
           UseTab == R.UseTab && SpacesInParentheses == R.SpacesInParentheses &&
           SpacesInAngles == R.SpacesInAngles &&
           SpaceInEmptyParentheses == R.SpaceInEmptyParentheses &&
           SpacesInCStyleCastParentheses == R.SpacesInCStyleCastParentheses &&
           SpaceAfterControlStatementKeyword ==
               R.SpaceAfterControlStatementKeyword &&
           SpaceBeforeAssignmentOperators == R.SpaceBeforeAssignmentOperators &&
           ContinuationIndentWidth == R.ContinuationIndentWidth;
  }
};

/// \brief Returns a format style complying with the LLVM coding standards:
/// http://llvm.org/docs/CodingStandards.html.
FormatStyle getLLVMStyle();

/// \brief Returns a format style complying with Google's C++ style guide:
/// http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml.
FormatStyle getGoogleStyle();

/// \brief Returns a format style complying with Chromium's style guide:
/// http://www.chromium.org/developers/coding-style.
FormatStyle getChromiumStyle();

/// \brief Returns a format style complying with Mozilla's style guide:
/// https://developer.mozilla.org/en-US/docs/Developer_Guide/Coding_Style.
FormatStyle getMozillaStyle();

/// \brief Returns a format style complying with Webkit's style guide:
/// http://www.webkit.org/coding/coding-style.html
FormatStyle getWebKitStyle();

/// \brief Gets a predefined style by name.
///
/// Currently supported names: LLVM, Google, Chromium, Mozilla. Names are
/// compared case-insensitively.
///
/// Returns \c true if the Style has been set.
bool getPredefinedStyle(llvm::StringRef Name, FormatStyle *Style);

std::vector<std::string> getStyleNames();

/// \brief Parse configuration from YAML-formatted text.
std::error_code parseConfiguration(const std::string &Text, FormatStyle *Style);

/// \brief Gets configuration in a YAML string.
std::string configurationAsText(const FormatStyle &Style,
                                const std::string &DefaultStyleName,
                                bool SkipSameValue);

} // end namespace clang_v3_4
