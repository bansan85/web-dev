//===--- Format.h - Format C++ code -----------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Various functions to configurably format source code.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "IncludeStyle.h"
#include <optional>
#include <system_error>
#include <vector>

namespace clang_v11 {

enum class ParseError {
  Success = 0,
  Error,
  Unsuitable,
  BinPackTrailingCommaConflict
};
class ParseErrorCategory final : public std::error_category {
public:
  const char *name() const noexcept override;
  std::string message(int EV) const override;
};
const std::error_category &getParseCategory();
std::error_code make_error_code(ParseError e);

/// The ``FormatStyle`` is used to configure the formatting to follow
/// specific guidelines.
struct FormatStyle {
  /// The extra indent or outdent of access modifiers, e.g. ``public:``.
  int AccessModifierOffset;

  /// Different styles for aligning after open brackets.
  enum BracketAlignmentStyle {
    /// Align parameters on the open bracket, e.g.:
    /// \code
    ///   someLongFunction(argument1,
    ///                    argument2);
    /// \endcode
    BAS_Align,
    /// Don't align, instead use ``ContinuationIndentWidth``, e.g.:
    /// \code
    ///   someLongFunction(argument1,
    ///       argument2);
    /// \endcode
    BAS_DontAlign,
    /// Always break after an open bracket, if the parameters don't fit
    /// on a single line, e.g.:
    /// \code
    ///   someLongFunction(
    ///       argument1, argument2);
    /// \endcode
    BAS_AlwaysBreak,
  };

  /// If ``true``, horizontally aligns arguments after an open bracket.
  ///
  /// This applies to round brackets (parentheses), angle brackets and square
  /// brackets.
  BracketAlignmentStyle AlignAfterOpenBracket;

  /// \brief If ``true``, aligns consecutive C/C++ preprocessor macros.
  ///
  /// This will align C/C++ preprocessor macros of consecutive lines.
  /// Will result in formattings like
  /// \code
  ///   #define SHORT_NAME       42
  ///   #define LONGER_NAME      0x007f
  ///   #define EVEN_LONGER_NAME (2)
  ///   #define foo(x)           (x * x)
  ///   #define bar(y, z)        (y + z)
  /// \endcode
  bool AlignConsecutiveMacros;

  /// If ``true``, aligns consecutive assignments.
  ///
  /// This will align the assignment operators of consecutive lines. This
  /// will result in formattings like
  /// \code
  ///   int aaaa = 12;
  ///   int b    = 23;
  ///   int ccc  = 23;
  /// \endcode
  bool AlignConsecutiveAssignments;

  /// If ``true``, aligns consecutive bitfield members.
  ///
  /// This will align the bitfield separators of consecutive lines. This
  /// will result in formattings like
  /// \code
  ///   int aaaa : 1;
  ///   int b    : 12;
  ///   int ccc  : 8;
  /// \endcode
  bool AlignConsecutiveBitFields;

  /// If ``true``, aligns consecutive declarations.
  ///
  /// This will align the declaration names of consecutive lines. This
  /// will result in formattings like
  /// \code
  ///   int         aaaa = 12;
  ///   float       b = 23;
  ///   std::string ccc = 23;
  /// \endcode
  bool AlignConsecutiveDeclarations;

  /// Different styles for aligning escaped newlines.
  enum EscapedNewlineAlignmentStyle {
    /// Don't align escaped newlines.
    /// \code
    ///   #define A \
    ///     int aaaa; \
    ///     int b; \
    ///     int dddddddddd;
    /// \endcode
    ENAS_DontAlign,
    /// Align escaped newlines as far left as possible.
    /// \code
    ///   true:
    ///   #define A   \
    ///     int aaaa; \
    ///     int b;    \
    ///     int dddddddddd;
    ///
    ///   false:
    /// \endcode
    ENAS_Left,
    /// Align escaped newlines in the right-most column.
    /// \code
    ///   #define A                                                                      \
    ///     int aaaa;                                                                    \
    ///     int b;                                                                       \
    ///     int dddddddddd;
    /// \endcode
    ENAS_Right,
  };

  /// Options for aligning backslashes in escaped newlines.
  EscapedNewlineAlignmentStyle AlignEscapedNewlines;

  /// Different styles for aligning operands.
  enum OperandAlignmentStyle {
    /// Do not align operands of binary and ternary expressions.
    /// The wrapped lines are indented ``ContinuationIndentWidth`` spaces from
    /// the start of the line.
    OAS_DontAlign,
    /// Horizontally align operands of binary and ternary expressions.
    ///
    /// Specifically, this aligns operands of a single expression that needs
    /// to be split over multiple lines, e.g.:
    /// \code
    ///   int aaa = bbbbbbbbbbbbbbb +
    ///             ccccccccccccccc;
    /// \endcode
    ///
    /// When ``BreakBeforeBinaryOperators`` is set, the wrapped operator is
    /// aligned with the operand on the first line.
    /// \code
    ///   int aaa = bbbbbbbbbbbbbbb
    ///             + ccccccccccccccc;
    /// \endcode
    OAS_Align,
    /// Horizontally align operands of binary and ternary expressions.
    ///
    /// This is similar to ``AO_Align``, except when
    /// ``BreakBeforeBinaryOperators`` is set, the operator is un-indented so
    /// that the wrapped operand is aligned with the operand on the first line.
    /// \code
    ///   int aaa = bbbbbbbbbbbbbbb
    ///           + ccccccccccccccc;
    /// \endcode
    OAS_AlignAfterOperator,
  };

  /// If ``true``, horizontally align operands of binary and ternary
  /// expressions.
  OperandAlignmentStyle AlignOperands;

  /// If ``true``, aligns trailing comments.
  /// \code
  ///   true:                                   false:
  ///   int a;     // My comment a      vs.     int a; // My comment a
  ///   int b = 2; // comment  b                int b = 2; // comment about b
  /// \endcode
  bool AlignTrailingComments;

  /// \brief If a function call or braced initializer list doesn't fit on a
  /// line, allow putting all arguments onto the next line, even if
  /// ``BinPackArguments`` is ``false``.
  /// \code
  ///   true:
  ///   callFunction(
  ///       a, b, c, d);
  ///
  ///   false:
  ///   callFunction(a,
  ///                b,
  ///                c,
  ///                d);
  /// \endcode
  bool AllowAllArgumentsOnNextLine;

  /// \brief If a constructor definition with a member initializer list doesn't
  /// fit on a single line, allow putting all member initializers onto the next
  /// line, if ```ConstructorInitializerAllOnOneLineOrOnePerLine``` is true.
  /// Note that this parameter has no effect if
  /// ```ConstructorInitializerAllOnOneLineOrOnePerLine``` is false.
  /// \code
  ///   true:
  ///   MyClass::MyClass() :
  ///       member0(0), member1(2) {}
  ///
  ///   false:
  ///   MyClass::MyClass() :
  ///       member0(0),
  ///       member1(2) {}
  bool AllowAllConstructorInitializersOnNextLine;

  /// If the function declaration doesn't fit on a line,
  /// allow putting all parameters of a function declaration onto
  /// the next line even if ``BinPackParameters`` is ``false``.
  /// \code
  ///   true:
  ///   void myFunction(
  ///       int a, int b, int c, int d, int e);
  ///
  ///   false:
  ///   void myFunction(int a,
  ///                   int b,
  ///                   int c,
  ///                   int d,
  ///                   int e);
  /// \endcode
  bool AllowAllParametersOfDeclarationOnNextLine;

  /// Allow short enums on a single line.
  /// \code
  ///   true:
  ///   enum { A, B } myEnum;
  ///
  ///   false:
  ///   enum
  ///   {
  ///     A,
  ///     B
  ///   } myEnum;
  /// \endcode
  bool AllowShortEnumsOnASingleLine;

  /// Different styles for merging short blocks containing at most one
  /// statement.
  enum ShortBlockStyle {
    /// Never merge blocks into a single line.
    /// \code
    ///   while (true) {
    ///   }
    ///   while (true) {
    ///     continue;
    ///   }
    /// \endcode
    SBS_Never,
    /// Only merge empty blocks.
    /// \code
    ///   while (true) {}
    ///   while (true) {
    ///     continue;
    ///   }
    /// \endcode
    SBS_Empty,
    /// Always merge short blocks into a single line.
    /// \code
    ///   while (true) {}
    ///   while (true) { continue; }
    /// \endcode
    SBS_Always,
  };

  /// Dependent on the value, ``while (true) { continue; }`` can be put on a
  /// single line.
  ShortBlockStyle AllowShortBlocksOnASingleLine;

  /// If ``true``, short case labels will be contracted to a single line.
  /// \code
  ///   true:                                   false:
  ///   switch (a) {                    vs.     switch (a) {
  ///   case 1: x = 1; break;                   case 1:
  ///   case 2: return;                           x = 1;
  ///   }                                         break;
  ///                                           case 2:
  ///                                             return;
  ///                                           }
  /// \endcode
  bool AllowShortCaseLabelsOnASingleLine;

  /// Different styles for merging short functions containing at most one
  /// statement.
  enum ShortFunctionStyle {
    /// Never merge functions into a single line.
    SFS_None,
    /// Only merge functions defined inside a class. Same as "inline",
    /// except it does not implies "empty": i.e. top level empty functions
    /// are not merged either.
    /// \code
    ///   class Foo {
    ///     void f() { foo(); }
    ///   };
    ///   void f() {
    ///     foo();
    ///   }
    ///   void f() {
    ///   }
    /// \endcode
    SFS_InlineOnly,
    /// Only merge empty functions.
    /// \code
    ///   void f() {}
    ///   void f2() {
    ///     bar2();
    ///   }
    /// \endcode
    SFS_Empty,
    /// Only merge functions defined inside a class. Implies "empty".
    /// \code
    ///   class Foo {
    ///     void f() { foo(); }
    ///   };
    ///   void f() {
    ///     foo();
    ///   }
    ///   void f() {}
    /// \endcode
    SFS_Inline,
    /// Merge all functions fitting on a single line.
    /// \code
    ///   class Foo {
    ///     void f() { foo(); }
    ///   };
    ///   void f() { bar(); }
    /// \endcode
    SFS_All,
  };

  /// Dependent on the value, ``int f() { return 0; }`` can be put on a
  /// single line.
  ShortFunctionStyle AllowShortFunctionsOnASingleLine;

  /// Different styles for handling short if lines
  enum ShortIfStyle {
    /// Never put short ifs on the same line.
    /// \code
    ///   if (a)
    ///     return ;
    ///   else {
    ///     return;
    ///   }
    /// \endcode
    SIS_Never,
    /// Without else put short ifs on the same line only if
    /// the else is not a compound statement.
    /// \code
    ///   if (a) return;
    ///   else
    ///     return;
    /// \endcode
    SIS_WithoutElse,
    /// Always put short ifs on the same line if
    /// the else is not a compound statement or not.
    /// \code
    ///   if (a) return;
    ///   else {
    ///     return;
    ///   }
    /// \endcode
    SIS_Always,
  };

  /// If ``true``, ``if (a) return;`` can be put on a single line.
  ShortIfStyle AllowShortIfStatementsOnASingleLine;

  /// Different styles for merging short lambdas containing at most one
  /// statement.
  enum ShortLambdaStyle {
    /// Never merge lambdas into a single line.
    SLS_None,
    /// Only merge empty lambdas.
    /// \code
    ///   auto lambda = [](int a) {}
    ///   auto lambda2 = [](int a) {
    ///       return a;
    ///   };
    /// \endcode
    SLS_Empty,
    /// Merge lambda into a single line if argument of a function.
    /// \code
    ///   auto lambda = [](int a) {
    ///       return a;
    ///   };
    ///   sort(a.begin(), a.end(), ()[] { return x < y; })
    /// \endcode
    SLS_Inline,
    /// Merge all lambdas fitting on a single line.
    /// \code
    ///   auto lambda = [](int a) {}
    ///   auto lambda2 = [](int a) { return a; };
    /// \endcode
    SLS_All,
  };

  /// Dependent on the value, ``auto lambda []() { return 0; }`` can be put on a
  /// single line.
  ShortLambdaStyle AllowShortLambdasOnASingleLine;

  /// If ``true``, ``while (true) continue;`` can be put on a single
  /// line.
  bool AllowShortLoopsOnASingleLine;

  /// Different ways to break after the function definition return type.
  /// This option is **deprecated** and is retained for backwards compatibility.
  enum DefinitionReturnTypeBreakingStyle {
    /// Break after return type automatically.
    /// ``PenaltyReturnTypeOnItsOwnLine`` is taken into account.
    DRTBS_None,
    /// Always break after the return type.
    DRTBS_All,
    /// Always break after the return types of top-level functions.
    DRTBS_TopLevel,
  };

  /// Different ways to break after the function definition or
  /// declaration return type.
  enum ReturnTypeBreakingStyle {
    /// Break after return type automatically.
    /// ``PenaltyReturnTypeOnItsOwnLine`` is taken into account.
    /// \code
    ///   class A {
    ///     int f() { return 0; };
    ///   };
    ///   int f();
    ///   int f() { return 1; }
    /// \endcode
    RTBS_None,
    /// Always break after the return type.
    /// \code
    ///   class A {
    ///     int
    ///     f() {
    ///       return 0;
    ///     };
    ///   };
    ///   int
    ///   f();
    ///   int
    ///   f() {
    ///     return 1;
    ///   }
    /// \endcode
    RTBS_All,
    /// Always break after the return types of top-level functions.
    /// \code
    ///   class A {
    ///     int f() { return 0; };
    ///   };
    ///   int
    ///   f();
    ///   int
    ///   f() {
    ///     return 1;
    ///   }
    /// \endcode
    RTBS_TopLevel,
    /// Always break after the return type of function definitions.
    /// \code
    ///   class A {
    ///     int
    ///     f() {
    ///       return 0;
    ///     };
    ///   };
    ///   int f();
    ///   int
    ///   f() {
    ///     return 1;
    ///   }
    /// \endcode
    RTBS_AllDefinitions,
    /// Always break after the return type of top-level definitions.
    /// \code
    ///   class A {
    ///     int f() { return 0; };
    ///   };
    ///   int f();
    ///   int
    ///   f() {
    ///     return 1;
    ///   }
    /// \endcode
    RTBS_TopLevelDefinitions,
  };

  /// The function definition return type breaking style to use.  This
  /// option is **deprecated** and is retained for backwards compatibility.
  DefinitionReturnTypeBreakingStyle AlwaysBreakAfterDefinitionReturnType;

  /// The function declaration return type breaking style to use.
  ReturnTypeBreakingStyle AlwaysBreakAfterReturnType;

  /// If ``true``, always break before multiline string literals.
  ///
  /// This flag is mean to make cases where there are multiple multiline strings
  /// in a file look more consistent. Thus, it will only take effect if wrapping
  /// the string at that point leads to it being indented
  /// ``ContinuationIndentWidth`` spaces from the start of the line.
  /// \code
  ///    true:                                  false:
  ///    aaaa =                         vs.     aaaa = "bbbb"
  ///        "bbbb"                                    "cccc";
  ///        "cccc";
  /// \endcode
  bool AlwaysBreakBeforeMultilineStrings;

  /// Different ways to break after the template declaration.
  enum BreakTemplateDeclarationsStyle {
    /// Do not force break before declaration.
    /// ``PenaltyBreakTemplateDeclaration`` is taken into account.
    /// \code
    ///    template <typename T> T foo() {
    ///    }
    ///    template <typename T> T foo(int aaaaaaaaaaaaaaaaaaaaa,
    ///                                int bbbbbbbbbbbbbbbbbbbbb) {
    ///    }
    /// \endcode
    BTDS_No,
    /// Force break after template declaration only when the following
    /// declaration spans multiple lines.
    /// \code
    ///    template <typename T> T foo() {
    ///    }
    ///    template <typename T>
    ///    T foo(int aaaaaaaaaaaaaaaaaaaaa,
    ///          int bbbbbbbbbbbbbbbbbbbbb) {
    ///    }
    /// \endcode
    BTDS_MultiLine,
    /// Always break after template declaration.
    /// \code
    ///    template <typename T>
    ///    T foo() {
    ///    }
    ///    template <typename T>
    ///    T foo(int aaaaaaaaaaaaaaaaaaaaa,
    ///          int bbbbbbbbbbbbbbbbbbbbb) {
    ///    }
    /// \endcode
    BTDS_Yes
  };

  /// The template declaration breaking style to use.
  BreakTemplateDeclarationsStyle AlwaysBreakTemplateDeclarations;

  /// If ``false``, a function call's arguments will either be all on the
  /// same line or will have one line each.
  /// \code
  ///   true:
  ///   void f() {
  ///     f(aaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaa,
  ///       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa);
  ///   }
  ///
  ///   false:
  ///   void f() {
  ///     f(aaaaaaaaaaaaaaaaaaaa,
  ///       aaaaaaaaaaaaaaaaaaaa,
  ///       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa);
  ///   }
  /// \endcode
  bool BinPackArguments;

  /// The style of inserting trailing commas into container literals.
  enum TrailingCommaStyle {
    /// Do not insert trailing commas.
    TCS_None,
    /// Insert trailing commas in container literals that were wrapped over
    /// multiple lines. Note that this is conceptually incompatible with
    /// bin-packing, because the trailing comma is used as an indicator
    /// that a container should be formatted one-per-line (i.e. not bin-packed).
    /// So inserting a trailing comma counteracts bin-packing.
    TCS_Wrapped,
  };

  /// If set to ``TCS_Wrapped`` will insert trailing commas in container
  /// literals (arrays and objects) that wrap across multiple lines.
  /// It is currently only available for JavaScript
  /// and disabled by default ``TCS_None``.
  /// ``InsertTrailingCommas`` cannot be used together with ``BinPackArguments``
  /// as inserting the comma disables bin-packing.
  /// \code
  ///   TSC_Wrapped:
  ///   const someArray = [
  ///   aaaaaaaaaaaaaaaaaaaaaaaaaa,
  ///   aaaaaaaaaaaaaaaaaaaaaaaaaa,
  ///   aaaaaaaaaaaaaaaaaaaaaaaaaa,
  ///   //                        ^ inserted
  ///   ]
  /// \endcode
  TrailingCommaStyle InsertTrailingCommas;

  /// If ``false``, a function declaration's or function definition's
  /// parameters will either all be on the same line or will have one line each.
  /// \code
  ///   true:
  ///   void f(int aaaaaaaaaaaaaaaaaaaa, int aaaaaaaaaaaaaaaaaaaa,
  ///          int aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa) {}
  ///
  ///   false:
  ///   void f(int aaaaaaaaaaaaaaaaaaaa,
  ///          int aaaaaaaaaaaaaaaaaaaa,
  ///          int aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa) {}
  /// \endcode
  bool BinPackParameters;

  /// The style of wrapping parameters on the same line (bin-packed) or
  /// on one line each.
  enum BinPackStyle {
    /// Automatically determine parameter bin-packing behavior.
    BPS_Auto,
    /// Always bin-pack parameters.
    BPS_Always,
    /// Never bin-pack parameters.
    BPS_Never,
  };

  /// The style of breaking before or after binary operators.
  enum BinaryOperatorStyle {
    /// Break after operators.
    /// \code
    ///    LooooooooooongType loooooooooooooooooooooongVariable =
    ///        someLooooooooooooooooongFunction();
    ///
    ///    bool value = aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa +
    ///                         aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa ==
    ///                     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa &&
    ///                 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa >
    ///                     ccccccccccccccccccccccccccccccccccccccccc;
    /// \endcode
    BOS_None,
    /// Break before operators that aren't assignments.
    /// \code
    ///    LooooooooooongType loooooooooooooooooooooongVariable =
    ///        someLooooooooooooooooongFunction();
    ///
    ///    bool value = aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    ///                         + aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    ///                     == aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    ///                 && aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    ///                        > ccccccccccccccccccccccccccccccccccccccccc;
    /// \endcode
    BOS_NonAssignment,
    /// Break before operators.
    /// \code
    ///    LooooooooooongType loooooooooooooooooooooongVariable
    ///        = someLooooooooooooooooongFunction();
    ///
    ///    bool value = aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    ///                         + aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    ///                     == aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    ///                 && aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    ///                        > ccccccccccccccccccccccccccccccccccccccccc;
    /// \endcode
    BOS_All,
  };

  /// The way to wrap binary operators.
  BinaryOperatorStyle BreakBeforeBinaryOperators;

  /// Different ways to attach braces to their surrounding context.
  enum BraceBreakingStyle {
    /// Always attach braces to surrounding context.
    /// \code
    ///   try {
    ///     foo();
    ///   } catch () {
    ///   }
    ///   void foo() { bar(); }
    ///   class foo {};
    ///   if (foo()) {
    ///   } else {
    ///   }
    ///   enum X : int { A, B };
    /// \endcode
    BS_Attach,
    /// Like ``Attach``, but break before braces on function, namespace and
    /// class definitions.
    /// \code
    ///   try {
    ///     foo();
    ///   } catch () {
    ///   }
    ///   void foo() { bar(); }
    ///   class foo
    ///   {
    ///   };
    ///   if (foo()) {
    ///   } else {
    ///   }
    ///   enum X : int { A, B };
    /// \endcode
    BS_Linux,
    /// Like ``Attach``, but break before braces on enum, function, and record
    /// definitions.
    /// \code
    ///   try {
    ///     foo();
    ///   } catch () {
    ///   }
    ///   void foo() { bar(); }
    ///   class foo
    ///   {
    ///   };
    ///   if (foo()) {
    ///   } else {
    ///   }
    ///   enum X : int { A, B };
    /// \endcode
    BS_Mozilla,
    /// Like ``Attach``, but break before function definitions, ``catch``, and
    /// ``else``.
    /// \code
    ///   try {
    ///     foo();
    ///   }
    ///   catch () {
    ///   }
    ///   void foo() { bar(); }
    ///   class foo {
    ///   };
    ///   if (foo()) {
    ///   }
    ///   else {
    ///   }
    ///   enum X : int { A, B };
    /// \endcode
    BS_Stroustrup,
    /// Always break before braces.
    /// \code
    ///   try
    ///   {
    ///     foo();
    ///   }
    ///   catch ()
    ///   {
    ///   }
    ///   void foo() { bar(); }
    ///   class foo
    ///   {
    ///   };
    ///   if (foo())
    ///   {
    ///   }
    ///   else
    ///   {
    ///   }
    ///   enum X : int
    ///   {
    ///     A,
    ///     B
    ///   };
    /// \endcode
    BS_Allman,
    /// Like ``Allman`` but always indent braces and line up code with braces.
    /// \code
    ///   try
    ///     {
    ///     foo();
    ///     }
    ///   catch ()
    ///     {
    ///     }
    ///   void foo() { bar(); }
    ///   class foo
    ///     {
    ///     };
    ///   if (foo())
    ///     {
    ///     }
    ///   else
    ///     {
    ///     }
    ///   enum X : int
    ///     {
    ///     A,
    ///     B
    ///     };
    /// \endcode
    BS_Whitesmiths,
    /// Always break before braces and add an extra level of indentation to
    /// braces of control statements, not to those of class, function
    /// or other definitions.
    /// \code
    ///   try
    ///     {
    ///       foo();
    ///     }
    ///   catch ()
    ///     {
    ///     }
    ///   void foo() { bar(); }
    ///   class foo
    ///   {
    ///   };
    ///   if (foo())
    ///     {
    ///     }
    ///   else
    ///     {
    ///     }
    ///   enum X : int
    ///   {
    ///     A,
    ///     B
    ///   };
    /// \endcode
    BS_GNU,
    /// Like ``Attach``, but break before functions.
    /// \code
    ///   try {
    ///     foo();
    ///   } catch () {
    ///   }
    ///   void foo() { bar(); }
    ///   class foo {
    ///   };
    ///   if (foo()) {
    ///   } else {
    ///   }
    ///   enum X : int { A, B };
    /// \endcode
    BS_WebKit,
    /// Configure each individual brace in `BraceWrapping`.
    BS_Custom
  };

  /// The brace breaking style to use.
  BraceBreakingStyle BreakBeforeBraces;

  /// Different ways to wrap braces after control statements.
  enum BraceWrappingAfterControlStatementStyle {
    /// Never wrap braces after a control statement.
    /// \code
    ///   if (foo()) {
    ///   } else {
    ///   }
    ///   for (int i = 0; i < 10; ++i) {
    ///   }
    /// \endcode
    BWACS_Never,
    /// Only wrap braces after a multi-line control statement.
    /// \code
    ///   if (foo && bar &&
    ///       baz)
    ///   {
    ///     quux();
    ///   }
    ///   while (foo || bar) {
    ///   }
    /// \endcode
    BWACS_MultiLine,
    /// Always wrap braces after a control statement.
    /// \code
    ///   if (foo())
    ///   {
    ///   } else
    ///   {}
    ///   for (int i = 0; i < 10; ++i)
    ///   {}
    /// \endcode
    BWACS_Always
  };

  /// Precise control over the wrapping of braces.
  /// \code
  ///   # Should be declared this way:
  ///   BreakBeforeBraces: Custom
  ///   BraceWrapping:
  ///       AfterClass: true
  /// \endcode
  struct BraceWrappingFlags {
    /// Wrap case labels.
    /// \code
    ///   false:                                true:
    ///   switch (foo) {                vs.     switch (foo) {
    ///     case 1: {                             case 1:
    ///       bar();                              {
    ///       break;                                bar();
    ///     }                                       break;
    ///     default: {                            }
    ///       plop();                             default:
    ///     }                                     {
    ///   }                                         plop();
    ///                                           }
    ///                                         }
    /// \endcode
    bool AfterCaseLabel;
    /// Wrap class definitions.
    /// \code
    ///   true:
    ///   class foo {};
    ///
    ///   false:
    ///   class foo
    ///   {};
    /// \endcode
    bool AfterClass;

    /// Wrap control statements (``if``/``for``/``while``/``switch``/..).
    BraceWrappingAfterControlStatementStyle AfterControlStatement;
    /// Wrap enum definitions.
    /// \code
    ///   true:
    ///   enum X : int
    ///   {
    ///     B
    ///   };
    ///
    ///   false:
    ///   enum X : int { B };
    /// \endcode
    bool AfterEnum;
    /// Wrap function definitions.
    /// \code
    ///   true:
    ///   void foo()
    ///   {
    ///     bar();
    ///     bar2();
    ///   }
    ///
    ///   false:
    ///   void foo() {
    ///     bar();
    ///     bar2();
    ///   }
    /// \endcode
    bool AfterFunction;
    /// Wrap namespace definitions.
    /// \code
    ///   true:
    ///   namespace
    ///   {
    ///   int foo();
    ///   int bar();
    ///   }
    ///
    ///   false:
    ///   namespace {
    ///   int foo();
    ///   int bar();
    ///   }
    /// \endcode
    bool AfterNamespace;
    /// Wrap ObjC definitions (interfaces, implementations...).
    /// \note @autoreleasepool and @synchronized blocks are wrapped
    /// according to `AfterControlStatement` flag.
    bool AfterObjCDeclaration;
    /// Wrap struct definitions.
    /// \code
    ///   true:
    ///   struct foo
    ///   {
    ///     int x;
    ///   };
    ///
    ///   false:
    ///   struct foo {
    ///     int x;
    ///   };
    /// \endcode
    bool AfterStruct;
    /// Wrap union definitions.
    /// \code
    ///   true:
    ///   union foo
    ///   {
    ///     int x;
    ///   }
    ///
    ///   false:
    ///   union foo {
    ///     int x;
    ///   }
    /// \endcode
    bool AfterUnion;
    /// Wrap extern blocks.
    /// \code
    ///   true:
    ///   extern "C"
    ///   {
    ///     int foo();
    ///   }
    ///
    ///   false:
    ///   extern "C" {
    ///   int foo();
    ///   }
    /// \endcode
    bool AfterExternBlock; // Partially superseded by IndentExternBlock
    /// Wrap before ``catch``.
    /// \code
    ///   true:
    ///   try {
    ///     foo();
    ///   }
    ///   catch () {
    ///   }
    ///
    ///   false:
    ///   try {
    ///     foo();
    ///   } catch () {
    ///   }
    /// \endcode
    bool BeforeCatch;
    /// Wrap before ``else``.
    /// \code
    ///   true:
    ///   if (foo()) {
    ///   }
    ///   else {
    ///   }
    ///
    ///   false:
    ///   if (foo()) {
    ///   } else {
    ///   }
    /// \endcode
    bool BeforeElse;
    /// Wrap lambda block.
    /// \code
    ///   true:
    ///   connect(
    ///     []()
    ///     {
    ///       foo();
    ///       bar();
    ///     });
    ///
    ///   false:
    ///   connect([]() {
    ///     foo();
    ///     bar();
    ///   });
    /// \endcode
    bool BeforeLambdaBody;
    /// Wrap before ``while``.
    /// \code
    ///   true:
    ///   do {
    ///     foo();
    ///   }
    ///   while (1);
    ///
    ///   false:
    ///   do {
    ///     foo();
    ///   } while (1);
    /// \endcode
    bool BeforeWhile;
    /// Indent the wrapped braces themselves.
    bool IndentBraces;
    /// If ``false``, empty function body can be put on a single line.
    /// This option is used only if the opening brace of the function has
    /// already been wrapped, i.e. the `AfterFunction` brace wrapping mode is
    /// set, and the function could/should not be put on a single line (as per
    /// `AllowShortFunctionsOnASingleLine` and constructor formatting options).
    /// \code
    ///   int f()   vs.   int f()
    ///   {}              {
    ///                   }
    /// \endcode
    ///
    bool SplitEmptyFunction;
    /// If ``false``, empty record (e.g. class, struct or union) body
    /// can be put on a single line. This option is used only if the opening
    /// brace of the record has already been wrapped, i.e. the `AfterClass`
    /// (for classes) brace wrapping mode is set.
    /// \code
    ///   class Foo   vs.  class Foo
    ///   {}               {
    ///                    }
    /// \endcode
    ///
    bool SplitEmptyRecord;
    /// If ``false``, empty namespace body can be put on a single line.
    /// This option is used only if the opening brace of the namespace has
    /// already been wrapped, i.e. the `AfterNamespace` brace wrapping mode is
    /// set.
    /// \code
    ///   namespace Foo   vs.  namespace Foo
    ///   {}                   {
    ///                        }
    /// \endcode
    ///
    bool SplitEmptyNamespace;
  };

  /// Control of individual brace wrapping cases.
  ///
  /// If ``BreakBeforeBraces`` is set to ``BS_Custom``, use this to specify how
  /// each individual brace case should be handled. Otherwise, this is ignored.
  /// \code{.yaml}
  ///   # Example of usage:
  ///   BreakBeforeBraces: Custom
  ///   BraceWrapping:
  ///     AfterEnum: true
  ///     AfterStruct: false
  ///     SplitEmptyFunction: false
  /// \endcode
  BraceWrappingFlags BraceWrapping;

  /// If ``true``, ternary operators will be placed after line breaks.
  /// \code
  ///    true:
  ///    veryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongDescription
  ///        ? firstValue
  ///        : SecondValueVeryVeryVeryVeryLong;
  ///
  ///    false:
  ///    veryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongDescription ?
  ///        firstValue :
  ///        SecondValueVeryVeryVeryVeryLong;
  /// \endcode
  bool BreakBeforeTernaryOperators;

  /// Different ways to break initializers.
  enum BreakConstructorInitializersStyle {
    /// Break constructor initializers before the colon and after the commas.
    /// \code
    ///    Constructor()
    ///        : initializer1(),
    ///          initializer2()
    /// \endcode
    BCIS_BeforeColon,
    /// Break constructor initializers before the colon and commas, and align
    /// the commas with the colon.
    /// \code
    ///    Constructor()
    ///        : initializer1()
    ///        , initializer2()
    /// \endcode
    BCIS_BeforeComma,
    /// Break constructor initializers after the colon and commas.
    /// \code
    ///    Constructor() :
    ///        initializer1(),
    ///        initializer2()
    /// \endcode
    BCIS_AfterColon
  };

  /// The constructor initializers style to use.
  BreakConstructorInitializersStyle BreakConstructorInitializers;

  /// Break after each annotation on a field in Java files.
  /// \code{.java}
  ///    true:                                  false:
  ///    @Partial                       vs.     @Partial @Mock DataLoad loader;
  ///    @Mock
  ///    DataLoad loader;
  /// \endcode
  bool BreakAfterJavaFieldAnnotations;

  /// Allow breaking string literals when formatting.
  /// \code
  ///    true:
  ///    const char* x = "veryVeryVeryVeryVeryVe"
  ///                    "ryVeryVeryVeryVeryVery"
  ///                    "VeryLongString";
  ///
  ///    false:
  ///    const char* x =
  ///      "veryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongString";
  /// \endcode
  bool BreakStringLiterals;

  /// The column limit.
  ///
  /// A column limit of ``0`` means that there is no column limit. In this case,
  /// clang-format will respect the input's line breaking decisions within
  /// statements unless they contradict other rules.
  unsigned ColumnLimit;

  /// A regular expression that describes comments with special meaning,
  /// which should not be split into lines or otherwise changed.
  /// \code
  ///    // CommentPragmas: '^ FOOBAR pragma:'
  ///    // Will leave the following line unaffected
  ///    #include <vector> // FOOBAR pragma: keep
  /// \endcode
  std::string CommentPragmas;

  /// Different ways to break inheritance list.
  enum BreakInheritanceListStyle {
    /// Break inheritance list before the colon and after the commas.
    /// \code
    ///    class Foo
    ///        : Base1,
    ///          Base2
    ///    {};
    /// \endcode
    BILS_BeforeColon,
    /// Break inheritance list before the colon and commas, and align
    /// the commas with the colon.
    /// \code
    ///    class Foo
    ///        : Base1
    ///        , Base2
    ///    {};
    /// \endcode
    BILS_BeforeComma,
    /// Break inheritance list after the colon and commas.
    /// \code
    ///    class Foo :
    ///        Base1,
    ///        Base2
    ///    {};
    /// \endcode
    BILS_AfterColon
  };

  /// The inheritance list style to use.
  BreakInheritanceListStyle BreakInheritanceList;

  /// If ``true``, consecutive namespace declarations will be on the same
  /// line. If ``false``, each namespace is declared on a new line.
  /// \code
  ///   true:
  ///   namespace Foo { namespace Bar {
  ///   }}
  ///
  ///   false:
  ///   namespace Foo {
  ///   namespace Bar {
  ///   }
  ///   }
  /// \endcode
  ///
  /// If it does not fit on a single line, the overflowing namespaces get
  /// wrapped:
  /// \code
  ///   namespace Foo { namespace Bar {
  ///   namespace Extra {
  ///   }}}
  /// \endcode
  bool CompactNamespaces;

  // clang-format off
  /// If the constructor initializers don't fit on a line, put each
  /// initializer on its own line.
  /// \code
  ///   true:
  ///   SomeClass::Constructor()
  ///       : aaaaaaaa(aaaaaaaa), aaaaaaaa(aaaaaaaa), aaaaaaaa(aaaaaaaaaaaaaaaaaaaaaaaaa) {
  ///     return 0;
  ///   }
  ///
  ///   false:
  ///   SomeClass::Constructor()
  ///       : aaaaaaaa(aaaaaaaa), aaaaaaaa(aaaaaaaa),
  ///         aaaaaaaa(aaaaaaaaaaaaaaaaaaaaaaaaa) {
  ///     return 0;
  ///   }
  /// \endcode
  bool ConstructorInitializerAllOnOneLineOrOnePerLine;
  // clang-format on

  /// The number of characters to use for indentation of constructor
  /// initializer lists as well as inheritance lists.
  unsigned ConstructorInitializerIndentWidth;

  /// Indent width for line continuations.
  /// \code
  ///    ContinuationIndentWidth: 2
  ///
  ///    int i =         //  VeryVeryVeryVeryVeryLongComment
  ///      longFunction( // Again a long comment
  ///        arg);
  /// \endcode
  unsigned ContinuationIndentWidth;

  /// If ``true``, format braced lists as best suited for C++11 braced
  /// lists.
  ///
  /// Important differences:
  /// - No spaces inside the braced list.
  /// - No line break before the closing brace.
  /// - Indentation with the continuation indent, not with the block indent.
  ///
  /// Fundamentally, C++11 braced lists are formatted exactly like function
  /// calls would be formatted in their place. If the braced list follows a name
  /// (e.g. a type or variable name), clang-format formats as if the ``{}`` were
  /// the parentheses of a function call with that name. If there is no name,
  /// a zero-length name is assumed.
  /// \code
  ///    true:                                  false:
  ///    vector<int> x{1, 2, 3, 4};     vs.     vector<int> x{ 1, 2, 3, 4 };
  ///    vector<T> x{{}, {}, {}, {}};           vector<T> x{ {}, {}, {}, {} };
  ///    f(MyMap[{composite, key}]);            f(MyMap[{ composite, key }]);
  ///    new int[3]{1, 2, 3};                   new int[3]{ 1, 2, 3 };
  /// \endcode
  bool Cpp11BracedListStyle;

  /// \brief Analyze the formatted file for the most used line ending (``\r\n``
  /// or ``\n``). ``UseCRLF`` is only used as a fallback if none can be derived.
  bool DeriveLineEnding;

  /// If ``true``, analyze the formatted file for the most common
  /// alignment of ``&`` and ``*``.
  /// Pointer and reference alignment styles are going to be updated according
  /// to the preferences found in the file.
  /// ``PointerAlignment`` is then used only as fallback.
  bool DerivePointerAlignment;

  /// Disables formatting completely.
  bool DisableFormat;

  /// If ``true``, clang-format detects whether function calls and
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

  /// If ``true``, clang-format adds missing namespace end comments and
  /// fixes invalid existing ones.
  /// \code
  ///    true:                                  false:
  ///    namespace a {                  vs.     namespace a {
  ///    foo();                                 foo();
  ///    } // namespace a                       }
  /// \endcode
  bool FixNamespaceComments;

  /// A vector of macros that should be interpreted as foreach loops
  /// instead of as function calls.
  ///
  /// These are expected to be macros of the form:
  /// \code
  ///   FOREACH(<variable-declaration>, ...)
  ///     <loop-body>
  /// \endcode
  ///
  /// In the .clang-format configuration file, this can be configured like:
  /// \code{.yaml}
  ///   ForEachMacros: ['RANGES_FOR', 'FOREACH']
  /// \endcode
  ///
  /// For example: BOOST_FOREACH.
  std::vector<std::string> ForEachMacros;

  /// \brief A vector of macros that should be interpreted as type declarations
  /// instead of as function calls.
  ///
  /// These are expected to be macros of the form:
  /// \code
  ///   STACK_OF(...)
  /// \endcode
  ///
  /// In the .clang-format configuration file, this can be configured like:
  /// \code{.yaml}
  ///   TypenameMacros: ['STACK_OF', 'LIST']
  /// \endcode
  ///
  /// For example: OpenSSL STACK_OF, BSD LIST_ENTRY.
  std::vector<std::string> TypenameMacros;

  /// A vector of macros that should be interpreted as complete
  /// statements.
  ///
  /// Typical macros are expressions, and require a semi-colon to be
  /// added; sometimes this is not the case, and this allows to make
  /// clang-format aware of such cases.
  ///
  /// For example: Q_UNUSED
  std::vector<std::string> StatementMacros;

  /// A vector of macros which are used to open namespace blocks.
  ///
  /// These are expected to be macros of the form:
  /// \code
  ///   NAMESPACE(<namespace-name>, ...) {
  ///     <namespace-content>
  ///   }
  /// \endcode
  ///
  /// For example: TESTSUITE
  std::vector<std::string> NamespaceMacros;

  /// A vector of macros which are whitespace-sensitive and shouldn't be
  /// touched.
  ///
  /// These are expected to be macros of the form:
  /// \code
  ///   STRINGIZE(...)
  /// \endcode
  ///
  /// For example: STRINGIZE
  std::vector<std::string> WhitespaceSensitiveMacros;

  clang_v11::IncludeStyle IncludeStyle;

  /// Indent case labels one level from the switch statement.
  ///
  /// When ``false``, use the same indentation level as for the switch
  /// statement. Switch statement body is always indented one level more than
  /// case labels (except the first block following the case label, which
  /// itself indents the code - unless IndentCaseBlocks is enabled).
  /// \code
  ///    false:                                 true:
  ///    switch (fool) {                vs.     switch (fool) {
  ///    case 1:                                  case 1:
  ///      bar();                                   bar();
  ///      break;                                   break;
  ///    default:                                 default:
  ///      plop();                                  plop();
  ///    }                                      }
  /// \endcode
  bool IndentCaseLabels;

  /// Indent case label blocks one level from the case label.
  ///
  /// When ``false``, the block following the case label uses the same
  /// indentation level as for the case label, treating the case label the same
  /// as an if-statement.
  /// When ``true``, the block gets indented as a scope block.
  /// \code
  ///    false:                                 true:
  ///    switch (fool) {                vs.     switch (fool) {
  ///    case 1: {                              case 1:
  ///      bar();                                 {
  ///    } break;                                   bar();
  ///    default: {                               }
  ///      plop();                                break;
  ///    }                                      default:
  ///    }                                        {
  ///                                               plop();
  ///                                             }
  ///                                           }
  /// \endcode
  bool IndentCaseBlocks;

  /// Indent goto labels.
  ///
  /// When ``false``, goto labels are flushed left.
  /// \code
  ///    true:                                  false:
  ///    int f() {                      vs.     int f() {
  ///      if (foo()) {                           if (foo()) {
  ///      label1:                              label1:
  ///        bar();                                 bar();
  ///      }                                      }
  ///    label2:                                label2:
  ///      return 1;                              return 1;
  ///    }                                      }
  /// \endcode
  bool IndentGotoLabels;

  /// Options for indenting preprocessor directives.
  enum PPDirectiveIndentStyle {
    /// Does not indent any directives.
    /// \code
    ///    #if FOO
    ///    #if BAR
    ///    #include <foo>
    ///    #endif
    ///    #endif
    /// \endcode
    PPDIS_None,
    /// Indents directives after the hash.
    /// \code
    ///    #if FOO
    ///    #  if BAR
    ///    #    include <foo>
    ///    #  endif
    ///    #endif
    /// \endcode
    PPDIS_AfterHash,
    /// Indents directives before the hash.
    /// \code
    ///    #if FOO
    ///      #if BAR
    ///        #include <foo>
    ///      #endif
    ///    #endif
    /// \endcode
    PPDIS_BeforeHash
  };

  /// The preprocessor directive indenting style to use.
  PPDirectiveIndentStyle IndentPPDirectives;

  /// Indents extern blocks
  enum IndentExternBlockStyle {
    /// Backwards compatible with AfterExternBlock's indenting.
    /// \code
    ///    IndentExternBlock: AfterExternBlock
    ///    BraceWrapping.AfterExternBlock: true
    ///    extern "C"
    ///    {
    ///        void foo();
    ///    }
    /// \endcode
    ///
    /// \code
    ///    IndentExternBlock: AfterExternBlock
    ///    BraceWrapping.AfterExternBlock: false
    ///    extern "C" {
    ///    void foo();
    ///    }
    /// \endcode
    IEBS_AfterExternBlock,
    /// Does not indent extern blocks.
    /// \code
    ///     extern "C" {
    ///     void foo();
    ///     }
    /// \endcode
    IEBS_NoIndent,
    /// Indents extern blocks.
    /// \code
    ///     extern "C" {
    ///       void foo();
    ///     }
    /// \endcode
    IEBS_Indent,
  };

  /// IndentExternBlockStyle is the type of indenting of extern blocks.
  IndentExternBlockStyle IndentExternBlock;

  /// The number of columns to use for indentation.
  /// \code
  ///    IndentWidth: 3
  ///
  ///    void f() {
  ///       someFunction();
  ///       if (true, false) {
  ///          f();
  ///       }
  ///    }
  /// \endcode
  unsigned IndentWidth;

  /// Indent if a function definition or declaration is wrapped after the
  /// type.
  /// \code
  ///    true:
  ///    LoooooooooooooooooooooooooooooooooooooooongReturnType
  ///        LoooooooooooooooooooooooooooooooongFunctionDeclaration();
  ///
  ///    false:
  ///    LoooooooooooooooooooooooooooooooooooooooongReturnType
  ///    LoooooooooooooooooooooooooooooooongFunctionDeclaration();
  /// \endcode
  bool IndentWrappedFunctionNames;

  /// A vector of prefixes ordered by the desired groups for Java imports.
  ///
  /// Each group is separated by a newline. Static imports will also follow the
  /// same grouping convention above all non-static imports. One group's prefix
  /// can be a subset of another - the longest prefix is always matched. Within
  /// a group, the imports are ordered lexicographically.
  ///
  /// In the .clang-format configuration file, this can be configured like
  /// in the following yaml example. This will result in imports being
  /// formatted as in the Java example below.
  /// \code{.yaml}
  ///   JavaImportGroups: ['com.example', 'com', 'org']
  /// \endcode
  ///
  /// \code{.java}
  ///    import static com.example.function1;
  ///
  ///    import static com.test.function2;
  ///
  ///    import static org.example.function3;
  ///
  ///    import com.example.ClassA;
  ///    import com.example.Test;
  ///    import com.example.a.ClassB;
  ///
  ///    import com.test.ClassC;
  ///
  ///    import org.example.ClassD;
  /// \endcode
  std::vector<std::string> JavaImportGroups;

  /// Quotation styles for JavaScript strings. Does not affect template
  /// strings.
  enum JavaScriptQuoteStyle {
    /// Leave string quotes as they are.
    /// \code{.js}
    ///    string1 = "foo";
    ///    string2 = 'bar';
    /// \endcode
    JSQS_Leave,
    /// Always use single quotes.
    /// \code{.js}
    ///    string1 = 'foo';
    ///    string2 = 'bar';
    /// \endcode
    JSQS_Single,
    /// Always use double quotes.
    /// \code{.js}
    ///    string1 = "foo";
    ///    string2 = "bar";
    /// \endcode
    JSQS_Double
  };

  /// The JavaScriptQuoteStyle to use for JavaScript strings.
  JavaScriptQuoteStyle JavaScriptQuotes;

  // clang-format off
  /// Whether to wrap JavaScript import/export statements.
  /// \code{.js}
  ///    true:
  ///    import {
  ///        VeryLongImportsAreAnnoying,
  ///        VeryLongImportsAreAnnoying,
  ///        VeryLongImportsAreAnnoying,
  ///    } from 'some/module.js'
  ///
  ///    false:
  ///    import {VeryLongImportsAreAnnoying, VeryLongImportsAreAnnoying, VeryLongImportsAreAnnoying,} from "some/module.js"
  /// \endcode
  bool JavaScriptWrapImports;
  // clang-format on

  /// If true, the empty line at the start of blocks is kept.
  /// \code
  ///    true:                                  false:
  ///    if (foo) {                     vs.     if (foo) {
  ///                                             bar();
  ///      bar();                               }
  ///    }
  /// \endcode
  bool KeepEmptyLinesAtTheStartOfBlocks;

  /// Supported languages.
  ///
  /// When stored in a configuration file, specifies the language, that the
  /// configuration targets. When passed to the ``reformat()`` function, enables
  /// syntax features specific to the language.
  enum LanguageKind {
    /// Do not use.
    LK_None,
    /// Should be used for C, C++.
    LK_Cpp,
    /// Should be used for C#.
    LK_CSharp,
    /// Should be used for Java.
    LK_Java,
    /// Should be used for JavaScript.
    LK_JavaScript,
    /// Should be used for Objective-C, Objective-C++.
    LK_ObjC,
    /// Should be used for Protocol Buffers
    /// (https://developers.google.com/protocol-buffers/).
    LK_Proto,
    /// Should be used for TableGen code.
    LK_TableGen,
    /// Should be used for Protocol Buffer messages in text format
    /// (https://developers.google.com/protocol-buffers/).
    LK_TextProto
  };
  bool isCpp() const { return Language == LK_Cpp || Language == LK_ObjC; }
  bool isCSharp() const { return Language == LK_CSharp; }

  /// Language, this format style is targeted at.
  LanguageKind Language;

  /// A regular expression matching macros that start a block.
  /// \code
  ///    # With:
  ///    MacroBlockBegin: "^NS_MAP_BEGIN|\
  ///    NS_TABLE_HEAD$"
  ///    MacroBlockEnd: "^\
  ///    NS_MAP_END|\
  ///    NS_TABLE_.*_END$"
  ///
  ///    NS_MAP_BEGIN
  ///      foo();
  ///    NS_MAP_END
  ///
  ///    NS_TABLE_HEAD
  ///      bar();
  ///    NS_TABLE_FOO_END
  ///
  ///    # Without:
  ///    NS_MAP_BEGIN
  ///    foo();
  ///    NS_MAP_END
  ///
  ///    NS_TABLE_HEAD
  ///    bar();
  ///    NS_TABLE_FOO_END
  /// \endcode
  std::string MacroBlockBegin;

  /// A regular expression matching macros that end a block.
  std::string MacroBlockEnd;

  /// The maximum number of consecutive empty lines to keep.
  /// \code
  ///    MaxEmptyLinesToKeep: 1         vs.     MaxEmptyLinesToKeep: 0
  ///    int f() {                              int f() {
  ///      int = 1;                                 int i = 1;
  ///                                               i = foo();
  ///      i = foo();                               return i;
  ///                                           }
  ///      return i;
  ///    }
  /// \endcode
  unsigned MaxEmptyLinesToKeep;

  /// Different ways to indent namespace contents.
  enum NamespaceIndentationKind {
    /// Don't indent in namespaces.
    /// \code
    ///    namespace out {
    ///    int i;
    ///    namespace in {
    ///    int i;
    ///    }
    ///    }
    /// \endcode
    NI_None,
    /// Indent only in inner namespaces (nested in other namespaces).
    /// \code
    ///    namespace out {
    ///    int i;
    ///    namespace in {
    ///      int i;
    ///    }
    ///    }
    /// \endcode
    NI_Inner,
    /// Indent in all namespaces.
    /// \code
    ///    namespace out {
    ///      int i;
    ///      namespace in {
    ///        int i;
    ///      }
    ///    }
    /// \endcode
    NI_All
  };

  /// The indentation used for namespaces.
  NamespaceIndentationKind NamespaceIndentation;

  /// Controls bin-packing Objective-C protocol conformance list
  /// items into as few lines as possible when they go over ``ColumnLimit``.
  ///
  /// If ``Auto`` (the default), delegates to the value in
  /// ``BinPackParameters``. If that is ``true``, bin-packs Objective-C
  /// protocol conformance list items into as few lines as possible
  /// whenever they go over ``ColumnLimit``.
  ///
  /// If ``Always``, always bin-packs Objective-C protocol conformance
  /// list items into as few lines as possible whenever they go over
  /// ``ColumnLimit``.
  ///
  /// If ``Never``, lays out Objective-C protocol conformance list items
  /// onto individual lines whenever they go over ``ColumnLimit``.
  ///
  /// \code{.objc}
  ///    Always (or Auto, if BinPackParameters=true):
  ///    @interface ccccccccccccc () <
  ///        ccccccccccccc, ccccccccccccc,
  ///        ccccccccccccc, ccccccccccccc> {
  ///    }
  ///
  ///    Never (or Auto, if BinPackParameters=false):
  ///    @interface ddddddddddddd () <
  ///        ddddddddddddd,
  ///        ddddddddddddd,
  ///        ddddddddddddd,
  ///        ddddddddddddd> {
  ///    }
  /// \endcode
  BinPackStyle ObjCBinPackProtocolList;

  /// The number of characters to use for indentation of ObjC blocks.
  /// \code{.objc}
  ///    ObjCBlockIndentWidth: 4
  ///
  ///    [operation setCompletionBlock:^{
  ///        [self onOperationDone];
  ///    }];
  /// \endcode
  unsigned ObjCBlockIndentWidth;

  /// Add a space after ``@property`` in Objective-C, i.e. use
  /// ``@property (readonly)`` instead of ``@property(readonly)``.
  bool ObjCSpaceAfterProperty;

  /// Break parameters list into lines when there is nested block
  /// parameters in a fuction call.
  /// \code
  ///   false:
  ///    - (void)_aMethod
  ///    {
  ///        [self.test1 t:self w:self callback:^(typeof(self) self, NSNumber
  ///        *u, NSNumber *v) {
  ///            u = c;
  ///        }]
  ///    }
  ///    true:
  ///    - (void)_aMethod
  ///    {
  ///       [self.test1 t:self
  ///                    w:self
  ///           callback:^(typeof(self) self, NSNumber *u, NSNumber *v) {
  ///                u = c;
  ///            }]
  ///    }
  /// \endcode
  bool ObjCBreakBeforeNestedBlockParam;

  /// Add a space in front of an Objective-C protocol list, i.e. use
  /// ``Foo <Protocol>`` instead of ``Foo<Protocol>``.
  bool ObjCSpaceBeforeProtocolList;

  /// The penalty for breaking around an assignment operator.
  unsigned PenaltyBreakAssignment;

  /// The penalty for breaking a function call after ``call(``.
  unsigned PenaltyBreakBeforeFirstCallParameter;

  /// The penalty for each line break introduced inside a comment.
  unsigned PenaltyBreakComment;

  /// The penalty for breaking before the first ``<<``.
  unsigned PenaltyBreakFirstLessLess;

  /// The penalty for each line break introduced inside a string literal.
  unsigned PenaltyBreakString;

  /// The penalty for breaking after template declaration.
  unsigned PenaltyBreakTemplateDeclaration;

  /// The penalty for each character outside of the column limit.
  unsigned PenaltyExcessCharacter;

  /// Penalty for putting the return type of a function onto its own
  /// line.
  unsigned PenaltyReturnTypeOnItsOwnLine;

  /// The ``&`` and ``*`` alignment style.
  enum PointerAlignmentStyle {
    /// Align pointer to the left.
    /// \code
    ///   int* a;
    /// \endcode
    PAS_Left,
    /// Align pointer to the right.
    /// \code
    ///   int *a;
    /// \endcode
    PAS_Right,
    /// Align pointer in the middle.
    /// \code
    ///   int * a;
    /// \endcode
    PAS_Middle
  };

  /// Pointer and reference alignment style.
  PointerAlignmentStyle PointerAlignment;

  /// See documentation of ``RawStringFormats``.
  struct RawStringFormat {
    /// The language of this raw string.
    LanguageKind Language;
    /// A list of raw string delimiters that match this language.
    std::vector<std::string> Delimiters;
    /// A list of enclosing function names that match this language.
    std::vector<std::string> EnclosingFunctions;
    /// The canonical delimiter for this language.
    std::string CanonicalDelimiter;
    /// The style name on which this raw string format is based on.
    /// If not specified, the raw string format is based on the style that this
    /// format is based on.
    std::string BasedOnStyle;
    bool operator==(const RawStringFormat &Other) const {
      return Language == Other.Language && Delimiters == Other.Delimiters &&
             EnclosingFunctions == Other.EnclosingFunctions &&
             CanonicalDelimiter == Other.CanonicalDelimiter &&
             BasedOnStyle == Other.BasedOnStyle;
    }
  };

  /// Defines hints for detecting supported languages code blocks in raw
  /// strings.
  ///
  /// A raw string with a matching delimiter or a matching enclosing function
  /// name will be reformatted assuming the specified language based on the
  /// style for that language defined in the .clang-format file. If no style has
  /// been defined in the .clang-format file for the specific language, a
  /// predefined style given by 'BasedOnStyle' is used. If 'BasedOnStyle' is not
  /// found, the formatting is based on llvm style. A matching delimiter takes
  /// precedence over a matching enclosing function name for determining the
  /// language of the raw string contents.
  ///
  /// If a canonical delimiter is specified, occurrences of other delimiters for
  /// the same language will be updated to the canonical if possible.
  ///
  /// There should be at most one specification per language and each delimiter
  /// and enclosing function should not occur in multiple specifications.
  ///
  /// To configure this in the .clang-format file, use:
  /// \code{.yaml}
  ///   RawStringFormats:
  ///     - Language: TextProto
  ///         Delimiters:
  ///           - 'pb'
  ///           - 'proto'
  ///         EnclosingFunctions:
  ///           - 'PARSE_TEXT_PROTO'
  ///         BasedOnStyle: google
  ///     - Language: Cpp
  ///         Delimiters:
  ///           - 'cc'
  ///           - 'cpp'
  ///         BasedOnStyle: llvm
  ///         CanonicalDelimiter: 'cc'
  /// \endcode
  std::vector<RawStringFormat> RawStringFormats;

  // clang-format off
  /// If ``true``, clang-format will attempt to re-flow comments.
  /// \code
  ///    false:
  ///    // veryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongComment with plenty of information
  ///    /* second veryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongComment with plenty of information */
  ///
  ///    true:
  ///    // veryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongComment with plenty of
  ///    // information
  ///    /* second veryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongComment with plenty of
  ///     * information */
  /// \endcode
  bool ReflowComments;
  // clang-format on

  /// If ``true``, clang-format will sort ``#includes``.
  /// \code
  ///    false:                                 true:
  ///    #include "b.h"                 vs.     #include "a.h"
  ///    #include "a.h"                         #include "b.h"
  /// \endcode
  bool SortIncludes;

  /// If ``true``, clang-format will sort using declarations.
  ///
  /// The order of using declarations is defined as follows:
  /// Split the strings by "::" and discard any initial empty strings. The last
  /// element of each list is a non-namespace name; all others are namespace
  /// names. Sort the lists of names lexicographically, where the sort order of
  /// individual names is that all non-namespace names come before all namespace
  /// names, and within those groups, names are in case-insensitive
  /// lexicographic order.
  /// \code
  ///    false:                                 true:
  ///    using std::cout;               vs.     using std::cin;
  ///    using std::cin;                        using std::cout;
  /// \endcode
  bool SortUsingDeclarations;

  /// If ``true``, a space is inserted after C style casts.
  /// \code
  ///    true:                                  false:
  ///    (int) i;                       vs.     (int)i;
  /// \endcode
  bool SpaceAfterCStyleCast;

  /// If ``true``, a space is inserted after the logical not operator (``!``).
  /// \code
  ///    true:                                  false:
  ///    ! someExpression();            vs.     !someExpression();
  /// \endcode
  bool SpaceAfterLogicalNot;

  /// If \c true, a space will be inserted after the 'template' keyword.
  /// \code
  ///    true:                                  false:
  ///    template <int> void foo();     vs.     template<int> void foo();
  /// \endcode
  bool SpaceAfterTemplateKeyword;

  /// If ``false``, spaces will be removed before assignment operators.
  /// \code
  ///    true:                                  false:
  ///    int a = 5;                     vs.     int a= 5;
  ///    a += 42;                               a+= 42;
  /// \endcode
  bool SpaceBeforeAssignmentOperators;

  /// If ``true``, a space will be inserted before a C++11 braced list
  /// used to initialize an object (after the preceding identifier or type).
  /// \code
  ///    true:                                  false:
  ///    Foo foo { bar };               vs.     Foo foo{ bar };
  ///    Foo {};                                Foo{};
  ///    vector<int> { 1, 2, 3 };               vector<int>{ 1, 2, 3 };
  ///    new int[3] { 1, 2, 3 };                new int[3]{ 1, 2, 3 };
  /// \endcode
  bool SpaceBeforeCpp11BracedList;

  /// If ``false``, spaces will be removed before constructor initializer
  /// colon.
  /// \code
  ///    true:                                  false:
  ///    Foo::Foo() : a(a) {}                   Foo::Foo(): a(a) {}
  /// \endcode
  bool SpaceBeforeCtorInitializerColon;

  /// If ``false``, spaces will be removed before inheritance colon.
  /// \code
  ///    true:                                  false:
  ///    class Foo : Bar {}             vs.     class Foo: Bar {}
  /// \endcode
  bool SpaceBeforeInheritanceColon;

  /// Different ways to put a space before opening parentheses.
  enum SpaceBeforeParensOptions {
    /// Never put a space before opening parentheses.
    /// \code
    ///    void f() {
    ///      if(true) {
    ///        f();
    ///      }
    ///    }
    /// \endcode
    SBPO_Never,
    /// Put a space before opening parentheses only after control statement
    /// keywords (``for/if/while...``).
    /// \code
    ///    void f() {
    ///      if (true) {
    ///        f();
    ///      }
    ///    }
    /// \endcode
    SBPO_ControlStatements,
    /// Same as ``SBPO_ControlStatements`` except this option doesn't apply to
    /// ForEach macros. This is useful in projects where ForEach macros are
    /// treated as function calls instead of control statements.
    /// \code
    ///    void f() {
    ///      Q_FOREACH(...) {
    ///        f();
    ///      }
    ///    }
    /// \endcode
    SBPO_ControlStatementsExceptForEachMacros,
    /// Put a space before opening parentheses only if the parentheses are not
    /// empty i.e. '()'
    /// \code
    ///   void() {
    ///     if (true) {
    ///       f();
    ///       g (x, y, z);
    ///     }
    ///   }
    /// \endcode
    SBPO_NonEmptyParentheses,
    /// Always put a space before opening parentheses, except when it's
    /// prohibited by the syntax rules (in function-like macro definitions) or
    /// when determined by other style rules (after unary operators, opening
    /// parentheses, etc.)
    /// \code
    ///    void f () {
    ///      if (true) {
    ///        f ();
    ///      }
    ///    }
    /// \endcode
    SBPO_Always
  };

  /// Defines in which cases to put a space before opening parentheses.
  SpaceBeforeParensOptions SpaceBeforeParens;

  /// If ``false``, spaces will be removed before range-based for loop
  /// colon.
  /// \code
  ///    true:                                  false:
  ///    for (auto v : values) {}       vs.     for(auto v: values) {}
  /// \endcode
  bool SpaceBeforeRangeBasedForLoopColon;

  /// If ``true``, spaces will be inserted into ``{}``.
  /// \code
  ///    true:                                false:
  ///    void f() { }                   vs.   void f() {}
  ///    while (true) { }                     while (true) {}
  /// \endcode
  bool SpaceInEmptyBlock;

  /// If ``true``, spaces may be inserted into ``()``.
  /// \code
  ///    true:                                false:
  ///    void f( ) {                    vs.   void f() {
  ///      int x[] = {foo( ), bar( )};          int x[] = {foo(), bar()};
  ///      if (true) {                          if (true) {
  ///        f( );                                f();
  ///      }                                    }
  ///    }                                    }
  /// \endcode
  bool SpaceInEmptyParentheses;

  /// The number of spaces before trailing line comments
  /// (``//`` - comments).
  ///
  /// This does not affect trailing block comments (``/*`` - comments) as
  /// those commonly have different usage patterns and a number of special
  /// cases.
  /// \code
  ///    SpacesBeforeTrailingComments: 3
  ///    void f() {
  ///      if (true) {   // foo1
  ///        f();        // bar
  ///      }             // foo
  ///    }
  /// \endcode
  unsigned SpacesBeforeTrailingComments;

  /// If ``true``, spaces will be inserted after ``<`` and before ``>``
  /// in template argument lists.
  /// \code
  ///    true:                                  false:
  ///    static_cast< int >(arg);       vs.     static_cast<int>(arg);
  ///    std::function< void(int) > fct;        std::function<void(int)> fct;
  /// \endcode
  bool SpacesInAngles;

  /// If ``true``, spaces will be inserted around if/for/switch/while
  /// conditions.
  /// \code
  ///    true:                                  false:
  ///    if ( a )  { ... }              vs.     if (a) { ... }
  ///    while ( i < 5 )  { ... }               while (i < 5) { ... }
  /// \endcode
  bool SpacesInConditionalStatement;

  /// If ``true``, spaces are inserted inside container literals (e.g.
  /// ObjC and Javascript array and dict literals).
  /// \code{.js}
  ///    true:                                  false:
  ///    var arr = [ 1, 2, 3 ];         vs.     var arr = [1, 2, 3];
  ///    f({a : 1, b : 2, c : 3});              f({a: 1, b: 2, c: 3});
  /// \endcode
  bool SpacesInContainerLiterals;

  /// If ``true``, spaces may be inserted into C style casts.
  /// \code
  ///    true:                                  false:
  ///    x = ( int32 )y                 vs.     x = (int32)y
  /// \endcode
  bool SpacesInCStyleCastParentheses;

  /// If ``true``, spaces will be inserted after ``(`` and before ``)``.
  /// \code
  ///    true:                                  false:
  ///    t f( Deleted & ) & = delete;   vs.     t f(Deleted &) & = delete;
  /// \endcode
  bool SpacesInParentheses;

  /// If ``true``, spaces will be inserted after ``[`` and before ``]``.
  /// Lambdas without arguments or unspecified size array declarations will not
  /// be affected.
  /// \code
  ///    true:                                  false:
  ///    int a[ 5 ];                    vs.     int a[5];
  ///    std::unique_ptr<int[]> foo() {} // Won't be affected
  /// \endcode
  bool SpacesInSquareBrackets;

  /// If ``true``, spaces will be before  ``[``.
  /// Lambdas will not be affected. Only the first ``[`` will get a space added.
  /// \code
  ///    true:                                  false:
  ///    int a [5];                    vs.      int a[5];
  ///    int a [5][5];                 vs.      int a[5][5];
  /// \endcode
  bool SpaceBeforeSquareBrackets;

  /// Supported language standards for parsing and formatting C++ constructs.
  /// \code
  ///    Latest:                                vector<set<int>>
  ///    c++03                          vs.     vector<set<int> >
  /// \endcode
  ///
  /// The correct way to spell a specific language version is e.g. ``c++11``.
  /// The historical aliases ``Cpp03`` and ``Cpp11`` are deprecated.
  enum LanguageStandard {
    /// Parse and format as C++03.
    /// ``Cpp03`` is a deprecated alias for ``c++03``
    LS_Cpp03, // c++03
    /// Parse and format as C++11.
    LS_Cpp11, // c++11
    /// Parse and format as C++14.
    LS_Cpp14, // c++14
    /// Parse and format as C++17.
    LS_Cpp17, // c++17
    /// Parse and format as C++20.
    LS_Cpp20, // c++20
    /// Parse and format using the latest supported language version.
    /// ``Cpp11`` is a deprecated alias for ``Latest``
    LS_Latest,
    /// Automatic detection based on the input.
    LS_Auto,
  };

  /// Parse and format C++ constructs compatible with this standard.
  /// \code
  ///    c++03:                                 latest:
  ///    vector<set<int> > x;           vs.     vector<set<int>> x;
  /// \endcode
  LanguageStandard Standard;

  /// The number of columns used for tab stops.
  unsigned TabWidth;

  /// Different ways to use tab in formatting.
  enum UseTabStyle {
    /// Never use tab.
    UT_Never,
    /// Use tabs only for indentation.
    UT_ForIndentation,
    /// Fill all leading whitespace with tabs, and use spaces for alignment that
    /// appears within a line (e.g. consecutive assignments and declarations).
    UT_ForContinuationAndIndentation,
    /// Use tabs for line continuation and indentation, and spaces for
    /// alignment.
    UT_AlignWithSpaces,
    /// Use tabs whenever we need to fill whitespace that spans at least from
    /// one tab stop to the next one.
    UT_Always
  };

  /// \brief Use ``\r\n`` instead of ``\n`` for line breaks.
  /// Also used as fallback if ``DeriveLineEnding`` is true.
  bool UseCRLF;

  /// The way to use tab characters in the resulting file.
  UseTabStyle UseTab;

  bool operator==(const FormatStyle &R) const {
    return AccessModifierOffset == R.AccessModifierOffset &&
           AlignAfterOpenBracket == R.AlignAfterOpenBracket &&
           AlignConsecutiveAssignments == R.AlignConsecutiveAssignments &&
           AlignConsecutiveBitFields == R.AlignConsecutiveBitFields &&
           AlignConsecutiveDeclarations == R.AlignConsecutiveDeclarations &&
           AlignEscapedNewlines == R.AlignEscapedNewlines &&
           AlignOperands == R.AlignOperands &&
           AlignTrailingComments == R.AlignTrailingComments &&
           AllowAllArgumentsOnNextLine == R.AllowAllArgumentsOnNextLine &&
           AllowAllConstructorInitializersOnNextLine ==
               R.AllowAllConstructorInitializersOnNextLine &&
           AllowAllParametersOfDeclarationOnNextLine ==
               R.AllowAllParametersOfDeclarationOnNextLine &&
           AllowShortEnumsOnASingleLine == R.AllowShortEnumsOnASingleLine &&
           AllowShortBlocksOnASingleLine == R.AllowShortBlocksOnASingleLine &&
           AllowShortCaseLabelsOnASingleLine ==
               R.AllowShortCaseLabelsOnASingleLine &&
           AllowShortFunctionsOnASingleLine ==
               R.AllowShortFunctionsOnASingleLine &&
           AllowShortIfStatementsOnASingleLine ==
               R.AllowShortIfStatementsOnASingleLine &&
           AllowShortLambdasOnASingleLine == R.AllowShortLambdasOnASingleLine &&
           AllowShortLoopsOnASingleLine == R.AllowShortLoopsOnASingleLine &&
           AlwaysBreakAfterReturnType == R.AlwaysBreakAfterReturnType &&
           AlwaysBreakBeforeMultilineStrings ==
               R.AlwaysBreakBeforeMultilineStrings &&
           AlwaysBreakTemplateDeclarations ==
               R.AlwaysBreakTemplateDeclarations &&
           BinPackArguments == R.BinPackArguments &&
           BinPackParameters == R.BinPackParameters &&
           BreakBeforeBinaryOperators == R.BreakBeforeBinaryOperators &&
           BreakBeforeBraces == R.BreakBeforeBraces &&
           BreakBeforeTernaryOperators == R.BreakBeforeTernaryOperators &&
           BreakConstructorInitializers == R.BreakConstructorInitializers &&
           CompactNamespaces == R.CompactNamespaces &&
           BreakAfterJavaFieldAnnotations == R.BreakAfterJavaFieldAnnotations &&
           BreakStringLiterals == R.BreakStringLiterals &&
           ColumnLimit == R.ColumnLimit && CommentPragmas == R.CommentPragmas &&
           BreakInheritanceList == R.BreakInheritanceList &&
           ConstructorInitializerAllOnOneLineOrOnePerLine ==
               R.ConstructorInitializerAllOnOneLineOrOnePerLine &&
           ConstructorInitializerIndentWidth ==
               R.ConstructorInitializerIndentWidth &&
           ContinuationIndentWidth == R.ContinuationIndentWidth &&
           Cpp11BracedListStyle == R.Cpp11BracedListStyle &&
           DeriveLineEnding == R.DeriveLineEnding &&
           DerivePointerAlignment == R.DerivePointerAlignment &&
           DisableFormat == R.DisableFormat &&
           ExperimentalAutoDetectBinPacking ==
               R.ExperimentalAutoDetectBinPacking &&
           FixNamespaceComments == R.FixNamespaceComments &&
           ForEachMacros == R.ForEachMacros &&
           IncludeStyle.IncludeBlocks == R.IncludeStyle.IncludeBlocks &&
           IncludeStyle.IncludeCategories == R.IncludeStyle.IncludeCategories &&
           IncludeStyle.IncludeIsMainRegex ==
               R.IncludeStyle.IncludeIsMainRegex &&
           IncludeStyle.IncludeIsMainSourceRegex ==
               R.IncludeStyle.IncludeIsMainSourceRegex &&
           IndentCaseLabels == R.IndentCaseLabels &&
           IndentCaseBlocks == R.IndentCaseBlocks &&
           IndentGotoLabels == R.IndentGotoLabels &&
           IndentPPDirectives == R.IndentPPDirectives &&
           IndentExternBlock == R.IndentExternBlock &&
           IndentWidth == R.IndentWidth && Language == R.Language &&
           IndentWrappedFunctionNames == R.IndentWrappedFunctionNames &&
           JavaImportGroups == R.JavaImportGroups &&
           JavaScriptQuotes == R.JavaScriptQuotes &&
           JavaScriptWrapImports == R.JavaScriptWrapImports &&
           KeepEmptyLinesAtTheStartOfBlocks ==
               R.KeepEmptyLinesAtTheStartOfBlocks &&
           MacroBlockBegin == R.MacroBlockBegin &&
           MacroBlockEnd == R.MacroBlockEnd &&
           MaxEmptyLinesToKeep == R.MaxEmptyLinesToKeep &&
           NamespaceIndentation == R.NamespaceIndentation &&
           NamespaceMacros == R.NamespaceMacros &&
           ObjCBinPackProtocolList == R.ObjCBinPackProtocolList &&
           ObjCBlockIndentWidth == R.ObjCBlockIndentWidth &&
           ObjCBreakBeforeNestedBlockParam ==
               R.ObjCBreakBeforeNestedBlockParam &&
           ObjCSpaceAfterProperty == R.ObjCSpaceAfterProperty &&
           ObjCSpaceBeforeProtocolList == R.ObjCSpaceBeforeProtocolList &&
           PenaltyBreakAssignment == R.PenaltyBreakAssignment &&
           PenaltyBreakBeforeFirstCallParameter ==
               R.PenaltyBreakBeforeFirstCallParameter &&
           PenaltyBreakComment == R.PenaltyBreakComment &&
           PenaltyBreakFirstLessLess == R.PenaltyBreakFirstLessLess &&
           PenaltyBreakString == R.PenaltyBreakString &&
           PenaltyExcessCharacter == R.PenaltyExcessCharacter &&
           PenaltyReturnTypeOnItsOwnLine == R.PenaltyReturnTypeOnItsOwnLine &&
           PenaltyBreakTemplateDeclaration ==
               R.PenaltyBreakTemplateDeclaration &&
           PointerAlignment == R.PointerAlignment &&
           RawStringFormats == R.RawStringFormats &&
           SpaceAfterCStyleCast == R.SpaceAfterCStyleCast &&
           SpaceAfterLogicalNot == R.SpaceAfterLogicalNot &&
           SpaceAfterTemplateKeyword == R.SpaceAfterTemplateKeyword &&
           SpaceBeforeAssignmentOperators == R.SpaceBeforeAssignmentOperators &&
           SpaceBeforeCpp11BracedList == R.SpaceBeforeCpp11BracedList &&
           SpaceBeforeCtorInitializerColon ==
               R.SpaceBeforeCtorInitializerColon &&
           SpaceBeforeInheritanceColon == R.SpaceBeforeInheritanceColon &&
           SpaceBeforeParens == R.SpaceBeforeParens &&
           SpaceBeforeRangeBasedForLoopColon ==
               R.SpaceBeforeRangeBasedForLoopColon &&
           SpaceInEmptyBlock == R.SpaceInEmptyBlock &&
           SpaceInEmptyParentheses == R.SpaceInEmptyParentheses &&
           SpacesBeforeTrailingComments == R.SpacesBeforeTrailingComments &&
           SpacesInAngles == R.SpacesInAngles &&
           SpacesInConditionalStatement == R.SpacesInConditionalStatement &&
           SpacesInContainerLiterals == R.SpacesInContainerLiterals &&
           SpacesInCStyleCastParentheses == R.SpacesInCStyleCastParentheses &&
           SpacesInParentheses == R.SpacesInParentheses &&
           SpacesInSquareBrackets == R.SpacesInSquareBrackets &&
           SpaceBeforeSquareBrackets == R.SpaceBeforeSquareBrackets &&
           Standard == R.Standard && TabWidth == R.TabWidth &&
           StatementMacros == R.StatementMacros && UseTab == R.UseTab &&
           UseCRLF == R.UseCRLF && TypenameMacros == R.TypenameMacros;
  }

  std::optional<FormatStyle> GetLanguageStyle(LanguageKind Language) const;

  // Stores per-language styles. A FormatStyle instance inside has an empty
  // StyleSet. A FormatStyle instance returned by the Get method has its
  // StyleSet set to a copy of the originating StyleSet, effectively keeping the
  // internal representation of that StyleSet alive.
  //
  // The memory management and ownership reminds of a birds nest: chicks
  // leaving the nest take photos of the nest with them.
  struct FormatStyleSet {
    typedef std::map<FormatStyle::LanguageKind, FormatStyle> MapType;

    std::optional<FormatStyle> Get(FormatStyle::LanguageKind Language) const;

    // Adds \p Style to this FormatStyleSet. Style must not have an associated
    // FormatStyleSet.
    // Style.Language should be different than LK_None. If this FormatStyleSet
    // already contains an entry for Style.Language, that gets replaced with the
    // passed Style.
    void Add(FormatStyle Style);

    // Clears this FormatStyleSet.
    void Clear();

  private:
    std::shared_ptr<MapType> Styles;
  };

  static FormatStyleSet BuildStyleSetFromConfiguration(
      const FormatStyle &MainStyle,
      const std::vector<FormatStyle> &ConfigurationStyles);

private:
  FormatStyleSet StyleSet;

  friend std::error_code parseConfiguration(const std::string &Text,
                                            FormatStyle *Style);
};

/// Returns a format style complying with the LLVM coding standards:
/// http://llvm.org/docs/CodingStandards.html.
FormatStyle getLLVMStyle(
    FormatStyle::LanguageKind Language = FormatStyle::LanguageKind::LK_Cpp);

/// Returns a format style complying with one of Google's style guides:
/// http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml.
/// http://google-styleguide.googlecode.com/svn/trunk/javascriptguide.xml.
/// https://developers.google.com/protocol-buffers/docs/style.
FormatStyle getGoogleStyle(FormatStyle::LanguageKind Language);

/// Returns a format style complying with Chromium's style guide:
/// http://www.chromium.org/developers/coding-style.
FormatStyle getChromiumStyle(FormatStyle::LanguageKind Language);

/// Returns a format style complying with Mozilla's style guide:
/// https://developer.mozilla.org/en-US/docs/Developer_Guide/Coding_Style.
FormatStyle getMozillaStyle();

/// Returns a format style complying with Webkit's style guide:
/// http://www.webkit.org/coding/coding-style.html
FormatStyle getWebKitStyle();

/// Returns a format style complying with GNU Coding Standards:
/// http://www.gnu.org/prep/standards/standards.html
FormatStyle getGNUStyle();

/// Returns a format style complying with Microsoft style guide:
/// https://docs.microsoft.com/en-us/visualstudio/ide/editorconfig-code-style-settings-reference?view=vs-2017
FormatStyle getMicrosoftStyle(FormatStyle::LanguageKind Language);

/// Returns style indicating formatting should be not applied at all.
FormatStyle getNoStyle();

/// Gets a predefined style for the specified language by name.
///
/// Currently supported names: LLVM, Google, Chromium, Mozilla. Names are
/// compared case-insensitively.
///
/// Returns ``true`` if the Style has been set.
bool getPredefinedStyle(llvm::StringRef Name,
                        FormatStyle::LanguageKind Language, FormatStyle *Style);

std::vector<std::string> getStyleNames();

/// Parse configuration from YAML-formatted text.
///
/// Style->Language is used to get the base style, if the ``BasedOnStyle``
/// option is present.
///
/// The FormatStyleSet of Style is reset.
///
/// When ``BasedOnStyle`` is not present, options not present in the YAML
/// document, are retained in \p Style.
std::error_code parseConfiguration(const std::string &Text, FormatStyle *Style);

/// Gets configuration in a YAML string.
std::string configurationAsText(const FormatStyle &Style,
                                const std::string &DefaultStyleName,
                                bool SkipSameValue);

} // end namespace clang_v11

namespace std {
template <>
struct is_error_code_enum<clang_v11::ParseError> : std::true_type {};
} // namespace std
