#include "update.h"
#include <frozen/unordered_map.h>
#include <iostream>
#include <stdexcept>

namespace {
template <typename T, typename U>
void assignWithWarning(const std::string &old_field_name, T &old_field,
                       const std::string &new_field_name, U &new_field,
                       U new_value, const std::string &version) {
  if (new_field != new_value) {
    std::cout << "Warning when migrating to version " << version
              << ". Overriding field " << new_field_name << " from value "
              << new_field << " to " << new_value << " based on field "
              << old_field_name << " with value " << old_field
              << " from previous version.";
    new_field = new_value;
  }
}
} // namespace

namespace clang_update_v3_4 {

constexpr frozen::unordered_map<clang_v3_3::FormatStyle::LanguageStandard,
                                clang_v3_4::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v3_3::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v3_4::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v3_3::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v3_4::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v3_3::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v3_4::FormatStyle::LanguageStandard::LS_Auto}};

clang_v3_4::FormatStyle update(clang_v3_3::FormatStyle &old,
                               const std::string &style) {
  clang_v3_4::FormatStyle retval;
  if (!clang_v3_4::getPredefinedStyle(style, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.ColumnLimit = old.ColumnLimit;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.PointerBindsToType = old.PointerBindsToType;
  retval.DerivePointerBinding = old.DerivePointerBinding;
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.Standard = language_standard.at(old.Standard);
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  retval.AllowShortIfStatementsOnASingleLine =
      old.AllowShortIfStatementsOnASingleLine;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.AlignEscapedNewlinesLeft = old.AlignEscapedNewlinesLeft;

  return retval;
}

} // namespace clang_update_v3_4

namespace clang_update_v3_5 {

constexpr frozen::unordered_map<clang_v3_4::FormatStyle::LanguageStandard,
                                clang_v3_5::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v3_4::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v3_5::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v3_4::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v3_5::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v3_4::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v3_5::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<
    clang_v3_4::FormatStyle::NamespaceIndentationKind,
    clang_v3_5::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v3_4::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v3_5::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v3_4::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v3_5::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v3_4::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v3_5::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v3_4::FormatStyle::UseTabStyle,
                                clang_v3_5::FormatStyle::UseTabStyle, 3>
    use_tab_style{{clang_v3_4::FormatStyle::UseTabStyle::UT_Never,
                   clang_v3_5::FormatStyle::UseTabStyle::UT_Never},
                  {clang_v3_4::FormatStyle::UseTabStyle::UT_ForIndentation,
                   clang_v3_5::FormatStyle::UseTabStyle::UT_ForIndentation},
                  {clang_v3_4::FormatStyle::UseTabStyle::UT_Always,
                   clang_v3_5::FormatStyle::UseTabStyle::UT_Always}};

constexpr frozen::unordered_map<clang_v3_4::FormatStyle::BraceBreakingStyle,
                                clang_v3_5::FormatStyle::BraceBreakingStyle, 4>
    brace_breaking_style{
        {clang_v3_4::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v3_5::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v3_4::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v3_5::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v3_4::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v3_5::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v3_4::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v3_5::FormatStyle::BraceBreakingStyle::BS_Allman}};

constexpr frozen::unordered_map<
    bool, clang_v3_5::FormatStyle::PointerAlignmentStyle, 2>
    pointer_alignment{
        {false, clang_v3_5::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {true, clang_v3_5::FormatStyle::PointerAlignmentStyle::PAS_Left}};

constexpr frozen::unordered_map<
    bool, clang_v3_5::FormatStyle::SpaceBeforeParensOptions, 2>
    space_before_parens_options{
        {false, clang_v3_5::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {true, clang_v3_5::FormatStyle::SpaceBeforeParensOptions::
                   SBPO_ControlStatements}};

clang_v3_5::FormatStyle update(clang_v3_4::FormatStyle &old,
                               const std::string &style) {
  clang_v3_5::FormatStyle retval;
  if (!clang_v3_5::getPredefinedStyle(
          style, clang_v3_5::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.ColumnLimit = old.ColumnLimit;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  assignWithWarning("PointerBindsToType", old.PointerBindsToType,
                    "PointerAlignment", retval.PointerAlignment,
                    pointer_alignment.at(old.PointerBindsToType), "3.5");
  assignWithWarning("DerivePointerBinding", old.DerivePointerBinding,
                    "DerivePointerAlignment", retval.DerivePointerAlignment,
                    old.DerivePointerBinding, "3.5");
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.Standard = language_standard.at(old.Standard);
  retval.IndentCaseLabels = old.IndentCaseLabels;
  assignWithWarning("IndentFunctionDeclarationAfterType",
                    old.IndentFunctionDeclarationAfterType,
                    "IndentWrappedFunctionNames",
                    retval.IndentWrappedFunctionNames,
                    old.IndentFunctionDeclarationAfterType, "3.5");
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  retval.BreakConstructorInitializersBeforeComma =
      old.BreakConstructorInitializersBeforeComma;
  retval.AllowShortIfStatementsOnASingleLine =
      old.AllowShortIfStatementsOnASingleLine;
  retval.AllowShortLoopsOnASingleLine = old.AllowShortLoopsOnASingleLine;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AlignEscapedNewlinesLeft = old.AlignEscapedNewlinesLeft;
  retval.IndentWidth = old.IndentWidth;
  retval.TabWidth = old.TabWidth;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.AlwaysBreakTemplateDeclarations = old.AlwaysBreakTemplateDeclarations;
  retval.AlwaysBreakBeforeMultilineStrings =
      old.AlwaysBreakBeforeMultilineStrings;
  retval.UseTab = use_tab_style.at(old.UseTab);
  retval.BreakBeforeBinaryOperators = old.BreakBeforeBinaryOperators;
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInAngles = old.SpacesInAngles;
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  assignWithWarning(
      "SpaceAfterControlStatementKeyword",
      old.SpaceAfterControlStatementKeyword, "SpaceBeforeParens",
      retval.SpaceBeforeParens,
      space_before_parens_options.at(old.SpaceAfterControlStatementKeyword),
      "3.5");
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;

  return retval;
}

} // namespace clang_update_v3_5
