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

std::ostream &
operator<<(std::ostream &os,
           const std::vector<clang_v6::FormatStyle::RawStringFormat> &formats) {
  os << "{";
  for (const auto &format : formats) {
    std::cout << "Delimiter: " << format.Delimiter
              << ", Language: " << static_cast<int>(format.Language)
              << ", BasedOnStyle: " << format.BasedOnStyle << std::endl;
  }
  os << "}";
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

namespace clang_update_v5 {

constexpr frozen::unordered_map<clang_v4::FormatStyle::BracketAlignmentStyle,
                                clang_v5::FormatStyle::BracketAlignmentStyle, 3>
    bracket_all_alignment_style{
        {clang_v4::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v5::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v4::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v5::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v4::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v5::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<
    bool, clang_v5::FormatStyle::EscapedNewlineAlignmentStyle, 2>
    escaped_new_line_alignment_style{
        {false,
         clang_v5::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right},
        {true, clang_v5::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left}};

constexpr frozen::unordered_map<clang_v4::FormatStyle::ShortFunctionStyle,
                                clang_v5::FormatStyle::ShortFunctionStyle, 4>
    short_function_style{
        {clang_v4::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v5::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v4::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v5::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v4::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v5::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v4::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v5::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<
    clang_v4::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v5::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v4::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v5::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v4::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v5::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v4::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v5::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v4::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v5::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v4::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v5::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v4::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v5::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v4::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v5::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v4::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v5::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v4::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v5::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<clang_v4::FormatStyle::BinaryOperatorStyle,
                                clang_v5::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v4::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v5::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v4::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v5::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v4::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v5::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v4::FormatStyle::BraceBreakingStyle,
                                clang_v5::FormatStyle::BraceBreakingStyle, 8>
    brace_breaking_style{
        {clang_v4::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v5::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v4::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v5::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v4::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v5::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v4::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v5::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v4::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v5::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v4::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v5::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v4::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v5::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v4::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v5::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    bool, clang_v5::FormatStyle::BreakConstructorInitializersStyle, 2>
    break_constructor_initializers_style{
        {false, clang_v5::FormatStyle::BreakConstructorInitializersStyle::
                    BCIS_AfterColon},
        {true, clang_v5::FormatStyle::BreakConstructorInitializersStyle::
                   BCIS_BeforeComma}};

void assign(std::vector<clang_v4::FormatStyle::IncludeCategory> &lhs,
            std::vector<clang_v5::FormatStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(
        clang_v5::FormatStyle::IncludeCategory{item.Regex, item.Priority});
  }
}

constexpr frozen::unordered_map<clang_v4::FormatStyle::JavaScriptQuoteStyle,
                                clang_v5::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v4::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v5::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v4::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v5::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v4::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v5::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v4::FormatStyle::LanguageKind,
                                clang_v5::FormatStyle::LanguageKind, 7>
    language_king{{clang_v4::FormatStyle::LanguageKind::LK_None,
                   clang_v5::FormatStyle::LanguageKind::LK_None},
                  {clang_v4::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v5::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v4::FormatStyle::LanguageKind::LK_Java,
                   clang_v5::FormatStyle::LanguageKind::LK_Java},
                  {clang_v4::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v5::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v4::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v5::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v4::FormatStyle::LanguageKind::LK_Proto,
                   clang_v5::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v4::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v5::FormatStyle::LanguageKind::LK_TableGen}};

constexpr frozen::unordered_map<clang_v4::FormatStyle::NamespaceIndentationKind,
                                clang_v5::FormatStyle::NamespaceIndentationKind,
                                3>
    namespace_indentation_kind{
        {clang_v4::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v5::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v4::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v5::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v4::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v5::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v4::FormatStyle::PointerAlignmentStyle,
                                clang_v5::FormatStyle::PointerAlignmentStyle, 3>
    pointer_alignment_style{
        {clang_v4::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v5::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v4::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v5::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v4::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v5::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

constexpr frozen::unordered_map<clang_v4::FormatStyle::SpaceBeforeParensOptions,
                                clang_v5::FormatStyle::SpaceBeforeParensOptions,
                                3>
    space_before_parens_options{
        {clang_v4::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v5::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v4::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v5::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v4::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v5::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v4::FormatStyle::LanguageStandard,
                                clang_v5::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v4::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v5::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v4::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v5::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v4::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v5::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v4::FormatStyle::UseTabStyle,
                                clang_v5::FormatStyle::UseTabStyle, 4>
    use_tab_style{
        {clang_v4::FormatStyle::UseTabStyle::UT_Never,
         clang_v5::FormatStyle::UseTabStyle::UT_Never},
        {clang_v4::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v5::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v4::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v5::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v4::FormatStyle::UseTabStyle::UT_Always,
         clang_v5::FormatStyle::UseTabStyle::UT_Always}};

clang_v5::FormatStyle update(clang_v4::FormatStyle &old,
                             const std::string &style) {
  clang_v5::FormatStyle retval;
  if (!clang_v5::getPredefinedStyle(
          style, clang_v5::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignConsecutiveAssignments = old.AlignConsecutiveAssignments;
  retval.AlignConsecutiveDeclarations = old.AlignConsecutiveDeclarations;
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlinesLeft);
  improveField("AlignEscapedNewlines", "DontAlign", "5");
  retval.AlignOperands = old.AlignOperands;
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortBlocksOnASingleLine = old.AllowShortBlocksOnASingleLine;
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  retval.AllowShortFunctionsOnASingleLine =
      short_function_style.at(old.AllowShortFunctionsOnASingleLine);
  improveField("AllowShortFunctionsOnASingleLine", "InlineOnly", "5");
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
  newField("BraceWrapping.SplitEmptyFunction", "5",
           retval.BraceWrapping.SplitEmptyFunction);
  newField("BraceWrapping.SplitEmptyRecord", "5",
           retval.BraceWrapping.SplitEmptyRecord);
  newField("BraceWrapping.SplitEmptyNamespace", "5",
           retval.BraceWrapping.SplitEmptyNamespace);
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  assignWithWarning("BreakConstructorInitializersBeforeComma",
                    old.BreakConstructorInitializersBeforeComma,
                    "BreakConstructorInitializers",
                    retval.BreakConstructorInitializers,
                    break_constructor_initializers_style.at(
                        old.BreakConstructorInitializersBeforeComma),
                    "5");
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  newField("BreakBeforeInheritanceComma", "5",
           retval.BreakBeforeInheritanceComma);
  newField("CompactNamespaces", "5", retval.CompactNamespaces);
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
  newField("FixNamespaceComments", "5", retval.FixNamespaceComments);
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
  improveField("Language", "TextProto", "5");
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  newField("PenaltyBreakAssignment", "5", retval.PenaltyBreakAssignment);
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
  newField("SortUsingDeclarations", "5", retval.SortUsingDeclarations);
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
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

  return retval;
}

} // namespace clang_update_v5

namespace clang_update_v6 {

constexpr frozen::unordered_map<clang_v5::FormatStyle::BracketAlignmentStyle,
                                clang_v6::FormatStyle::BracketAlignmentStyle, 3>
    bracket_all_alignment_style{
        {clang_v5::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v6::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v5::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v6::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v5::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v6::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<
    clang_v5::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v6::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v5::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v6::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v5::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v6::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v5::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v6::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v5::FormatStyle::ShortFunctionStyle,
                                clang_v6::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v5::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v6::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v5::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v6::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v5::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v6::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v5::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v6::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v5::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v6::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<
    clang_v5::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v6::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v5::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v6::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v5::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v6::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v5::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v6::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v5::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v6::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v5::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v6::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v5::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v6::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v5::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v6::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v5::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v6::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v5::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v6::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<clang_v5::FormatStyle::BinaryOperatorStyle,
                                clang_v6::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v5::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v6::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v5::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v6::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v5::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v6::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v5::FormatStyle::BraceBreakingStyle,
                                clang_v6::FormatStyle::BraceBreakingStyle, 8>
    brace_breaking_style{
        {clang_v5::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v6::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v5::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v6::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v5::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v6::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v5::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v6::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v5::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v6::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v5::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v6::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v5::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v6::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v5::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v6::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v5::FormatStyle::BreakConstructorInitializersStyle,
    clang_v6::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v5::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v6::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v5::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v6::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v5::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v6::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

void assign(std::vector<clang_v5::FormatStyle::IncludeCategory> &lhs,
            std::vector<clang_v6::FormatStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(
        clang_v6::FormatStyle::IncludeCategory{item.Regex, item.Priority});
  }
}

constexpr frozen::unordered_map<clang_v5::FormatStyle::JavaScriptQuoteStyle,
                                clang_v6::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v5::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v6::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v5::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v6::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v5::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v6::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v5::FormatStyle::LanguageKind,
                                clang_v6::FormatStyle::LanguageKind, 7>
    language_king{{clang_v5::FormatStyle::LanguageKind::LK_None,
                   clang_v6::FormatStyle::LanguageKind::LK_None},
                  {clang_v5::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v6::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v5::FormatStyle::LanguageKind::LK_Java,
                   clang_v6::FormatStyle::LanguageKind::LK_Java},
                  {clang_v5::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v6::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v5::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v6::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v5::FormatStyle::LanguageKind::LK_Proto,
                   clang_v6::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v5::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v6::FormatStyle::LanguageKind::LK_TableGen}};

constexpr frozen::unordered_map<clang_v5::FormatStyle::NamespaceIndentationKind,
                                clang_v6::FormatStyle::NamespaceIndentationKind,
                                3>
    namespace_indentation_kind{
        {clang_v5::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v6::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v5::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v6::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v5::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v6::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v5::FormatStyle::PointerAlignmentStyle,
                                clang_v6::FormatStyle::PointerAlignmentStyle, 3>
    pointer_alignment_style{
        {clang_v5::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v6::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v5::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v6::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v5::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v6::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

constexpr frozen::unordered_map<clang_v5::FormatStyle::SpaceBeforeParensOptions,
                                clang_v6::FormatStyle::SpaceBeforeParensOptions,
                                3>
    space_before_parens_options{
        {clang_v5::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v6::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v5::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v6::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v5::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v6::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v5::FormatStyle::LanguageStandard,
                                clang_v6::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v5::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v6::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v5::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v6::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v5::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v6::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v5::FormatStyle::UseTabStyle,
                                clang_v6::FormatStyle::UseTabStyle, 4>
    use_tab_style{
        {clang_v5::FormatStyle::UseTabStyle::UT_Never,
         clang_v6::FormatStyle::UseTabStyle::UT_Never},
        {clang_v5::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v6::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v5::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v6::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v5::FormatStyle::UseTabStyle::UT_Always,
         clang_v6::FormatStyle::UseTabStyle::UT_Always}};

clang_v6::FormatStyle update(clang_v5::FormatStyle &old,
                             const std::string &style) {
  clang_v6::FormatStyle retval;
  if (!clang_v6::getPredefinedStyle(
          style, clang_v6::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignConsecutiveAssignments = old.AlignConsecutiveAssignments;
  retval.AlignConsecutiveDeclarations = old.AlignConsecutiveDeclarations;
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
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
  newField("BraceWrapping.AfterExternBlock", "6",
           retval.BraceWrapping.AfterExternBlock);
  retval.BraceWrapping.BeforeCatch = old.BraceWrapping.BeforeCatch;
  retval.BraceWrapping.BeforeElse = old.BraceWrapping.BeforeElse;
  newField("BraceWrapping.IndentBraces", "6",
           retval.BraceWrapping.IndentBraces);
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializers =
      break_constructor_initializers_style.at(old.BreakConstructorInitializers);
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.BreakBeforeInheritanceComma = old.BreakBeforeInheritanceComma;
  retval.CompactNamespaces = old.CompactNamespaces;
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
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  newField("IncludeBlocks", "6", retval.IncludeBlocks);
  assign(old.IncludeCategories, retval.IncludeCategories);
  retval.IncludeIsMainRegex = old.IncludeIsMainRegex;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  newField("IndentPPDirectives", "6", retval.IndentPPDirectives);
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
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  newField("RawStringFormats", "6", retval.RawStringFormats);
  retval.ReflowComments = old.ReflowComments;
  retval.SortIncludes = old.SortIncludes;
  retval.SortUsingDeclarations = old.SortUsingDeclarations;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
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

  return retval;
}

} // namespace clang_update_v6

namespace clang_update_v7 {

constexpr frozen::unordered_map<clang_v6::FormatStyle::BracketAlignmentStyle,
                                clang_v7::FormatStyle::BracketAlignmentStyle, 3>
    bracket_all_alignment_style{
        {clang_v6::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v7::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v6::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v7::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v6::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v7::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<
    clang_v6::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v7::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v6::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v7::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v6::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v7::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v6::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v7::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::ShortFunctionStyle,
                                clang_v7::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v6::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v7::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v6::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v7::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v6::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v7::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v6::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v7::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v6::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v7::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<
    clang_v6::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v7::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v6::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v7::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v6::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v7::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v6::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v7::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<
    bool, clang_v7::FormatStyle::BreakTemplateDeclarationsStyle, 2>
    break_template_declarations_style{
        {false,
         clang_v7::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine},
        {true,
         clang_v7::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v7::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v6::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v7::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v6::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v7::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v6::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v7::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v6::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v7::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v6::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v7::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::BinaryOperatorStyle,
                                clang_v7::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v6::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v7::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v6::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v7::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v6::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v7::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::BraceBreakingStyle,
                                clang_v7::FormatStyle::BraceBreakingStyle, 8>
    brace_breaking_style{
        {clang_v6::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v7::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v6::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v7::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v6::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v7::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v6::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v7::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v6::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v7::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v6::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v7::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v6::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v7::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v6::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v7::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v6::FormatStyle::BreakConstructorInitializersStyle,
    clang_v7::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v6::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v7::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v6::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v7::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v6::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v7::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    bool, clang_v7::FormatStyle::BreakInheritanceListStyle, 2>
    break_inheritance_list_style{
        {false,
         clang_v7::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {true,
         clang_v7::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::IncludeBlocksStyle,
                                clang_v7::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v6::FormatStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v7::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v6::FormatStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v7::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v6::FormatStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v7::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v6::FormatStyle::IncludeCategory> &lhs,
            std::vector<clang_v7::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(
        clang_v7::IncludeStyle::IncludeCategory{item.Regex, item.Priority});
  }
}

constexpr frozen::unordered_map<clang_v6::FormatStyle::PPDirectiveIndentStyle,
                                clang_v7::FormatStyle::PPDirectiveIndentStyle,
                                2>
    pp_directive_indent_style{
        {clang_v6::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v7::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v6::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v7::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::JavaScriptQuoteStyle,
                                clang_v7::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v6::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v7::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v6::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v7::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v6::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v7::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::LanguageKind,
                                clang_v7::FormatStyle::LanguageKind, 7>
    language_king{{clang_v6::FormatStyle::LanguageKind::LK_None,
                   clang_v7::FormatStyle::LanguageKind::LK_None},
                  {clang_v6::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v7::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v6::FormatStyle::LanguageKind::LK_Java,
                   clang_v7::FormatStyle::LanguageKind::LK_Java},
                  {clang_v6::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v7::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v6::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v7::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v6::FormatStyle::LanguageKind::LK_Proto,
                   clang_v7::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v6::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v7::FormatStyle::LanguageKind::LK_TableGen}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::NamespaceIndentationKind,
                                clang_v7::FormatStyle::NamespaceIndentationKind,
                                3>
    namespace_indentation_kind{
        {clang_v6::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v7::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v6::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v7::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v6::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v7::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::PointerAlignmentStyle,
                                clang_v7::FormatStyle::PointerAlignmentStyle, 3>
    pointer_alignment_style{
        {clang_v6::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v7::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v6::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v7::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v6::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v7::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

void assign(std::vector<clang_v6::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v7::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(
        clang_v7::FormatStyle::RawStringFormat{language_king.at(item.Language),
                                               {item.Delimiter},
                                               {},
                                               {},
                                               item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<clang_v6::FormatStyle::SpaceBeforeParensOptions,
                                clang_v7::FormatStyle::SpaceBeforeParensOptions,
                                3>
    space_before_parens_options{
        {clang_v6::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v7::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v6::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v7::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v6::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v7::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::LanguageStandard,
                                clang_v7::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v6::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v7::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v6::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v7::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v6::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v7::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v6::FormatStyle::UseTabStyle,
                                clang_v7::FormatStyle::UseTabStyle, 4>
    use_tab_style{
        {clang_v6::FormatStyle::UseTabStyle::UT_Never,
         clang_v7::FormatStyle::UseTabStyle::UT_Never},
        {clang_v6::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v7::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v6::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v7::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v6::FormatStyle::UseTabStyle::UT_Always,
         clang_v7::FormatStyle::UseTabStyle::UT_Always}};

clang_v7::FormatStyle update(clang_v6::FormatStyle &old,
                             const std::string &style) {
  clang_v7::FormatStyle retval;
  if (!clang_v7::getPredefinedStyle(
          style, clang_v7::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignConsecutiveAssignments = old.AlignConsecutiveAssignments;
  retval.AlignConsecutiveDeclarations = old.AlignConsecutiveDeclarations;
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
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
  retval.AlwaysBreakTemplateDeclarations =
      break_template_declarations_style.at(old.AlwaysBreakTemplateDeclarations);
  improveField("AlwaysBreakTemplateDeclarations", "No", "7");
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
  retval.BraceWrapping.AfterExternBlock = old.BraceWrapping.AfterExternBlock;
  retval.BraceWrapping.BeforeCatch = old.BraceWrapping.BeforeCatch;
  retval.BraceWrapping.BeforeElse = old.BraceWrapping.BeforeElse;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializers =
      break_constructor_initializers_style.at(old.BreakConstructorInitializers);
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.BreakInheritanceList =
      break_inheritance_list_style.at(old.BreakBeforeInheritanceComma);
  retval.CompactNamespaces = old.CompactNamespaces;
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
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeBlocks);
  assign(old.IncludeCategories, retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeIsMainRegex;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
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
  newField("ObjCBinPackProtocolList", "7", retval.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  newField("PenaltyBreakTemplateDeclaration", "7",
           retval.PenaltyBreakTemplateDeclaration);
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  assign(old.RawStringFormats, retval.RawStringFormats);
  improveField("RawStringFormats", "EnclosingFunctions", "7");
  improveField("RawStringFormats", "CanonicalDelimiter", "7");
  retval.ReflowComments = old.ReflowComments;
  retval.SortIncludes = old.SortIncludes;
  retval.SortUsingDeclarations = old.SortUsingDeclarations;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  newField("SpaceBeforeCpp11BracedList", "7",
           retval.SpaceBeforeCpp11BracedList);
  newField("SpaceBeforeCtorInitializerColon", "7",
           retval.SpaceBeforeCtorInitializerColon);
  newField("SpaceBeforeInheritanceColon", "7",
           retval.SpaceBeforeInheritanceColon);
  newField("SpaceBeforeRangeBasedForLoopColon", "7",
           retval.SpaceBeforeRangeBasedForLoopColon);
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

  return retval;
}

} // namespace clang_update_v7

namespace clang_update_v8 {

constexpr frozen::unordered_map<clang_v7::FormatStyle::BracketAlignmentStyle,
                                clang_v8::FormatStyle::BracketAlignmentStyle, 3>
    bracket_all_alignment_style{
        {clang_v7::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v8::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v7::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v8::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v7::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v8::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<
    clang_v7::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v8::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v7::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v8::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v7::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v8::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v7::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v8::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v7::FormatStyle::ShortFunctionStyle,
                                clang_v8::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v7::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v8::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v7::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v8::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v7::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v8::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v7::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v8::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v7::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v8::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<
    clang_v7::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v8::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v7::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v8::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v7::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v8::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v7::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v8::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v7::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v8::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v7::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v8::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v7::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v8::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v7::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v8::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v7::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v8::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v7::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v8::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v7::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v8::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v7::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v8::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v7::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v8::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine},
        {clang_v7::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v8::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<clang_v7::FormatStyle::BinaryOperatorStyle,
                                clang_v8::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v7::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v8::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v7::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v8::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v7::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v8::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v7::FormatStyle::BraceBreakingStyle,
                                clang_v8::FormatStyle::BraceBreakingStyle, 8>
    brace_breaking_style{
        {clang_v7::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v8::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v7::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v8::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v7::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v8::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v7::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v8::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v7::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v8::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v7::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v8::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v7::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v8::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v7::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v8::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v7::FormatStyle::BreakConstructorInitializersStyle,
    clang_v8::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v7::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v8::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v7::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v8::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v7::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v8::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v7::FormatStyle::BreakInheritanceListStyle,
    clang_v8::FormatStyle::BreakInheritanceListStyle, 3>
    break_inheritance_list_style{
        {clang_v7::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v8::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v7::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v8::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v7::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v8::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon}};

constexpr frozen::unordered_map<clang_v7::IncludeStyle::IncludeBlocksStyle,
                                clang_v8::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v7::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v8::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v7::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v8::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v7::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v8::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v7::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v8::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(
        clang_v8::IncludeStyle::IncludeCategory{item.Regex, item.Priority});
  }
}

constexpr frozen::unordered_map<clang_v7::FormatStyle::PPDirectiveIndentStyle,
                                clang_v8::FormatStyle::PPDirectiveIndentStyle,
                                2>
    pp_directive_indent_style{
        {clang_v7::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v8::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v7::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v8::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash}};

constexpr frozen::unordered_map<clang_v7::FormatStyle::JavaScriptQuoteStyle,
                                clang_v8::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v7::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v8::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v7::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v8::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v7::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v8::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v7::FormatStyle::LanguageKind,
                                clang_v8::FormatStyle::LanguageKind, 8>
    language_king{{clang_v7::FormatStyle::LanguageKind::LK_None,
                   clang_v8::FormatStyle::LanguageKind::LK_None},
                  {clang_v7::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v8::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v7::FormatStyle::LanguageKind::LK_Java,
                   clang_v8::FormatStyle::LanguageKind::LK_Java},
                  {clang_v7::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v8::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v7::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v8::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v7::FormatStyle::LanguageKind::LK_Proto,
                   clang_v8::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v7::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v8::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v7::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v8::FormatStyle::LanguageKind::LK_TextProto}};

constexpr frozen::unordered_map<clang_v7::FormatStyle::NamespaceIndentationKind,
                                clang_v8::FormatStyle::NamespaceIndentationKind,
                                3>
    namespace_indentation_kind{
        {clang_v7::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v8::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v7::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v8::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v7::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v8::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v7::FormatStyle::PointerAlignmentStyle,
                                clang_v8::FormatStyle::PointerAlignmentStyle, 3>
    pointer_alignment_style{
        {clang_v7::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v8::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v7::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v8::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v7::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v8::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

void assign(std::vector<clang_v7::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v8::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v8::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<clang_v7::FormatStyle::SpaceBeforeParensOptions,
                                clang_v8::FormatStyle::SpaceBeforeParensOptions,
                                3>
    space_before_parens_options{
        {clang_v7::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v8::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v7::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v8::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v7::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v8::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v7::FormatStyle::LanguageStandard,
                                clang_v8::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v7::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v8::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v7::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v8::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v7::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v8::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v7::FormatStyle::UseTabStyle,
                                clang_v8::FormatStyle::UseTabStyle, 4>
    use_tab_style{
        {clang_v7::FormatStyle::UseTabStyle::UT_Never,
         clang_v8::FormatStyle::UseTabStyle::UT_Never},
        {clang_v7::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v8::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v7::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v8::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v7::FormatStyle::UseTabStyle::UT_Always,
         clang_v8::FormatStyle::UseTabStyle::UT_Always}};

clang_v8::FormatStyle update(clang_v7::FormatStyle &old,
                             const std::string &style) {
  clang_v8::FormatStyle retval;
  if (!clang_v8::getPredefinedStyle(
          style, clang_v8::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignConsecutiveAssignments = old.AlignConsecutiveAssignments;
  retval.AlignConsecutiveDeclarations = old.AlignConsecutiveDeclarations;
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
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
  retval.AlwaysBreakTemplateDeclarations =
      break_template_declarations_style.at(old.AlwaysBreakTemplateDeclarations);
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
  retval.BraceWrapping.AfterExternBlock = old.BraceWrapping.AfterExternBlock;
  retval.BraceWrapping.BeforeCatch = old.BraceWrapping.BeforeCatch;
  retval.BraceWrapping.BeforeElse = old.BraceWrapping.BeforeElse;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializers =
      break_constructor_initializers_style.at(old.BreakConstructorInitializers);
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.BreakInheritanceList =
      break_inheritance_list_style.at(old.BreakInheritanceList);
  retval.CompactNamespaces = old.CompactNamespaces;
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
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  newField("StatementMacros", "8", retval.StatementMacros);
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  newField("JavaImportGroups", "8", retval.JavaImportGroups);
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
  newField("ObjCBinPackProtocolList", "7", retval.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  newField("PenaltyBreakTemplateDeclaration", "7",
           retval.PenaltyBreakTemplateDeclaration);
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  assign(old.RawStringFormats, retval.RawStringFormats);
  retval.ReflowComments = old.ReflowComments;
  retval.SortIncludes = old.SortIncludes;
  retval.SortUsingDeclarations = old.SortUsingDeclarations;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
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

  return retval;
}

} // namespace clang_update_v8

namespace clang_update_v9 {

constexpr frozen::unordered_map<clang_v8::FormatStyle::BracketAlignmentStyle,
                                clang_v9::FormatStyle::BracketAlignmentStyle, 3>
    bracket_all_alignment_style{
        {clang_v8::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v9::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v8::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v9::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v8::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v9::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<
    clang_v8::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v9::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v8::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v9::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v8::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v9::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v8::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v9::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::ShortFunctionStyle,
                                clang_v9::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v8::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v9::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v8::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v9::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v8::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v9::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v8::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v9::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v8::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v9::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<bool, clang_v9::FormatStyle::ShortIfStyle, 2>
    short_if_style{
        {false, clang_v9::FormatStyle::ShortIfStyle::SIS_Never},
        {true, clang_v9::FormatStyle::ShortIfStyle::SIS_WithoutElse}};

constexpr frozen::unordered_map<
    clang_v8::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v9::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v8::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v9::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v8::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v9::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v8::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v9::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v9::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v8::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v9::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v8::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v9::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v8::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v9::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v8::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v9::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v8::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v9::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v8::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v9::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v8::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v9::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v8::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v9::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine},
        {clang_v8::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v9::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::BinaryOperatorStyle,
                                clang_v9::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v8::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v9::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v8::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v9::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v8::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v9::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::BraceBreakingStyle,
                                clang_v9::FormatStyle::BraceBreakingStyle, 8>
    brace_breaking_style{
        {clang_v8::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v9::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v8::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v9::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v8::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v9::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v8::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v9::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v8::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v9::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v8::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v9::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v8::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v9::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v8::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v9::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v8::FormatStyle::BreakConstructorInitializersStyle,
    clang_v9::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v8::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v9::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v8::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v9::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v8::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v9::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v8::FormatStyle::BreakInheritanceListStyle,
    clang_v9::FormatStyle::BreakInheritanceListStyle, 3>
    break_inheritance_list_style{
        {clang_v8::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v9::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v8::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v9::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v8::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v9::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon}};

constexpr frozen::unordered_map<clang_v8::IncludeStyle::IncludeBlocksStyle,
                                clang_v9::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v8::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v9::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v8::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v9::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v8::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v9::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v8::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v9::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(
        clang_v9::IncludeStyle::IncludeCategory{item.Regex, item.Priority});
  }
}

constexpr frozen::unordered_map<clang_v8::FormatStyle::PPDirectiveIndentStyle,
                                clang_v9::FormatStyle::PPDirectiveIndentStyle,
                                2>
    pp_directive_indent_style{
        {clang_v8::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v9::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v8::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v9::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::JavaScriptQuoteStyle,
                                clang_v9::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v8::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v9::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v8::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v9::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v8::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v9::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::LanguageKind,
                                clang_v9::FormatStyle::LanguageKind, 8>
    language_king{{clang_v8::FormatStyle::LanguageKind::LK_None,
                   clang_v9::FormatStyle::LanguageKind::LK_None},
                  {clang_v8::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v9::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v8::FormatStyle::LanguageKind::LK_Java,
                   clang_v9::FormatStyle::LanguageKind::LK_Java},
                  {clang_v8::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v9::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v8::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v9::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v8::FormatStyle::LanguageKind::LK_Proto,
                   clang_v9::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v8::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v9::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v8::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v9::FormatStyle::LanguageKind::LK_TextProto}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::NamespaceIndentationKind,
                                clang_v9::FormatStyle::NamespaceIndentationKind,
                                3>
    namespace_indentation_kind{
        {clang_v8::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v9::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v8::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v9::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v8::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v9::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::BinPackStyle,
                                clang_v9::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v8::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v9::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v8::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v9::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v8::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v9::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::PointerAlignmentStyle,
                                clang_v9::FormatStyle::PointerAlignmentStyle, 3>
    pointer_alignment_style{
        {clang_v8::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v9::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v8::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v9::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v8::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v9::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

void assign(std::vector<clang_v8::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v9::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v9::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<clang_v8::FormatStyle::SpaceBeforeParensOptions,
                                clang_v9::FormatStyle::SpaceBeforeParensOptions,
                                3>
    space_before_parens_options{
        {clang_v8::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v9::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v8::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v9::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v8::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v9::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::LanguageStandard,
                                clang_v9::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v8::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v9::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v8::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v9::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v8::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v9::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v8::FormatStyle::UseTabStyle,
                                clang_v9::FormatStyle::UseTabStyle, 4>
    use_tab_style{
        {clang_v8::FormatStyle::UseTabStyle::UT_Never,
         clang_v9::FormatStyle::UseTabStyle::UT_Never},
        {clang_v8::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v9::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v8::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v9::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v8::FormatStyle::UseTabStyle::UT_Always,
         clang_v9::FormatStyle::UseTabStyle::UT_Always}};

clang_v9::FormatStyle update(clang_v8::FormatStyle &old,
                             const std::string &style) {
  clang_v9::FormatStyle retval;
  if (!clang_v9::getPredefinedStyle(
          style, clang_v9::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  newField("AlignConsecutiveMacros", "9", retval.AlignConsecutiveMacros);
  retval.AlignConsecutiveAssignments = old.AlignConsecutiveAssignments;
  retval.AlignConsecutiveDeclarations = old.AlignConsecutiveDeclarations;
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  retval.AlignOperands = old.AlignOperands;
  retval.AlignTrailingComments = old.AlignTrailingComments;
  newField("AllowAllArgumentsOnNextLine", "9",
           retval.AllowAllArgumentsOnNextLine);
  newField("AllowAllConstructorInitializersOnNextLine", "9",
           retval.AllowAllConstructorInitializersOnNextLine);
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortBlocksOnASingleLine = old.AllowShortBlocksOnASingleLine;
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  retval.AllowShortFunctionsOnASingleLine =
      short_function_style.at(old.AllowShortFunctionsOnASingleLine);
  retval.AllowShortIfStatementsOnASingleLine =
      short_if_style.at(old.AllowShortIfStatementsOnASingleLine);
  newField("AllowShortLambdasOnASingleLine", "9",
           retval.AllowShortLambdasOnASingleLine);
  retval.AllowShortLoopsOnASingleLine = old.AllowShortLoopsOnASingleLine;
  retval.AlwaysBreakAfterDefinitionReturnType =
      definition_return_type_breaking_style.at(
          old.AlwaysBreakAfterDefinitionReturnType);
  retval.AlwaysBreakAfterReturnType =
      return_type_breaking_style.at(old.AlwaysBreakAfterReturnType);
  retval.AlwaysBreakBeforeMultilineStrings =
      old.AlwaysBreakBeforeMultilineStrings;
  retval.AlwaysBreakTemplateDeclarations =
      break_template_declarations_style.at(old.AlwaysBreakTemplateDeclarations);
  retval.BinPackArguments = old.BinPackArguments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  newField("BraceWrapping.AfterCaseLabel", "9",
           retval.BraceWrapping.AfterCaseLabel);
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
  retval.BraceWrapping.AfterExternBlock = old.BraceWrapping.AfterExternBlock;
  retval.BraceWrapping.BeforeCatch = old.BraceWrapping.BeforeCatch;
  retval.BraceWrapping.BeforeElse = old.BraceWrapping.BeforeElse;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializers =
      break_constructor_initializers_style.at(old.BreakConstructorInitializers);
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.BreakInheritanceList =
      break_inheritance_list_style.at(old.BreakInheritanceList);
  retval.CompactNamespaces = old.CompactNamespaces;
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
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  newField("TypenameMacros", "9", retval.TypenameMacros);
  retval.StatementMacros = old.StatementMacros;
  newField("NamespaceMacros", "9", retval.NamespaceMacros);
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  improveField("IndentPPDirectives", "BeforeHash", "9");
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.JavaImportGroups = old.JavaImportGroups;
  retval.JavaScriptQuotes = java_script_quote_style.at(old.JavaScriptQuotes);
  retval.JavaScriptWrapImports = old.JavaScriptWrapImports;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.Language = language_king.at(old.Language);
  improveField("Language", "CSharp", "9");
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.ObjCBinPackProtocolList =
      bin_pack_style.at(old.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyBreakTemplateDeclaration = old.PenaltyBreakTemplateDeclaration;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  assign(old.RawStringFormats, retval.RawStringFormats);
  retval.ReflowComments = old.ReflowComments;
  retval.SortIncludes = old.SortIncludes;
  retval.SortUsingDeclarations = old.SortUsingDeclarations;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  newField("SpaceAfterLogicalNot", "9", retval.SpaceAfterLogicalNot);
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  improveField("SpaceBeforeParens", "NonEmptyParentheses", "9");
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
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

  return retval;
}

} // namespace clang_update_v9

namespace clang_update_v10 {

constexpr frozen::unordered_map<clang_v9::FormatStyle::BracketAlignmentStyle,
                                clang_v10::FormatStyle::BracketAlignmentStyle,
                                3>
    bracket_all_alignment_style{
        {clang_v9::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v10::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v9::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v10::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v9::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v10::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<
    clang_v9::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v10::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v9::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v10::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v9::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v10::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v9::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v10::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<bool, clang_v10::FormatStyle::ShortBlockStyle,
                                2>
    short_block_style{
        {false, clang_v10::FormatStyle::ShortBlockStyle::SBS_Never},
        {true, clang_v10::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::ShortFunctionStyle,
                                clang_v10::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v9::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v10::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v9::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v10::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v9::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v10::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v9::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v10::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v9::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v10::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::ShortIfStyle,
                                clang_v10::FormatStyle::ShortIfStyle, 3>
    short_if_style{{clang_v9::FormatStyle::ShortIfStyle::SIS_Never,
                    clang_v10::FormatStyle::ShortIfStyle::SIS_Never},
                   {clang_v9::FormatStyle::ShortIfStyle::SIS_WithoutElse,
                    clang_v10::FormatStyle::ShortIfStyle::SIS_WithoutElse},
                   {clang_v9::FormatStyle::ShortIfStyle::SIS_Always,
                    clang_v10::FormatStyle::ShortIfStyle::SIS_Always}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::ShortLambdaStyle,
                                clang_v10::FormatStyle::ShortLambdaStyle, 4>
    short_lambda_style{{clang_v9::FormatStyle::ShortLambdaStyle::SLS_None,
                        clang_v10::FormatStyle::ShortLambdaStyle::SLS_None},
                       {clang_v9::FormatStyle::ShortLambdaStyle::SLS_Empty,
                        clang_v10::FormatStyle::ShortLambdaStyle::SLS_Empty},
                       {clang_v9::FormatStyle::ShortLambdaStyle::SLS_Inline,
                        clang_v10::FormatStyle::ShortLambdaStyle::SLS_Inline},
                       {clang_v9::FormatStyle::ShortLambdaStyle::SLS_All,
                        clang_v10::FormatStyle::ShortLambdaStyle::SLS_All}};

constexpr frozen::unordered_map<
    clang_v9::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v10::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v9::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v10::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v9::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v10::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v9::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v10::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v10::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v9::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v10::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v9::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v10::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v9::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v10::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v9::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v10::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v9::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v10::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v9::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v10::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v9::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v10::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v9::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v10::FormatStyle::BreakTemplateDeclarationsStyle::
             BTDS_MultiLine},
        {clang_v9::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v10::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::BinaryOperatorStyle,
                                clang_v10::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v9::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v10::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v9::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v10::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v9::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v10::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::BraceBreakingStyle,
                                clang_v10::FormatStyle::BraceBreakingStyle, 8>
    brace_breaking_style{
        {clang_v9::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v10::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v9::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v10::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v9::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v10::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v9::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v10::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v9::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v10::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v9::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v10::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v9::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v10::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v9::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v10::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    bool, clang_v10::FormatStyle::BraceWrappingAfterControlStatementStyle, 2>
    brace_wrapping_after_control_statement_style{
        {false, clang_v10::FormatStyle::
                    BraceWrappingAfterControlStatementStyle::BWACS_Never},
        {true, clang_v10::FormatStyle::BraceWrappingAfterControlStatementStyle::
                   BWACS_Always}};

constexpr frozen::unordered_map<
    clang_v9::FormatStyle::BreakConstructorInitializersStyle,
    clang_v10::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v9::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v10::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v9::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v10::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v9::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v10::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v9::FormatStyle::BreakInheritanceListStyle,
    clang_v10::FormatStyle::BreakInheritanceListStyle, 3>
    break_inheritance_list_style{
        {clang_v9::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v10::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v9::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v10::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v9::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v10::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon}};

constexpr frozen::unordered_map<clang_v9::IncludeStyle::IncludeBlocksStyle,
                                clang_v10::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v9::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v10::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v9::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v10::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v9::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v10::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v9::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v10::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v10::IncludeStyle::IncludeCategory{
        item.Regex, item.Priority, {}});
  }
}

constexpr frozen::unordered_map<clang_v9::FormatStyle::PPDirectiveIndentStyle,
                                clang_v10::FormatStyle::PPDirectiveIndentStyle,
                                3>
    pp_directive_indent_style{
        {clang_v9::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v10::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v9::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v10::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash},
        {clang_v9::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash,
         clang_v10::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::JavaScriptQuoteStyle,
                                clang_v10::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v9::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v10::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v9::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v10::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v9::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v10::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::LanguageKind,
                                clang_v10::FormatStyle::LanguageKind, 9>
    language_king{{clang_v9::FormatStyle::LanguageKind::LK_None,
                   clang_v10::FormatStyle::LanguageKind::LK_None},
                  {clang_v9::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v10::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v9::FormatStyle::LanguageKind::LK_CSharp,
                   clang_v10::FormatStyle::LanguageKind::LK_CSharp},
                  {clang_v9::FormatStyle::LanguageKind::LK_Java,
                   clang_v10::FormatStyle::LanguageKind::LK_Java},
                  {clang_v9::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v10::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v9::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v10::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v9::FormatStyle::LanguageKind::LK_Proto,
                   clang_v10::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v9::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v10::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v9::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v10::FormatStyle::LanguageKind::LK_TextProto}};

constexpr frozen::unordered_map<
    clang_v9::FormatStyle::NamespaceIndentationKind,
    clang_v10::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v9::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v10::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v9::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v10::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v9::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v10::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::BinPackStyle,
                                clang_v10::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v9::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v10::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v9::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v10::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v9::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v10::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::PointerAlignmentStyle,
                                clang_v10::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v9::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v10::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v9::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v10::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v9::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v10::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

void assign(std::vector<clang_v9::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v10::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v10::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<
    clang_v9::FormatStyle::SpaceBeforeParensOptions,
    clang_v10::FormatStyle::SpaceBeforeParensOptions, 4>
    space_before_parens_options{
        {clang_v9::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v10::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v9::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v10::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v9::FormatStyle::SpaceBeforeParensOptions::
             SBPO_NonEmptyParentheses,
         clang_v10::FormatStyle::SpaceBeforeParensOptions::
             SBPO_NonEmptyParentheses},
        {clang_v9::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v10::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::LanguageStandard,
                                clang_v10::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v9::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v10::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v9::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v10::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v9::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v10::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v9::FormatStyle::UseTabStyle,
                                clang_v10::FormatStyle::UseTabStyle, 4>
    use_tab_style{
        {clang_v9::FormatStyle::UseTabStyle::UT_Never,
         clang_v10::FormatStyle::UseTabStyle::UT_Never},
        {clang_v9::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v10::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v9::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v10::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v9::FormatStyle::UseTabStyle::UT_Always,
         clang_v10::FormatStyle::UseTabStyle::UT_Always}};

clang_v10::FormatStyle update(clang_v9::FormatStyle &old,
                              const std::string &style) {
  clang_v10::FormatStyle retval;
  if (!clang_v10::getPredefinedStyle(
          style, clang_v10::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignConsecutiveMacros = old.AlignConsecutiveMacros;
  retval.AlignConsecutiveAssignments = old.AlignConsecutiveAssignments;
  retval.AlignConsecutiveDeclarations = old.AlignConsecutiveDeclarations;
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  retval.AlignOperands = old.AlignOperands;
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AllowAllArgumentsOnNextLine = old.AllowAllArgumentsOnNextLine;
  retval.AllowAllConstructorInitializersOnNextLine =
      old.AllowAllConstructorInitializersOnNextLine;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortBlocksOnASingleLine =
      short_block_style.at(old.AllowShortBlocksOnASingleLine);
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  retval.AllowShortFunctionsOnASingleLine =
      short_function_style.at(old.AllowShortFunctionsOnASingleLine);
  retval.AllowShortIfStatementsOnASingleLine =
      short_if_style.at(old.AllowShortIfStatementsOnASingleLine);
  retval.AllowShortLambdasOnASingleLine =
      short_lambda_style.at(old.AllowShortLambdasOnASingleLine);
  retval.AllowShortLoopsOnASingleLine = old.AllowShortLoopsOnASingleLine;
  retval.AlwaysBreakAfterDefinitionReturnType =
      definition_return_type_breaking_style.at(
          old.AlwaysBreakAfterDefinitionReturnType);
  retval.AlwaysBreakAfterReturnType =
      return_type_breaking_style.at(old.AlwaysBreakAfterReturnType);
  retval.AlwaysBreakBeforeMultilineStrings =
      old.AlwaysBreakBeforeMultilineStrings;
  retval.AlwaysBreakTemplateDeclarations =
      break_template_declarations_style.at(old.AlwaysBreakTemplateDeclarations);
  retval.BinPackArguments = old.BinPackArguments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  improveField("BreakBeforeBraces", "Whitesmiths", "10");
  retval.BraceWrapping.AfterCaseLabel = old.BraceWrapping.AfterCaseLabel;
  retval.BraceWrapping.AfterClass = old.BraceWrapping.AfterClass;
  retval.BraceWrapping.AfterControlStatement =
      brace_wrapping_after_control_statement_style.at(
          old.BraceWrapping.AfterControlStatement);
  retval.BraceWrapping.AfterEnum = old.BraceWrapping.AfterEnum;
  retval.BraceWrapping.AfterFunction = old.BraceWrapping.AfterFunction;
  retval.BraceWrapping.AfterNamespace = old.BraceWrapping.AfterNamespace;
  retval.BraceWrapping.AfterObjCDeclaration =
      old.BraceWrapping.AfterObjCDeclaration;
  retval.BraceWrapping.AfterStruct = old.BraceWrapping.AfterStruct;
  retval.BraceWrapping.AfterUnion = old.BraceWrapping.AfterUnion;
  retval.BraceWrapping.AfterExternBlock = old.BraceWrapping.AfterExternBlock;
  retval.BraceWrapping.BeforeCatch = old.BraceWrapping.BeforeCatch;
  retval.BraceWrapping.BeforeElse = old.BraceWrapping.BeforeElse;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializers =
      break_constructor_initializers_style.at(old.BreakConstructorInitializers);
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.BreakInheritanceList =
      break_inheritance_list_style.at(old.BreakInheritanceList);
  retval.CompactNamespaces = old.CompactNamespaces;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  newField("DeriveLineEnding", "10", retval.DeriveLineEnding);
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  retval.TypenameMacros = old.TypenameMacros;
  retval.StatementMacros = old.StatementMacros;
  retval.NamespaceMacros = old.NamespaceMacros;
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  newField("IncludeIsMainSourceRegex", "10", retval.IncludeStyle.IncludeIsMainSourceRegex);
  retval.IndentCaseLabels = old.IndentCaseLabels;
  newField("IndentGotoLabels", "10", retval.IndentGotoLabels);
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.JavaImportGroups = old.JavaImportGroups;
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
  retval.ObjCBinPackProtocolList =
      bin_pack_style.at(old.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyBreakTemplateDeclaration = old.PenaltyBreakTemplateDeclaration;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  assign(old.RawStringFormats, retval.RawStringFormats);
  retval.ReflowComments = old.ReflowComments;
  retval.SortIncludes = old.SortIncludes;
  retval.SortUsingDeclarations = old.SortUsingDeclarations;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterLogicalNot = old.SpaceAfterLogicalNot;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
  newField("SpaceInEmptyBlock", "10", retval.SpaceInEmptyBlock);
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = old.SpacesInAngles;
  newField("SpacesInConditionalStatement", "10",
           retval.SpacesInConditionalStatement);
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  newField("SpaceBeforeSquareBrackets", "10",
           retval.SpaceBeforeSquareBrackets);
  retval.Standard = language_standard.at(old.Standard);
  improveField("Standard", "Cpp14", "10");
  improveField("Standard", "Cpp17", "10");
  improveField("Standard", "Cpp10", "10");
  improveField("Standard", "Latest", "10");
  retval.TabWidth = old.TabWidth;
  newField("UseCRLF", "10", retval.UseCRLF);
  retval.UseTab = use_tab_style.at(old.UseTab);

  return retval;
}

} // namespace clang_update_v10
