#include "update.h"
#include <frozen/unordered_map.h>
#include <iostream>
#include <stdexcept>
#include <string_view>

std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::string> &vec) {
  os << "{";
  for (size_t i = 0; i < vec.size(); ++i) {
    os << vec[i];
    if (i != vec.size() - 1)
      os << ", ";
  }
  os << "}";
  return os;
}

std::ostream &
operator<<(std::ostream &os,
           const std::vector<clang_v3_8::FormatStyle::IncludeCategory> &vec) {
  os << "{";
  for (size_t i = 0; i < vec.size(); ++i) {
    os << "Regex: " << vec[i].Regex << ", Priority: " << vec[i].Priority;
    if (i != vec.size() - 1)
      os << ", ";
  }
  os << "}";
  return os;
}

std::ostream &
operator<<(std::ostream &os,
           const clang_v3_8::FormatStyle::BraceWrappingFlags &BraceWrapping) {
  os << "AfterClass: " << BraceWrapping.AfterClass << ", ";
  os << "AfterControlStatement: " << BraceWrapping.AfterControlStatement
     << ", ";
  os << "AfterEnum: " << BraceWrapping.AfterEnum << ", ";
  os << "AfterFunction: " << BraceWrapping.AfterFunction << ", ";
  os << "AfterNamespace: " << BraceWrapping.AfterNamespace << ", ";
  os << "AfterObjCDeclaration: " << BraceWrapping.AfterObjCDeclaration << ", ";
  os << "AfterStruct: " << BraceWrapping.AfterStruct << ", ";
  os << "AfterUnion: " << BraceWrapping.AfterUnion << ", ";
  os << "BeforeCatch: " << BraceWrapping.BeforeCatch << ", ";
  os << "BeforeElse: " << BraceWrapping.BeforeElse << ", ";
  os << "IndentBraces: " << BraceWrapping.IndentBraces;
  return os;
}

namespace {

template <typename T, typename U>
void assignWithWarning(std::string_view old_field_name, T &old_field,
                       std::string_view new_field_name, U &new_field,
                       U new_value, std::string_view version) {
  if (new_field != new_value) {
    std::cout << "Warning when migrating to version " << version
              << ". Overriding field " << new_field_name << " from value "
              << new_field << " to " << new_value << " based on field "
              << old_field_name << " with value " << old_field
              << " from previous version.\n";
    new_field = new_value;
  }
}

template <typename T>
void newField(std::string_view field_name, std::string_view version,
              const T &field_value) {
  std::cout << "Info when migrating to version " << version
            << ". New field: " << field_name << " with value " << field_value
            << ".\n";
}

void improveField(std::string_view field_name, std::string_view new_option,
                  std::string_view version) {
  std::cout << "Info when migrating to version " << version << ". Field "
            << field_name << " has new feature " << new_option << ".\n";
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
  newField("PenaltyBreakComment", "3.4", retval.PenaltyBreakComment);
  newField("PenaltyBreakString", "3.4", retval.PenaltyBreakString);
  newField("PenaltyBreakFirstLessLess", "3.4",
           retval.PenaltyBreakFirstLessLess);
  newField("PenaltyBreakBeforeFirstCallParameter", "3.4",
           retval.PenaltyBreakBeforeFirstCallParameter);
  retval.PointerBindsToType = old.PointerBindsToType;
  retval.DerivePointerBinding = old.DerivePointerBinding;
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.Standard = language_standard.at(old.Standard);
  retval.IndentCaseLabels = old.IndentCaseLabels;
  newField("NamespaceIndentation", "3.4", retval.NamespaceIndentation);
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.BinPackParameters = old.BinPackParameters;
  newField("ExperimentalAutoDetectBinPacking", "3.4",
           retval.ExperimentalAutoDetectBinPacking);
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  newField("BreakConstructorInitializersBeforeComma", "3.4",
           retval.BreakConstructorInitializersBeforeComma);
  retval.AllowShortIfStatementsOnASingleLine =
      old.AllowShortIfStatementsOnASingleLine;
  newField("AllowShortLoopsOnASingleLine", "3.4",
           retval.AllowShortLoopsOnASingleLine);
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  newField("AlignTrailingComments", "3.4", retval.AlignTrailingComments);
  retval.AlignEscapedNewlinesLeft = old.AlignEscapedNewlinesLeft;
  newField("IndentWidth", "3.4", retval.IndentWidth);
  newField("TabWidth", "3.4", retval.TabWidth);
  newField("ConstructorInitializerIndentWidth", "3.4",
           retval.ConstructorInitializerIndentWidth);
  newField("AlwaysBreakTemplateDeclarations", "3.4",
           retval.AlwaysBreakTemplateDeclarations);
  newField("AlwaysBreakBeforeMultilineStrings", "3.4",
           retval.AlwaysBreakBeforeMultilineStrings);
  newField("UseTab", "3.4", retval.UseTab);
  newField("BreakBeforeBinaryOperators", "3.4",
           retval.BreakBeforeBinaryOperators);
  newField("BreakBeforeTernaryOperators", "3.4",
           retval.BreakBeforeTernaryOperators);
  newField("BreakBeforeBraces", "3.4", retval.BreakBeforeBraces);
  newField("Cpp11BracedListStyle", "3.4", retval.Cpp11BracedListStyle);
  newField("IndentFunctionDeclarationAfterType", "3.4",
           retval.IndentFunctionDeclarationAfterType);
  newField("SpacesInParentheses", "3.4", retval.SpacesInParentheses);
  newField("SpacesInAngles", "3.4", retval.SpacesInAngles);
  newField("SpaceInEmptyParentheses", "3.4", retval.SpaceInEmptyParentheses);
  newField("SpacesInCStyleCastParentheses", "3.4",
           retval.SpacesInCStyleCastParentheses);
  newField("SpaceAfterControlStatementKeyword", "3.4",
           retval.SpaceAfterControlStatementKeyword);
  newField("SpaceBeforeAssignmentOperators", "3.4",
           retval.SpaceBeforeAssignmentOperators);
  newField("ContinuationIndentWidth", "3.4", retval.ContinuationIndentWidth);

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

  newField("Language", "3.5", retval.Language);
  retval.ColumnLimit = old.ColumnLimit;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  newField("KeepEmptyLinesAtTheStartOfBlocks", "3.5",
           retval.KeepEmptyLinesAtTheStartOfBlocks);
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  improveField("PointerAlignment", "Middle", "3.5");
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
  newField("AllowShortBlocksOnASingleLine", "3.5",
           retval.AllowShortBlocksOnASingleLine);
  retval.AllowShortIfStatementsOnASingleLine =
      old.AllowShortIfStatementsOnASingleLine;
  retval.AllowShortLoopsOnASingleLine = old.AllowShortLoopsOnASingleLine;
  newField("AllowShortFunctionsOnASingleLine", "3.5",
           retval.AllowShortFunctionsOnASingleLine);
  newField("ObjCSpaceAfterProperty", "3.5", retval.ObjCSpaceAfterProperty);
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
  improveField("BreakBeforeBraces", "GNU", "3.5");
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInAngles = old.SpacesInAngles;
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  newField("SpacesInContainerLiterals", "3.5",
           retval.SpacesInContainerLiterals);
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  improveField("SpaceBeforeParens", "Always", "3.5");
  assignWithWarning(
      "SpaceAfterControlStatementKeyword",
      old.SpaceAfterControlStatementKeyword, "SpaceBeforeParens",
      retval.SpaceBeforeParens,
      space_before_parens_options.at(old.SpaceAfterControlStatementKeyword),
      "3.5");
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  newField("CommentPragmas", "3.5", retval.CommentPragmas);
  newField("DisableFormat", "3.5", retval.DisableFormat);
  newField("ForEachMacros", "3.5", retval.ForEachMacros);

  return retval;
}

} // namespace clang_update_v3_5

namespace clang_update_v3_6 {

constexpr frozen::unordered_map<clang_v3_5::FormatStyle::LanguageKind,
                                clang_v3_6::FormatStyle::LanguageKind, 4>
    language_king{{clang_v3_5::FormatStyle::LanguageKind::LK_None,
                   clang_v3_6::FormatStyle::LanguageKind::LK_None},
                  {clang_v3_5::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v3_6::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v3_5::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v3_6::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v3_5::FormatStyle::LanguageKind::LK_Proto,
                   clang_v3_6::FormatStyle::LanguageKind::LK_Proto}};

constexpr frozen::unordered_map<clang_v3_5::FormatStyle::PointerAlignmentStyle,
                                clang_v3_6::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v3_5::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v3_6::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v3_5::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v3_6::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v3_5::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v3_6::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

constexpr frozen::unordered_map<clang_v3_5::FormatStyle::LanguageStandard,
                                clang_v3_6::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v3_5::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v3_6::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v3_5::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v3_6::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v3_5::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v3_6::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<
    clang_v3_5::FormatStyle::NamespaceIndentationKind,
    clang_v3_6::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v3_5::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v3_6::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v3_5::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v3_6::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v3_5::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v3_6::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v3_5::FormatStyle::ShortFunctionStyle,
                                clang_v3_6::FormatStyle::ShortFunctionStyle, 3>
    short_function_style{
        {clang_v3_5::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v3_6::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v3_5::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v3_6::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v3_5::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v3_6::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v3_5::FormatStyle::UseTabStyle,
                                clang_v3_6::FormatStyle::UseTabStyle, 3>
    use_tab_style{{clang_v3_5::FormatStyle::UseTabStyle::UT_Never,
                   clang_v3_6::FormatStyle::UseTabStyle::UT_Never},
                  {clang_v3_5::FormatStyle::UseTabStyle::UT_ForIndentation,
                   clang_v3_6::FormatStyle::UseTabStyle::UT_ForIndentation},
                  {clang_v3_5::FormatStyle::UseTabStyle::UT_Always,
                   clang_v3_6::FormatStyle::UseTabStyle::UT_Always}};

constexpr frozen::unordered_map<bool,
                                clang_v3_6::FormatStyle::BinaryOperatorStyle, 2>
    binary_operator_style{
        {false, clang_v3_6::FormatStyle::BinaryOperatorStyle::BOS_None},
        {true, clang_v3_6::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v3_5::FormatStyle::BraceBreakingStyle,
                                clang_v3_6::FormatStyle::BraceBreakingStyle, 5>
    brace_breaking_style{
        {clang_v3_5::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v3_6::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v3_5::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v3_6::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v3_5::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v3_6::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v3_5::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v3_6::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v3_5::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v3_6::FormatStyle::BraceBreakingStyle::BS_GNU}};

constexpr frozen::unordered_map<
    clang_v3_5::FormatStyle::SpaceBeforeParensOptions,
    clang_v3_6::FormatStyle::SpaceBeforeParensOptions, 3>
    space_before_parens_options{
        {clang_v3_5::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v3_6::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v3_5::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v3_6::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v3_5::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v3_6::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

clang_v3_6::FormatStyle update(clang_v3_5::FormatStyle &old,
                               const std::string &style) {
  clang_v3_6::FormatStyle retval;
  if (!clang_v3_6::getPredefinedStyle(
          style, clang_v3_6::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  improveField("Language", "Java", "3.6");
  retval.Language = language_king.at(old.Language);
  retval.ColumnLimit = old.ColumnLimit;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.Standard = language_standard.at(old.Standard);
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.BinPackParameters = old.BinPackParameters;
  newField("BinPackArguments", "3.6", retval.BinPackArguments);
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  retval.BreakConstructorInitializersBeforeComma =
      old.BreakConstructorInitializersBeforeComma;
  retval.AllowShortBlocksOnASingleLine = old.AllowShortBlocksOnASingleLine;
  retval.AllowShortIfStatementsOnASingleLine =
      old.AllowShortIfStatementsOnASingleLine;
  retval.AllowShortLoopsOnASingleLine = old.AllowShortLoopsOnASingleLine;
  newField("AllowShortCaseLabelsOnASingleLine", "3.6",
           retval.AllowShortCaseLabelsOnASingleLine);
  improveField("AllowShortFunctionsOnASingleLine", "Empty", "3.6");
  retval.AllowShortFunctionsOnASingleLine =
      short_function_style.at(old.AllowShortFunctionsOnASingleLine);
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  newField("AlignAfterOpenBracket", "3.6", retval.AlignAfterOpenBracket);
  newField("AlignOperands", "3.6", retval.AlignOperands);
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AlignEscapedNewlinesLeft = old.AlignEscapedNewlinesLeft;
  retval.IndentWidth = old.IndentWidth;
  retval.TabWidth = old.TabWidth;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  newField("ObjCBlockIndentWidth", "3.6", retval.ObjCBlockIndentWidth);
  newField("AlwaysBreakAfterDefinitionReturnType", "3.6",
           retval.AlwaysBreakAfterDefinitionReturnType);
  retval.AlwaysBreakTemplateDeclarations = old.AlwaysBreakTemplateDeclarations;
  retval.AlwaysBreakBeforeMultilineStrings =
      old.AlwaysBreakBeforeMultilineStrings;
  retval.UseTab = use_tab_style.at(old.UseTab);
  improveField("BreakBeforeBinaryOperators", "NonAssignment", "3.6");
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInAngles = old.SpacesInAngles;
  newField("SpacesInSquareBrackets", "3.6", retval.SpacesInSquareBrackets);
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  newField("SpaceAfterCStyleCast", "3.6", retval.SpaceAfterCStyleCast);
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.CommentPragmas = old.CommentPragmas;
  retval.DisableFormat = old.DisableFormat;
  retval.ForEachMacros = old.ForEachMacros;

  return retval;
}

} // namespace clang_update_v3_6

namespace clang_update_v3_7 {
constexpr frozen::unordered_map<clang_v3_6::FormatStyle::LanguageKind,
                                clang_v3_7::FormatStyle::LanguageKind, 5>
    language_king{{clang_v3_6::FormatStyle::LanguageKind::LK_None,
                   clang_v3_7::FormatStyle::LanguageKind::LK_None},
                  {clang_v3_6::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v3_7::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v3_6::FormatStyle::LanguageKind::LK_Java,
                   clang_v3_7::FormatStyle::LanguageKind::LK_Java},
                  {clang_v3_6::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v3_7::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v3_6::FormatStyle::LanguageKind::LK_Proto,
                   clang_v3_7::FormatStyle::LanguageKind::LK_Proto}};

constexpr frozen::unordered_map<clang_v3_6::FormatStyle::PointerAlignmentStyle,
                                clang_v3_7::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v3_6::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v3_7::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v3_6::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v3_7::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v3_6::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v3_7::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

constexpr frozen::unordered_map<clang_v3_6::FormatStyle::LanguageStandard,
                                clang_v3_7::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v3_6::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v3_7::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v3_6::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v3_7::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v3_6::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v3_7::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<
    clang_v3_6::FormatStyle::NamespaceIndentationKind,
    clang_v3_7::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v3_6::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v3_7::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v3_6::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v3_7::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v3_6::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v3_7::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v3_6::FormatStyle::ShortFunctionStyle,
                                clang_v3_7::FormatStyle::ShortFunctionStyle, 3>
    short_function_style{
        {clang_v3_6::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v3_7::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v3_6::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v3_7::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v3_6::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v3_7::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<
    bool, clang_v3_7::FormatStyle::DefinitionReturnTypeBreakingStyle, 2>
    definition_return_type_breaking_style{
        {false, clang_v3_7::FormatStyle::DefinitionReturnTypeBreakingStyle::
                    DRTBS_None},
        {true, clang_v3_7::FormatStyle::DefinitionReturnTypeBreakingStyle::
                   DRTBS_All}};

constexpr frozen::unordered_map<clang_v3_6::FormatStyle::UseTabStyle,
                                clang_v3_7::FormatStyle::UseTabStyle, 3>
    use_tab_style{{clang_v3_6::FormatStyle::UseTabStyle::UT_Never,
                   clang_v3_7::FormatStyle::UseTabStyle::UT_Never},
                  {clang_v3_6::FormatStyle::UseTabStyle::UT_ForIndentation,
                   clang_v3_7::FormatStyle::UseTabStyle::UT_ForIndentation},
                  {clang_v3_6::FormatStyle::UseTabStyle::UT_Always,
                   clang_v3_7::FormatStyle::UseTabStyle::UT_Always}};

constexpr frozen::unordered_map<clang_v3_6::FormatStyle::BinaryOperatorStyle,
                                clang_v3_7::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v3_6::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v3_7::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v3_6::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v3_7::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v3_6::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v3_7::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v3_6::FormatStyle::BraceBreakingStyle,
                                clang_v3_7::FormatStyle::BraceBreakingStyle, 5>
    brace_breaking_style{
        {clang_v3_6::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v3_6::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v3_6::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v3_6::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v3_6::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v3_7::FormatStyle::BraceBreakingStyle::BS_GNU}};

constexpr frozen::unordered_map<
    clang_v3_6::FormatStyle::SpaceBeforeParensOptions,
    clang_v3_7::FormatStyle::SpaceBeforeParensOptions, 3>
    space_before_parens_options{
        {clang_v3_6::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v3_7::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v3_6::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v3_7::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v3_6::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v3_7::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

clang_v3_7::FormatStyle update(clang_v3_6::FormatStyle &old,
                               const std::string &style) {
  clang_v3_7::FormatStyle retval;
  if (!clang_v3_7::getPredefinedStyle(
          style, clang_v3_7::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.Language = language_king.at(old.Language);
  retval.ColumnLimit = old.ColumnLimit;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.Standard = language_standard.at(old.Standard);
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.BinPackArguments = old.BinPackArguments;
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  retval.BreakConstructorInitializersBeforeComma =
      old.BreakConstructorInitializersBeforeComma;
  retval.AllowShortBlocksOnASingleLine = old.AllowShortBlocksOnASingleLine;
  retval.AllowShortIfStatementsOnASingleLine =
      old.AllowShortIfStatementsOnASingleLine;
  retval.AllowShortLoopsOnASingleLine = old.AllowShortLoopsOnASingleLine;
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  retval.AllowShortFunctionsOnASingleLine =
      short_function_style.at(old.AllowShortFunctionsOnASingleLine);
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.AlignAfterOpenBracket = old.AlignAfterOpenBracket;
  retval.AlignOperands = old.AlignOperands;
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AlignEscapedNewlinesLeft = old.AlignEscapedNewlinesLeft;
  retval.IndentWidth = old.IndentWidth;
  retval.TabWidth = old.TabWidth;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.AlwaysBreakAfterDefinitionReturnType =
      definition_return_type_breaking_style.at(
          old.AlwaysBreakAfterDefinitionReturnType);
  improveField("AlwaysBreakAfterDefinitionReturnType", "TopLevel", "3.7");
  retval.AlwaysBreakTemplateDeclarations = old.AlwaysBreakTemplateDeclarations;
  retval.AlwaysBreakBeforeMultilineStrings =
      old.AlwaysBreakBeforeMultilineStrings;
  retval.UseTab = use_tab_style.at(old.UseTab);
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  improveField("BreakBeforeBraces", "Mozilla", "3.7");
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInAngles = old.SpacesInAngles;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.CommentPragmas = old.CommentPragmas;
  retval.DisableFormat = old.DisableFormat;
  retval.ForEachMacros = old.ForEachMacros;

  newField("AlignConsecutiveAssignments", "3.7",
           retval.AlignConsecutiveAssignments);
  newField("MacroBlockBegin", "3.7", retval.MacroBlockBegin);
  newField("MacroBlockEnd", "3.7", retval.MacroBlockEnd);

  return retval;
}

} // namespace clang_update_v3_7

namespace clang_update_v3_8 {

constexpr frozen::unordered_map<
    bool, clang_v3_8::FormatStyle::BracketAlignmentStyle, 2>
    bracket_all_alignment_style{
        {false, clang_v3_8::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {true, clang_v3_8::FormatStyle::BracketAlignmentStyle::BAS_Align}};

constexpr frozen::unordered_map<clang_v3_7::FormatStyle::ShortFunctionStyle,
                                clang_v3_8::FormatStyle::ShortFunctionStyle, 4>
    short_function_style{
        {clang_v3_7::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v3_8::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v3_7::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v3_8::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v3_7::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v3_8::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v3_7::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v3_8::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<
    clang_v3_7::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v3_8::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v3_7::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v3_8::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_None},
        {clang_v3_7::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v3_8::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v3_7::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v3_8::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v3_7::FormatStyle::BinaryOperatorStyle,
                                clang_v3_8::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v3_7::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v3_8::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v3_7::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v3_8::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v3_7::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v3_8::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v3_7::FormatStyle::BraceBreakingStyle,
                                clang_v3_8::FormatStyle::BraceBreakingStyle, 6>
    brace_breaking_style{
        {clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v3_7::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v3_8::FormatStyle::BraceBreakingStyle::BS_GNU}};

constexpr frozen::unordered_map<clang_v3_7::FormatStyle::LanguageKind,
                                clang_v3_8::FormatStyle::LanguageKind, 5>
    language_king{{clang_v3_7::FormatStyle::LanguageKind::LK_None,
                   clang_v3_8::FormatStyle::LanguageKind::LK_None},
                  {clang_v3_7::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v3_8::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v3_7::FormatStyle::LanguageKind::LK_Java,
                   clang_v3_8::FormatStyle::LanguageKind::LK_Java},
                  {clang_v3_7::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v3_8::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v3_7::FormatStyle::LanguageKind::LK_Proto,
                   clang_v3_8::FormatStyle::LanguageKind::LK_Proto}};

constexpr frozen::unordered_map<
    clang_v3_7::FormatStyle::NamespaceIndentationKind,
    clang_v3_8::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v3_7::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v3_8::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v3_7::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v3_8::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v3_7::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v3_8::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v3_7::FormatStyle::PointerAlignmentStyle,
                                clang_v3_8::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v3_7::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v3_8::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v3_7::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v3_8::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v3_7::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v3_8::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

constexpr frozen::unordered_map<
    clang_v3_7::FormatStyle::SpaceBeforeParensOptions,
    clang_v3_8::FormatStyle::SpaceBeforeParensOptions, 3>
    space_before_parens_options{
        {clang_v3_7::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v3_8::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v3_7::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v3_8::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v3_7::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v3_8::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v3_7::FormatStyle::LanguageStandard,
                                clang_v3_8::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v3_7::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v3_8::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v3_7::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v3_8::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v3_7::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v3_8::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v3_7::FormatStyle::UseTabStyle,
                                clang_v3_8::FormatStyle::UseTabStyle, 3>
    use_tab_style{{clang_v3_7::FormatStyle::UseTabStyle::UT_Never,
                   clang_v3_8::FormatStyle::UseTabStyle::UT_Never},
                  {clang_v3_7::FormatStyle::UseTabStyle::UT_ForIndentation,
                   clang_v3_8::FormatStyle::UseTabStyle::UT_ForIndentation},
                  {clang_v3_7::FormatStyle::UseTabStyle::UT_Always,
                   clang_v3_8::FormatStyle::UseTabStyle::UT_Always}};

clang_v3_8::FormatStyle update(clang_v3_7::FormatStyle &old,
                               const std::string &style) {
  clang_v3_8::FormatStyle retval;
  if (!clang_v3_8::getPredefinedStyle(
          style, clang_v3_8::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  improveField("AlignAfterOpenBracket", "AlwaysBreak", "3.8");

  retval.AlignConsecutiveAssignments = old.AlignConsecutiveAssignments;
  retval.AlignEscapedNewlinesLeft = old.AlignEscapedNewlinesLeft;
  retval.AlignOperands = old.AlignOperands;
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortBlocksOnASingleLine = old.AllowShortBlocksOnASingleLine;
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  retval.AllowShortFunctionsOnASingleLine =
      short_function_style.at(old.AllowShortFunctionsOnASingleLine);
  retval.AllowShortIfStatementsOnASingleLine =
      old.AllowShortIfStatementsOnASingleLine;
  retval.AllowShortLoopsOnASingleLine = old.AllowShortLoopsOnASingleLine;
  retval.AlwaysBreakAfterDefinitionReturnType =
      definition_return_type_breaking_style.at(
          old.AlwaysBreakAfterDefinitionReturnType);
  retval.AlwaysBreakBeforeMultilineStrings =
      old.AlwaysBreakBeforeMultilineStrings;
  retval.AlwaysBreakTemplateDeclarations = old.AlwaysBreakTemplateDeclarations;
  retval.BinPackArguments = old.BinPackArguments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  improveField("BreakBeforeBraces", "WebKit", "3.8");
  improveField("BreakBeforeBraces", "Custom", "3.8");
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializersBeforeComma =
      old.BreakConstructorInitializersBeforeComma;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.ForEachMacros = old.ForEachMacros;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.Language = language_king.at(old.Language);
  improveField("Language", "TableGen", "3.8");
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = old.SpacesInAngles;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.Standard = language_standard.at(old.Standard);
  retval.TabWidth = old.TabWidth;
  retval.UseTab = use_tab_style.at(old.UseTab);

  newField("AlignConsecutiveDeclarations", "3.8",
           retval.AlignConsecutiveDeclarations);
  newField("AlwaysBreakAfterReturnType", "3.8",
           retval.AlwaysBreakAfterReturnType);
  newField("BraceWrapping", "3.8", retval.BraceWrapping);
  newField("BreakAfterJavaFieldAnnotations", "3.8",
           retval.BreakAfterJavaFieldAnnotations);
  newField("IncludeCategories", "3.8", retval.IncludeCategories);
  newField("ReflowComments", "3.8", retval.ReflowComments);
  newField("SortIncludes", "3.8", retval.SortIncludes);

  return retval;
}

} // namespace clang_update_v3_8

namespace clang_update_v3_9 {
constexpr frozen::unordered_map<clang_v3_8::FormatStyle::BracketAlignmentStyle,
                                clang_v3_9::FormatStyle::BracketAlignmentStyle,
                                3>
    bracket_all_alignment_style{
        {clang_v3_8::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v3_9::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v3_8::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v3_9::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v3_8::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v3_9::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<clang_v3_8::FormatStyle::ShortFunctionStyle,
                                clang_v3_9::FormatStyle::ShortFunctionStyle, 4>
    short_function_style{
        {clang_v3_8::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v3_9::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v3_8::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v3_9::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v3_8::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v3_9::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v3_8::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v3_9::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<
    clang_v3_8::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v3_9::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v3_8::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v3_9::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_None},
        {clang_v3_8::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v3_9::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v3_8::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v3_9::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<
    clang_v3_8::FormatStyle::ReturnTypeBreakingStyle,
    clang_v3_9::FormatStyle::ReturnTypeBreakingStyle, 5>
    return_type_breaking_style{
        {clang_v3_8::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v3_9::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v3_8::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v3_9::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v3_8::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v3_9::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v3_8::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v3_9::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v3_8::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v3_9::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<clang_v3_8::FormatStyle::BinaryOperatorStyle,
                                clang_v3_9::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v3_8::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v3_9::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v3_8::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v3_9::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v3_8::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v3_9::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v3_8::FormatStyle::BraceBreakingStyle,
                                clang_v3_9::FormatStyle::BraceBreakingStyle, 8>
    brace_breaking_style{
        {clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v3_8::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v3_9::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v3_8::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v3_9::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Custom}};

void assign(std::vector<clang_v3_8::FormatStyle::IncludeCategory> &lhs,
            std::vector<clang_v3_9::FormatStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(
        clang_v3_9::FormatStyle::IncludeCategory{item.Regex, item.Priority});
  }
}

constexpr frozen::unordered_map<clang_v3_8::FormatStyle::LanguageKind,
                                clang_v3_9::FormatStyle::LanguageKind, 6>
    language_king{{clang_v3_8::FormatStyle::LanguageKind::LK_None,
                   clang_v3_9::FormatStyle::LanguageKind::LK_None},
                  {clang_v3_8::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v3_9::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v3_8::FormatStyle::LanguageKind::LK_Java,
                   clang_v3_9::FormatStyle::LanguageKind::LK_Java},
                  {clang_v3_8::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v3_9::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v3_8::FormatStyle::LanguageKind::LK_Proto,
                   clang_v3_9::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v3_8::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v3_9::FormatStyle::LanguageKind::LK_TableGen}};

constexpr frozen::unordered_map<
    clang_v3_8::FormatStyle::NamespaceIndentationKind,
    clang_v3_9::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v3_8::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v3_9::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v3_8::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v3_9::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v3_8::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v3_9::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v3_8::FormatStyle::PointerAlignmentStyle,
                                clang_v3_9::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v3_8::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v3_9::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v3_8::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v3_9::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v3_8::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v3_9::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

constexpr frozen::unordered_map<
    clang_v3_8::FormatStyle::SpaceBeforeParensOptions,
    clang_v3_9::FormatStyle::SpaceBeforeParensOptions, 3>
    space_before_parens_options{
        {clang_v3_8::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v3_9::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v3_8::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v3_9::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v3_8::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v3_9::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v3_8::FormatStyle::LanguageStandard,
                                clang_v3_9::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v3_8::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v3_9::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v3_8::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v3_9::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v3_8::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v3_9::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v3_8::FormatStyle::UseTabStyle,
                                clang_v3_9::FormatStyle::UseTabStyle, 3>
    use_tab_style{{clang_v3_8::FormatStyle::UseTabStyle::UT_Never,
                   clang_v3_9::FormatStyle::UseTabStyle::UT_Never},
                  {clang_v3_8::FormatStyle::UseTabStyle::UT_ForIndentation,
                   clang_v3_9::FormatStyle::UseTabStyle::UT_ForIndentation},
                  {clang_v3_8::FormatStyle::UseTabStyle::UT_Always,
                   clang_v3_9::FormatStyle::UseTabStyle::UT_Always}};

clang_v3_9::FormatStyle update(clang_v3_8::FormatStyle &old,
                               const std::string &style) {
  clang_v3_9::FormatStyle retval;
  if (!clang_v3_9::getPredefinedStyle(
          style, clang_v3_9::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);

  retval.AlignConsecutiveAssignments = old.AlignConsecutiveAssignments;
  retval.AlignConsecutiveDeclarations = old.AlignConsecutiveDeclarations;
  retval.AlignEscapedNewlinesLeft = old.AlignEscapedNewlinesLeft;
  retval.AlignOperands = old.AlignOperands;
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortBlocksOnASingleLine = old.AllowShortBlocksOnASingleLine;
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  retval.AllowShortFunctionsOnASingleLine =
      short_function_style.at(old.AllowShortFunctionsOnASingleLine);
  retval.AllowShortIfStatementsOnASingleLine =
      old.AllowShortIfStatementsOnASingleLine;
  retval.AllowShortLoopsOnASingleLine = old.AllowShortLoopsOnASingleLine;
  retval.AlwaysBreakAfterDefinitionReturnType =
      definition_return_type_breaking_style.at(
          old.AlwaysBreakAfterDefinitionReturnType);
  retval.AlwaysBreakAfterReturnType =
      return_type_breaking_style.at(old.AlwaysBreakAfterReturnType);
  retval.AlwaysBreakBeforeMultilineStrings =
      old.AlwaysBreakBeforeMultilineStrings;
  retval.AlwaysBreakTemplateDeclarations = old.AlwaysBreakTemplateDeclarations;
  retval.BinPackArguments = old.BinPackArguments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  retval.BraceWrapping.AfterClass = old.BraceWrapping.AfterClass;
  retval.BraceWrapping.AfterControlStatement =
      old.BraceWrapping.AfterControlStatement;
  retval.BraceWrapping.AfterEnum = old.BraceWrapping.AfterEnum;
  retval.BraceWrapping.AfterFunction = old.BraceWrapping.AfterFunction;
  retval.BraceWrapping.AfterNamespace = old.BraceWrapping.AfterNamespace;
  retval.BraceWrapping.AfterObjCDeclaration =
      old.BraceWrapping.AfterObjCDeclaration;
  retval.BraceWrapping.AfterStruct = old.BraceWrapping.AfterStruct;
  retval.BraceWrapping.AfterUnion = old.BraceWrapping.AfterUnion;
  retval.BraceWrapping.BeforeCatch = old.BraceWrapping.BeforeCatch;
  retval.BraceWrapping.BeforeElse = old.BraceWrapping.BeforeElse;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializersBeforeComma =
      old.BreakConstructorInitializersBeforeComma;
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.ForEachMacros = old.ForEachMacros;
  assign(old.IncludeCategories, retval.IncludeCategories);
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.Language = language_king.at(old.Language);
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.ReflowComments = old.ReflowComments;
  retval.SortIncludes = old.SortIncludes;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = old.SpacesInAngles;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.Standard = language_standard.at(old.Standard);
  retval.TabWidth = old.TabWidth;
  retval.UseTab = use_tab_style.at(old.UseTab);

  newField("BreakStringLiterals", "3.9", retval.BreakStringLiterals);
  newField("IncludeIsMainRegex", "3.9", retval.IncludeIsMainRegex);
  newField("JavaScriptQuotes", "3.9", retval.JavaScriptQuotes);
  newField("JavaScriptWrapImports", "3.9", retval.JavaScriptWrapImports);
  improveField("UseTab", "ForContinuationAndIndentation", "3.9");
  return retval;
}

} // namespace clang_update_v3_9

namespace clang_update_v4 {

constexpr frozen::unordered_map<clang_v3_9::FormatStyle::BracketAlignmentStyle,
                                clang_v4::FormatStyle::BracketAlignmentStyle, 3>
    bracket_all_alignment_style{
        {clang_v3_9::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v4::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v3_9::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v4::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v3_9::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v4::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<clang_v3_9::FormatStyle::ShortFunctionStyle,
                                clang_v4::FormatStyle::ShortFunctionStyle, 4>
    short_function_style{
        {clang_v3_9::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v4::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v3_9::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v4::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v3_9::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v4::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v3_9::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v4::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<
    clang_v3_9::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v4::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v3_9::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v4::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v3_9::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v4::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v3_9::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v4::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<
    clang_v3_9::FormatStyle::ReturnTypeBreakingStyle,
    clang_v4::FormatStyle::ReturnTypeBreakingStyle, 5>
    return_type_breaking_style{
        {clang_v3_9::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v4::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v3_9::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v4::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v3_9::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v4::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v3_9::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v4::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v3_9::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v4::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<clang_v3_9::FormatStyle::BinaryOperatorStyle,
                                clang_v4::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v3_9::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v4::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v3_9::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v4::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v3_9::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v4::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v3_9::FormatStyle::BraceBreakingStyle,
                                clang_v4::FormatStyle::BraceBreakingStyle, 8>
    brace_breaking_style{
        {clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v4::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v4::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v4::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v4::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v4::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v3_9::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v4::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v3_9::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v4::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v3_9::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v4::FormatStyle::BraceBreakingStyle::BS_Custom}};

void assign(std::vector<clang_v3_9::FormatStyle::IncludeCategory> &lhs,
            std::vector<clang_v4::FormatStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(
        clang_v4::FormatStyle::IncludeCategory{item.Regex, item.Priority});
  }
}

constexpr frozen::unordered_map<clang_v3_9::FormatStyle::JavaScriptQuoteStyle,
                                clang_v4::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v3_9::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v4::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v3_9::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v4::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v3_9::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v4::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v3_9::FormatStyle::LanguageKind,
                                clang_v4::FormatStyle::LanguageKind, 6>
    language_king{{clang_v3_9::FormatStyle::LanguageKind::LK_None,
                   clang_v4::FormatStyle::LanguageKind::LK_None},
                  {clang_v3_9::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v4::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v3_9::FormatStyle::LanguageKind::LK_Java,
                   clang_v4::FormatStyle::LanguageKind::LK_Java},
                  {clang_v3_9::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v4::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v3_9::FormatStyle::LanguageKind::LK_Proto,
                   clang_v4::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v3_9::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v4::FormatStyle::LanguageKind::LK_TableGen}};

constexpr frozen::unordered_map<
    clang_v3_9::FormatStyle::NamespaceIndentationKind,
    clang_v4::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v3_9::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v4::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v3_9::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v4::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v3_9::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v4::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v3_9::FormatStyle::PointerAlignmentStyle,
                                clang_v4::FormatStyle::PointerAlignmentStyle, 3>
    pointer_alignment_style{
        {clang_v3_9::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v4::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v3_9::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v4::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v3_9::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v4::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

constexpr frozen::unordered_map<
    clang_v3_9::FormatStyle::SpaceBeforeParensOptions,
    clang_v4::FormatStyle::SpaceBeforeParensOptions, 3>
    space_before_parens_options{
        {clang_v3_9::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v4::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v3_9::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v4::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v3_9::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v4::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v3_9::FormatStyle::LanguageStandard,
                                clang_v4::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v3_9::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v4::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v3_9::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v4::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v3_9::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v4::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v3_9::FormatStyle::UseTabStyle,
                                clang_v4::FormatStyle::UseTabStyle, 4>
    use_tab_style{
        {clang_v3_9::FormatStyle::UseTabStyle::UT_Never,
         clang_v4::FormatStyle::UseTabStyle::UT_Never},
        {clang_v3_9::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v4::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v3_9::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v4::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v3_9::FormatStyle::UseTabStyle::UT_Always,
         clang_v4::FormatStyle::UseTabStyle::UT_Always}};

clang_v4::FormatStyle update(clang_v3_9::FormatStyle &old,
                             const std::string &style) {
  clang_v4::FormatStyle retval;
  if (!clang_v4::getPredefinedStyle(
          style, clang_v4::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignConsecutiveAssignments = old.AlignConsecutiveAssignments;
  retval.AlignConsecutiveDeclarations = old.AlignConsecutiveDeclarations;
  retval.AlignEscapedNewlinesLeft = old.AlignEscapedNewlinesLeft;
  retval.AlignOperands = old.AlignOperands;
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortBlocksOnASingleLine = old.AllowShortBlocksOnASingleLine;
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  retval.AllowShortFunctionsOnASingleLine =
      short_function_style.at(old.AllowShortFunctionsOnASingleLine);
  retval.AllowShortIfStatementsOnASingleLine =
      old.AllowShortIfStatementsOnASingleLine;
  retval.AllowShortLoopsOnASingleLine = old.AllowShortLoopsOnASingleLine;
  retval.AlwaysBreakAfterDefinitionReturnType =
      definition_return_type_breaking_style.at(
          old.AlwaysBreakAfterDefinitionReturnType);
  retval.AlwaysBreakAfterReturnType =
      return_type_breaking_style.at(old.AlwaysBreakAfterReturnType);
  retval.AlwaysBreakBeforeMultilineStrings =
      old.AlwaysBreakBeforeMultilineStrings;
  retval.AlwaysBreakTemplateDeclarations = old.AlwaysBreakTemplateDeclarations;
  retval.BinPackArguments = old.BinPackArguments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  retval.BraceWrapping.AfterClass = old.BraceWrapping.AfterClass;
  retval.BraceWrapping.AfterControlStatement =
      old.BraceWrapping.AfterControlStatement;
  retval.BraceWrapping.AfterEnum = old.BraceWrapping.AfterEnum;
  retval.BraceWrapping.AfterFunction = old.BraceWrapping.AfterFunction;
  retval.BraceWrapping.AfterNamespace = old.BraceWrapping.AfterNamespace;
  retval.BraceWrapping.AfterObjCDeclaration =
      old.BraceWrapping.AfterObjCDeclaration;
  retval.BraceWrapping.AfterStruct = old.BraceWrapping.AfterStruct;
  retval.BraceWrapping.AfterUnion = old.BraceWrapping.AfterUnion;
  retval.BraceWrapping.BeforeCatch = old.BraceWrapping.BeforeCatch;
  retval.BraceWrapping.BeforeElse = old.BraceWrapping.BeforeElse;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializersBeforeComma =
      old.BreakConstructorInitializersBeforeComma;
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.ForEachMacros = old.ForEachMacros;
  assign(old.IncludeCategories, retval.IncludeCategories);
  retval.IncludeIsMainRegex = old.IncludeIsMainRegex;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.JavaScriptQuotes = java_script_quote_style.at(old.JavaScriptQuotes);
  retval.JavaScriptWrapImports = old.JavaScriptWrapImports;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.Language = language_king.at(old.Language);
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.ReflowComments = old.ReflowComments;
  retval.SortIncludes = old.SortIncludes;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = old.SpacesInAngles;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.Standard = language_standard.at(old.Standard);
  retval.TabWidth = old.TabWidth;
  retval.UseTab = use_tab_style.at(old.UseTab);

  newField("BreakStringLiterals", "3.9", retval.BreakStringLiterals);
  newField("IncludeIsMainRegex", "3.9", retval.IncludeIsMainRegex);
  newField("JavaScriptQuotes", "3.9", retval.JavaScriptQuotes);
  newField("JavaScriptWrapImports", "3.9", retval.JavaScriptWrapImports);
  improveField("UseTab", "ForContinuationAndIndentation", "3.9");

  improveField("Language", "ObjC", "4");
  newField("SpaceAfterTemplateKeyword", "4", retval.SpaceAfterTemplateKeyword);

  return retval;
}

} // namespace clang_update_v4
