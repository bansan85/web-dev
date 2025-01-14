#include "update.h"
#include <frozen/unordered_map.h>
#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <stdexcept>
#include <string_view>

#define XSTR(S) STR(S)
#define STR(S) #S

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

template <clang_vx::Update Upgrade = clang_vx::Update::UPGRADE, typename T>
void newField(std::string_view field_name, std::string_view version,
              const T &field_value) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    std::cout << "Info when upgrading to";
  } else {
    std::cout << "Warning when downgrading from";
  }
  std::cout << " version " << version << ". ";
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    std::cout << "New";
  } else {
    std::cout << "Removed";
  }
  std::cout << " field " << field_name << " with value " << field_value
            << ".\n";
}

template <clang_vx::Update Upgrade = clang_vx::Update::UPGRADE, typename T>
void newField(std::string_view field_name, std::string_view version,
              const std::optional<T> &field_value) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    std::cout << "Info when upgrading to";
  } else {
    std::cout << "Warning when downgrading from";
  }
  std::cout << " version " << version << ". ";
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    std::cout << "New";
  } else {
    std::cout << "Removed";
  }
  std::cout << " field " << field_name << " with value ";
  if (!field_value) {
    std::cout << "undefined";
  } else {
    std::cout << *field_value;
  }
  std::cout << ".\n";
}

void improveField(std::string_view field_name, std::string_view new_option,
                  std::string_view version) {
  std::cout << "Info when migrating to version " << version << ". Field "
            << field_name << " has new feature " << new_option << ".\n";
}

template <clang_vx::Update Upgrade = clang_vx::Update::UPGRADE, typename T,
          typename U>
void assignSameField(T &old_field, U &new_field) {
  static_assert(std::is_same_v<T, U>);
  static_assert(!std::is_enum_v<T>);
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    new_field = old_field;
  } else {
    old_field = new_field;
  }
}

template <clang_vx::Update Upgrade = clang_vx::Update::UPGRADE, typename T,
          typename U>
void assignSameEnum(T &old_field, U &new_field, std::string_view version) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    for (auto ls1 : magic_enum::enum_values<T>()) {
      if (!magic_enum::enum_cast<U>(magic_enum::enum_name(ls1)).has_value()) {
        if (old_field == ls1) {
          std::cout << "Error when upgrading to version " << version
                    << ". Enum " << magic_enum::enum_type_name<T>()
                    << "::" << magic_enum::enum_name(ls1)
                    << " is removed and was used.\n";
        } else {
          std::cout << "Info when upgrading to version " << version << ". Enum "
                    << magic_enum::enum_type_name<T>()
                    << "::" << magic_enum::enum_name(ls1)
                    << " is removed but was not used.\n";
        }
      }
    }
    for (auto ls2 : magic_enum::enum_values<U>()) {
      if (!magic_enum::enum_cast<T>(magic_enum::enum_name(ls2)).has_value()) {
        std::cout << "Info when upgrading to version " << version << ". Enum "
                  << magic_enum::enum_type_name<U>() << " have a new value "
                  << magic_enum::enum_name(ls2) << ".\n";
      }
    }
    new_field =
        magic_enum::enum_cast<U>(magic_enum::enum_name(old_field)).value();
  } else {
    for (auto ls1 : magic_enum::enum_values<U>()) {
      if (!magic_enum::enum_cast<T>(magic_enum::enum_name(ls1)).has_value()) {
        if (new_field == ls1) {
          std::cout << "Error when downgrading from version " << version
                    << ". Enum " << magic_enum::enum_type_name<U>()
                    << "::" << magic_enum::enum_name(ls1)
                    << " is removed and was used.\n";
        } else {
          std::cout << "Info when downgrading from version " << version
                    << ". Enum " << magic_enum::enum_type_name<U>()
                    << "::" << magic_enum::enum_name(ls1)
                    << " is removed but was not used.\n";
        }
      }
    }
    for (auto ls2 : magic_enum::enum_values<T>()) {
      if (!magic_enum::enum_cast<U>(magic_enum::enum_name(ls2)).has_value()) {
        std::cout << "Info when downgrading to version " << version << ". Enum "
                  << magic_enum::enum_type_name<T>() << " had an old value "
                  << magic_enum::enum_name(ls2) << ".\n";
      }
    }
    old_field =
        magic_enum::enum_cast<T>(magic_enum::enum_name(new_field)).value();
  }
}

} // namespace

#define ASSIGN_SAME_FIELD(FIELD)                                               \
  assignSameField<Upgrade>(prev.FIELD, next.FIELD)
#define ASSIGN_SAME_ENUM(FIELD)                                                \
  assignSameEnum<Upgrade>(prev.FIELD, next.FIELD, next_version)
#define NEW_FIELD(FIELD) newField<Upgrade>(STR(FIELD), next_version, next.FIELD)

namespace clang_update_v3_4 {

constexpr frozen::unordered_map<clang_v3_3::FormatStyle::LanguageStandard,
                                clang_v3_4::FormatStyle::LanguageStandard, 3>
    language_standard{{clang_v3_3::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v3_4::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v3_3::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v3_4::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v3_3::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v3_4::FormatStyle::LanguageStandard::LS_Auto}};

template <clang_vx::Update Upgrade>
void update(clang_v3_3::FormatStyle &prev, clang_v3_4::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v3_4::getPredefinedStyle(style, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "3.3";
  std::string_view next_version = "3.4";

  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  NEW_FIELD(PenaltyBreakComment);
  NEW_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  NEW_FIELD(PenaltyBreakFirstLessLess);
  NEW_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PointerBindsToType);
  ASSIGN_SAME_FIELD(DerivePointerBinding);
  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_SAME_ENUM(Standard);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  NEW_FIELD(NamespaceIndentation);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  NEW_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  NEW_FIELD(BreakConstructorInitializersBeforeComma);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  NEW_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  NEW_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AlignEscapedNewlinesLeft);
  NEW_FIELD(IndentWidth);
  NEW_FIELD(TabWidth);
  NEW_FIELD(ConstructorInitializerIndentWidth);
  NEW_FIELD(AlwaysBreakTemplateDeclarations);
  NEW_FIELD(AlwaysBreakBeforeMultilineStrings);
  NEW_FIELD(UseTab);
  NEW_FIELD(BreakBeforeBinaryOperators);
  NEW_FIELD(BreakBeforeTernaryOperators);
  NEW_FIELD(BreakBeforeBraces);
  NEW_FIELD(Cpp11BracedListStyle);
  NEW_FIELD(IndentFunctionDeclarationAfterType);
  NEW_FIELD(SpacesInParentheses);
  NEW_FIELD(SpacesInAngles);
  NEW_FIELD(SpaceInEmptyParentheses);
  NEW_FIELD(SpacesInCStyleCastParentheses);
  NEW_FIELD(SpaceAfterControlStatementKeyword);
  NEW_FIELD(SpaceBeforeAssignmentOperators);
  NEW_FIELD(ContinuationIndentWidth);
}

template void update<clang_vx::Update::UPGRADE>(clang_v3_3::FormatStyle &prev,
                                                clang_v3_4::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v3_3::FormatStyle &prev,
                                                  clang_v3_4::FormatStyle &next,
                                                  const std::string &style);

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
  newField("IncludeIsMainSourceRegex", "10",
           retval.IncludeStyle.IncludeIsMainSourceRegex);
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
  newField("SpaceBeforeSquareBrackets", "10", retval.SpaceBeforeSquareBrackets);
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

namespace clang_update_v11 {

constexpr frozen::unordered_map<clang_v10::FormatStyle::BracketAlignmentStyle,
                                clang_v11::FormatStyle::BracketAlignmentStyle,
                                3>
    bracket_all_alignment_style{
        {clang_v10::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v11::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v10::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v11::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v10::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v11::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<
    clang_v10::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v11::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v10::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v11::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v10::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v11::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v10::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v11::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<
    bool, clang_v11::FormatStyle::OperandAlignmentStyle, 2>
    operand_alignment_style{
        {false, clang_v11::FormatStyle::OperandAlignmentStyle::OAS_DontAlign},
        {true, clang_v11::FormatStyle::OperandAlignmentStyle::OAS_Align}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::ShortBlockStyle,
                                clang_v11::FormatStyle::ShortBlockStyle, 3>
    short_block_style{{clang_v10::FormatStyle::ShortBlockStyle::SBS_Never,
                       clang_v11::FormatStyle::ShortBlockStyle::SBS_Never},
                      {clang_v10::FormatStyle::ShortBlockStyle::SBS_Empty,
                       clang_v11::FormatStyle::ShortBlockStyle::SBS_Empty},
                      {clang_v10::FormatStyle::ShortBlockStyle::SBS_Always,
                       clang_v11::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::ShortFunctionStyle,
                                clang_v11::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v10::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v11::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v10::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v11::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v10::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v11::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v10::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v11::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v10::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v11::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::ShortIfStyle,
                                clang_v11::FormatStyle::ShortIfStyle, 3>
    short_if_style{{clang_v10::FormatStyle::ShortIfStyle::SIS_Never,
                    clang_v11::FormatStyle::ShortIfStyle::SIS_Never},
                   {clang_v10::FormatStyle::ShortIfStyle::SIS_WithoutElse,
                    clang_v11::FormatStyle::ShortIfStyle::SIS_WithoutElse},
                   {clang_v10::FormatStyle::ShortIfStyle::SIS_Always,
                    clang_v11::FormatStyle::ShortIfStyle::SIS_Always}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::ShortLambdaStyle,
                                clang_v11::FormatStyle::ShortLambdaStyle, 4>
    short_lambda_style{{clang_v10::FormatStyle::ShortLambdaStyle::SLS_None,
                        clang_v11::FormatStyle::ShortLambdaStyle::SLS_None},
                       {clang_v10::FormatStyle::ShortLambdaStyle::SLS_Empty,
                        clang_v11::FormatStyle::ShortLambdaStyle::SLS_Empty},
                       {clang_v10::FormatStyle::ShortLambdaStyle::SLS_Inline,
                        clang_v11::FormatStyle::ShortLambdaStyle::SLS_Inline},
                       {clang_v10::FormatStyle::ShortLambdaStyle::SLS_All,
                        clang_v11::FormatStyle::ShortLambdaStyle::SLS_All}};

constexpr frozen::unordered_map<
    clang_v10::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v11::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v10::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v11::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v10::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v11::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v10::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v11::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v11::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v10::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v11::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v10::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v11::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v10::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v11::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v10::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v11::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v10::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v11::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v10::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v11::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v10::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v11::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v10::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v11::FormatStyle::BreakTemplateDeclarationsStyle::
             BTDS_MultiLine},
        {clang_v10::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v11::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::BinaryOperatorStyle,
                                clang_v11::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v10::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v11::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v10::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v11::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v10::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v11::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::BraceBreakingStyle,
                                clang_v11::FormatStyle::BraceBreakingStyle, 9>
    brace_breaking_style{
        {clang_v10::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v11::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v10::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v11::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v10::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v11::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v10::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v11::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v10::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v11::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v10::FormatStyle::BraceBreakingStyle::BS_Whitesmiths,
         clang_v11::FormatStyle::BraceBreakingStyle::BS_Whitesmiths},
        {clang_v10::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v11::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v10::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v11::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v10::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v11::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v10::FormatStyle::BraceWrappingAfterControlStatementStyle,
    clang_v11::FormatStyle::BraceWrappingAfterControlStatementStyle, 3>
    brace_wrapping_after_control_statement_style{
        {clang_v10::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never,
         clang_v11::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never},
        {clang_v10::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine,
         clang_v11::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine},
        {clang_v10::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always,
         clang_v11::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always}};

constexpr frozen::unordered_map<
    clang_v10::FormatStyle::BreakConstructorInitializersStyle,
    clang_v11::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v10::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v11::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v10::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v11::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v10::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v11::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v10::FormatStyle::BreakInheritanceListStyle,
    clang_v11::FormatStyle::BreakInheritanceListStyle, 3>
    break_inheritance_list_style{
        {clang_v10::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v11::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v10::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v11::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v10::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v11::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon}};

constexpr frozen::unordered_map<clang_v10::IncludeStyle::IncludeBlocksStyle,
                                clang_v11::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v10::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v11::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v10::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v11::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v10::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v11::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v10::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v11::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v11::IncludeStyle::IncludeCategory{
        item.Regex, item.Priority, item.SortPriority});
  }
}

constexpr frozen::unordered_map<clang_v10::FormatStyle::PPDirectiveIndentStyle,
                                clang_v11::FormatStyle::PPDirectiveIndentStyle,
                                3>
    pp_directive_indent_style{
        {clang_v10::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v11::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v10::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v11::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash},
        {clang_v10::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash,
         clang_v11::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::JavaScriptQuoteStyle,
                                clang_v11::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v10::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v11::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v10::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v11::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v10::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v11::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::LanguageKind,
                                clang_v11::FormatStyle::LanguageKind, 9>
    language_king{{clang_v10::FormatStyle::LanguageKind::LK_None,
                   clang_v11::FormatStyle::LanguageKind::LK_None},
                  {clang_v10::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v11::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v10::FormatStyle::LanguageKind::LK_CSharp,
                   clang_v11::FormatStyle::LanguageKind::LK_CSharp},
                  {clang_v10::FormatStyle::LanguageKind::LK_Java,
                   clang_v11::FormatStyle::LanguageKind::LK_Java},
                  {clang_v10::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v11::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v10::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v11::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v10::FormatStyle::LanguageKind::LK_Proto,
                   clang_v11::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v10::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v11::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v10::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v11::FormatStyle::LanguageKind::LK_TextProto}};

constexpr frozen::unordered_map<
    clang_v10::FormatStyle::NamespaceIndentationKind,
    clang_v11::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v10::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v11::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v10::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v11::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v10::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v11::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::BinPackStyle,
                                clang_v11::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v10::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v11::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v10::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v11::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v10::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v11::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::PointerAlignmentStyle,
                                clang_v11::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v10::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v11::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v10::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v11::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v10::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v11::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

void assign(std::vector<clang_v10::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v11::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v11::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<
    clang_v10::FormatStyle::SpaceBeforeParensOptions,
    clang_v11::FormatStyle::SpaceBeforeParensOptions, 4>
    space_before_parens_options{
        {clang_v10::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v11::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v10::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v11::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v10::FormatStyle::SpaceBeforeParensOptions::
             SBPO_NonEmptyParentheses,
         clang_v11::FormatStyle::SpaceBeforeParensOptions::
             SBPO_NonEmptyParentheses},
        {clang_v10::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v11::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::LanguageStandard,
                                clang_v11::FormatStyle::LanguageStandard, 7>
    language_standard{{clang_v10::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v11::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v10::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v11::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v10::FormatStyle::LanguageStandard::LS_Cpp14,
                       clang_v11::FormatStyle::LanguageStandard::LS_Cpp14},
                      {clang_v10::FormatStyle::LanguageStandard::LS_Cpp17,
                       clang_v11::FormatStyle::LanguageStandard::LS_Cpp17},
                      {clang_v10::FormatStyle::LanguageStandard::LS_Cpp20,
                       clang_v11::FormatStyle::LanguageStandard::LS_Cpp20},
                      {clang_v10::FormatStyle::LanguageStandard::LS_Latest,
                       clang_v11::FormatStyle::LanguageStandard::LS_Latest},
                      {clang_v10::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v11::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v10::FormatStyle::UseTabStyle,
                                clang_v11::FormatStyle::UseTabStyle, 4>
    use_tab_style{
        {clang_v10::FormatStyle::UseTabStyle::UT_Never,
         clang_v11::FormatStyle::UseTabStyle::UT_Never},
        {clang_v10::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v11::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v10::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v11::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v10::FormatStyle::UseTabStyle::UT_Always,
         clang_v11::FormatStyle::UseTabStyle::UT_Always}};

clang_v11::FormatStyle update(clang_v10::FormatStyle &old,
                              const std::string &style) {
  clang_v11::FormatStyle retval;
  if (!clang_v11::getPredefinedStyle(
          style, clang_v11::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignConsecutiveMacros = old.AlignConsecutiveMacros;
  retval.AlignConsecutiveAssignments = old.AlignConsecutiveAssignments;
  newField("AlignConsecutiveBitFields", "11", retval.AlignConsecutiveBitFields);
  retval.AlignConsecutiveDeclarations = old.AlignConsecutiveDeclarations;
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  retval.AlignOperands = operand_alignment_style.at(old.AlignOperands);
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AllowAllArgumentsOnNextLine = old.AllowAllArgumentsOnNextLine;
  retval.AllowAllConstructorInitializersOnNextLine =
      old.AllowAllConstructorInitializersOnNextLine;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  newField("AllowShortEnumsOnASingleLine", "11",
           retval.AllowShortEnumsOnASingleLine);
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
  newField("InsertTrailingCommas", "11", retval.InsertTrailingCommas);
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
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
  newField("BraceWrapping.BeforeLambdaBody", "11",
           retval.BraceWrapping.BeforeLambdaBody);
  newField("BraceWrapping.BeforeWhile", "11", retval.BraceWrapping.BeforeWhile);
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
  retval.DeriveLineEnding = old.DeriveLineEnding;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  retval.TypenameMacros = old.TypenameMacros;
  retval.StatementMacros = old.StatementMacros;
  retval.NamespaceMacros = old.NamespaceMacros;
  newField("WhitespaceSensitiveMacros", "11", retval.WhitespaceSensitiveMacros);
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  newField("IncludeIsMainSourceRegex", "11",
           retval.IncludeStyle.IncludeIsMainSourceRegex);
  retval.IndentCaseLabels = old.IndentCaseLabels;
  newField("IndentCaseBlocks", "11", retval.IndentCaseBlocks);
  retval.IndentGotoLabels = old.IndentGotoLabels;
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  newField("IndentExternBlock", "11", retval.IndentExternBlock);
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
  newField("ObjCBreakBeforeNestedBlockParam", "11",
           retval.ObjCBreakBeforeNestedBlockParam);
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
  improveField("SpaceBeforeParens", "ControlStatementsExceptForEachMacros",
               "11");
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
  retval.SpaceInEmptyBlock = old.SpaceInEmptyBlock;
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = old.SpacesInAngles;
  retval.SpacesInConditionalStatement = old.SpacesInConditionalStatement;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.SpaceBeforeSquareBrackets = old.SpaceBeforeSquareBrackets;
  retval.Standard = language_standard.at(old.Standard);
  retval.TabWidth = old.TabWidth;
  retval.UseCRLF = old.UseCRLF;
  retval.UseTab = use_tab_style.at(old.UseTab);
  improveField("UseTab", "AlignWithSpaces", "11");

  return retval;
}

} // namespace clang_update_v11

namespace clang_update_v12 {

constexpr frozen::unordered_map<clang_v11::FormatStyle::BracketAlignmentStyle,
                                clang_v12::FormatStyle::BracketAlignmentStyle,
                                3>
    bracket_all_alignment_style{
        {clang_v11::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v12::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v11::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v12::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v11::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v12::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<
    bool, clang_v12::FormatStyle::AlignConsecutiveStyle, 2>
    align_consecutive_style{
        {false, clang_v12::FormatStyle::AlignConsecutiveStyle::ACS_None},
        {true, clang_v12::FormatStyle::AlignConsecutiveStyle::ACS_Consecutive}};

constexpr frozen::unordered_map<
    clang_v11::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v12::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v11::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v12::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v11::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v12::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v11::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v12::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::OperandAlignmentStyle,
                                clang_v12::FormatStyle::OperandAlignmentStyle,
                                3>
    operand_alignment_style{
        {clang_v11::FormatStyle::OperandAlignmentStyle::OAS_DontAlign,
         clang_v12::FormatStyle::OperandAlignmentStyle::OAS_DontAlign},
        {clang_v11::FormatStyle::OperandAlignmentStyle::OAS_Align,
         clang_v12::FormatStyle::OperandAlignmentStyle::OAS_Align},
        {clang_v11::FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator,
         clang_v12::FormatStyle::OperandAlignmentStyle::
             OAS_AlignAfterOperator}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::ShortBlockStyle,
                                clang_v12::FormatStyle::ShortBlockStyle, 3>
    short_block_style{{clang_v11::FormatStyle::ShortBlockStyle::SBS_Never,
                       clang_v12::FormatStyle::ShortBlockStyle::SBS_Never},
                      {clang_v11::FormatStyle::ShortBlockStyle::SBS_Empty,
                       clang_v12::FormatStyle::ShortBlockStyle::SBS_Empty},
                      {clang_v11::FormatStyle::ShortBlockStyle::SBS_Always,
                       clang_v12::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::ShortFunctionStyle,
                                clang_v12::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v11::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v12::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v11::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v12::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v11::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v12::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v11::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v12::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v11::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v12::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::ShortIfStyle,
                                clang_v12::FormatStyle::ShortIfStyle, 3>
    short_if_style{{clang_v11::FormatStyle::ShortIfStyle::SIS_Never,
                    clang_v12::FormatStyle::ShortIfStyle::SIS_Never},
                   {clang_v11::FormatStyle::ShortIfStyle::SIS_WithoutElse,
                    clang_v12::FormatStyle::ShortIfStyle::SIS_WithoutElse},
                   {clang_v11::FormatStyle::ShortIfStyle::SIS_Always,
                    clang_v12::FormatStyle::ShortIfStyle::SIS_Always}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::ShortLambdaStyle,
                                clang_v12::FormatStyle::ShortLambdaStyle, 4>
    short_lambda_style{{clang_v11::FormatStyle::ShortLambdaStyle::SLS_None,
                        clang_v12::FormatStyle::ShortLambdaStyle::SLS_None},
                       {clang_v11::FormatStyle::ShortLambdaStyle::SLS_Empty,
                        clang_v12::FormatStyle::ShortLambdaStyle::SLS_Empty},
                       {clang_v11::FormatStyle::ShortLambdaStyle::SLS_Inline,
                        clang_v12::FormatStyle::ShortLambdaStyle::SLS_Inline},
                       {clang_v11::FormatStyle::ShortLambdaStyle::SLS_All,
                        clang_v12::FormatStyle::ShortLambdaStyle::SLS_All}};

constexpr frozen::unordered_map<
    clang_v11::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v12::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v11::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v12::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v11::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v12::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v11::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v12::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v12::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v11::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v12::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v11::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v12::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v11::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v12::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v11::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v12::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v11::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v12::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v11::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v12::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v11::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v12::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v11::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v12::FormatStyle::BreakTemplateDeclarationsStyle::
             BTDS_MultiLine},
        {clang_v11::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v12::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::TrailingCommaStyle,
                                clang_v12::FormatStyle::TrailingCommaStyle, 2>
    trailing_comma_style{
        {clang_v11::FormatStyle::TrailingCommaStyle::TCS_None,
         clang_v12::FormatStyle::TrailingCommaStyle::TCS_None},
        {clang_v11::FormatStyle::TrailingCommaStyle::TCS_Wrapped,
         clang_v12::FormatStyle::TrailingCommaStyle::TCS_Wrapped}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::BinaryOperatorStyle,
                                clang_v12::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v11::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v12::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v11::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v12::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v11::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v12::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::BraceBreakingStyle,
                                clang_v12::FormatStyle::BraceBreakingStyle, 9>
    brace_breaking_style{
        {clang_v11::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v12::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v11::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v12::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v11::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v12::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v11::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v12::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v11::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v12::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v11::FormatStyle::BraceBreakingStyle::BS_Whitesmiths,
         clang_v12::FormatStyle::BraceBreakingStyle::BS_Whitesmiths},
        {clang_v11::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v12::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v11::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v12::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v11::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v12::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v11::FormatStyle::BraceWrappingAfterControlStatementStyle,
    clang_v12::FormatStyle::BraceWrappingAfterControlStatementStyle, 3>
    brace_wrapping_after_control_statement_style{
        {clang_v11::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never,
         clang_v12::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never},
        {clang_v11::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine,
         clang_v12::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine},
        {clang_v11::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always,
         clang_v12::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always}};

constexpr frozen::unordered_map<
    clang_v11::FormatStyle::BreakConstructorInitializersStyle,
    clang_v12::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v11::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v12::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v11::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v12::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v11::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v12::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v11::FormatStyle::BreakInheritanceListStyle,
    clang_v12::FormatStyle::BreakInheritanceListStyle, 3>
    break_inheritance_list_style{
        {clang_v11::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v12::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v11::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v12::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v11::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v12::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon}};

constexpr frozen::unordered_map<clang_v11::IncludeStyle::IncludeBlocksStyle,
                                clang_v12::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v11::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v12::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v11::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v12::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v11::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v12::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v11::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v12::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v12::IncludeStyle::IncludeCategory{
        item.Regex, item.Priority, item.SortPriority, true});
  }
}

constexpr frozen::unordered_map<clang_v11::FormatStyle::PPDirectiveIndentStyle,
                                clang_v12::FormatStyle::PPDirectiveIndentStyle,
                                3>
    pp_directive_indent_style{
        {clang_v11::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v12::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v11::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v12::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash},
        {clang_v11::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash,
         clang_v12::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::IndentExternBlockStyle,
                                clang_v12::FormatStyle::IndentExternBlockStyle,
                                3>
    indent_extern_block_style{
        {clang_v11::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock,
         clang_v12::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock},
        {clang_v11::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent,
         clang_v12::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent},
        {clang_v11::FormatStyle::IndentExternBlockStyle::IEBS_Indent,
         clang_v12::FormatStyle::IndentExternBlockStyle::IEBS_Indent}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::JavaScriptQuoteStyle,
                                clang_v12::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v11::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v12::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v11::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v12::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v11::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v12::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::LanguageKind,
                                clang_v12::FormatStyle::LanguageKind, 9>
    language_king{{clang_v11::FormatStyle::LanguageKind::LK_None,
                   clang_v12::FormatStyle::LanguageKind::LK_None},
                  {clang_v11::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v12::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v11::FormatStyle::LanguageKind::LK_CSharp,
                   clang_v12::FormatStyle::LanguageKind::LK_CSharp},
                  {clang_v11::FormatStyle::LanguageKind::LK_Java,
                   clang_v12::FormatStyle::LanguageKind::LK_Java},
                  {clang_v11::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v12::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v11::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v12::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v11::FormatStyle::LanguageKind::LK_Proto,
                   clang_v12::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v11::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v12::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v11::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v12::FormatStyle::LanguageKind::LK_TextProto}};

constexpr frozen::unordered_map<
    clang_v11::FormatStyle::NamespaceIndentationKind,
    clang_v12::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v11::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v12::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v11::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v12::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v11::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v12::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::BinPackStyle,
                                clang_v12::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v11::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v12::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v11::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v12::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v11::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v12::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::PointerAlignmentStyle,
                                clang_v12::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v11::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v12::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v11::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v12::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v11::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v12::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

void assign(std::vector<clang_v11::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v12::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v12::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<
    clang_v11::FormatStyle::SpaceBeforeParensOptions,
    clang_v12::FormatStyle::SpaceBeforeParensOptions, 5>
    space_before_parens_options{
        {clang_v11::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v12::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v11::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v12::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v11::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatementsExceptForEachMacros,
         clang_v12::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatementsExceptForEachMacros},
        {clang_v11::FormatStyle::SpaceBeforeParensOptions::
             SBPO_NonEmptyParentheses,
         clang_v12::FormatStyle::SpaceBeforeParensOptions::
             SBPO_NonEmptyParentheses},
        {clang_v11::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v12::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::LanguageStandard,
                                clang_v12::FormatStyle::LanguageStandard, 7>
    language_standard{{clang_v11::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v12::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v11::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v12::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v11::FormatStyle::LanguageStandard::LS_Cpp14,
                       clang_v12::FormatStyle::LanguageStandard::LS_Cpp14},
                      {clang_v11::FormatStyle::LanguageStandard::LS_Cpp17,
                       clang_v12::FormatStyle::LanguageStandard::LS_Cpp17},
                      {clang_v11::FormatStyle::LanguageStandard::LS_Cpp20,
                       clang_v12::FormatStyle::LanguageStandard::LS_Cpp20},
                      {clang_v11::FormatStyle::LanguageStandard::LS_Latest,
                       clang_v12::FormatStyle::LanguageStandard::LS_Latest},
                      {clang_v11::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v12::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v11::FormatStyle::UseTabStyle,
                                clang_v12::FormatStyle::UseTabStyle, 5>
    use_tab_style{
        {clang_v11::FormatStyle::UseTabStyle::UT_Never,
         clang_v12::FormatStyle::UseTabStyle::UT_Never},
        {clang_v11::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v12::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v11::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v12::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v11::FormatStyle::UseTabStyle::UT_AlignWithSpaces,
         clang_v12::FormatStyle::UseTabStyle::UT_AlignWithSpaces},
        {clang_v11::FormatStyle::UseTabStyle::UT_Always,
         clang_v12::FormatStyle::UseTabStyle::UT_Always}};

clang_v12::FormatStyle update(clang_v11::FormatStyle &old,
                              const std::string &style) {
  clang_v12::FormatStyle retval;
  if (!clang_v12::getPredefinedStyle(
          style, clang_v12::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignConsecutiveMacros =
      align_consecutive_style.at(old.AlignConsecutiveMacros);
  improveField("AlignConsecutiveMacros", "AcrossEmptyLines", "12");
  improveField("AlignConsecutiveMacros", "AcrossComments", "12");
  improveField("AlignConsecutiveMacros", "AcrossEmptyLinesAndComments", "12");
  retval.AlignConsecutiveAssignments =
      align_consecutive_style.at(old.AlignConsecutiveAssignments);
  improveField("AlignConsecutiveAssignments", "AcrossEmptyLines", "12");
  improveField("AlignConsecutiveAssignments", "AcrossComments", "12");
  improveField("AlignConsecutiveAssignments", "AcrossEmptyLinesAndComments",
               "12");
  retval.AlignConsecutiveBitFields =
      align_consecutive_style.at(old.AlignConsecutiveBitFields);
  improveField("AlignConsecutiveBitFields", "AcrossEmptyLines", "12");
  improveField("AlignConsecutiveBitFields", "AcrossComments", "12");
  improveField("AlignConsecutiveBitFields", "AcrossEmptyLinesAndComments",
               "12");
  retval.AlignConsecutiveDeclarations =
      align_consecutive_style.at(old.AlignConsecutiveDeclarations);
  improveField("AlignConsecutiveDeclarations", "AcrossEmptyLines", "12");
  improveField("AlignConsecutiveDeclarations", "AcrossComments", "12");
  improveField("AlignConsecutiveDeclarations", "AcrossEmptyLinesAndComments",
               "12");
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  retval.AlignOperands = operand_alignment_style.at(old.AlignOperands);
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AllowAllArgumentsOnNextLine = old.AllowAllArgumentsOnNextLine;
  retval.AllowAllConstructorInitializersOnNextLine =
      old.AllowAllConstructorInitializersOnNextLine;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortEnumsOnASingleLine = old.AllowShortEnumsOnASingleLine;
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
  newField("AttributeMacros", "12", retval.AttributeMacros);
  retval.BinPackArguments = old.BinPackArguments;
  retval.InsertTrailingCommas =
      trailing_comma_style.at(old.InsertTrailingCommas);
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
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
  retval.BraceWrapping.BeforeLambdaBody = old.BraceWrapping.BeforeLambdaBody;
  retval.BraceWrapping.BeforeWhile = old.BraceWrapping.BeforeWhile;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  newField("BreakBeforeConceptDeclarations", "12",
           retval.BreakBeforeConceptDeclarations);
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
  retval.DeriveLineEnding = old.DeriveLineEnding;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  newField("EmptyLineBeforeAccessModifier", "12",
           retval.EmptyLineBeforeAccessModifier);
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  retval.TypenameMacros = old.TypenameMacros;
  retval.StatementMacros = old.StatementMacros;
  retval.NamespaceMacros = old.NamespaceMacros;
  retval.WhitespaceSensitiveMacros = old.WhitespaceSensitiveMacros;
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  retval.IncludeStyle.IncludeIsMainSourceRegex =
      old.IncludeStyle.IncludeIsMainSourceRegex;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentCaseBlocks = old.IndentCaseBlocks;
  retval.IndentGotoLabels = old.IndentGotoLabels;
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  retval.IndentExternBlock =
      indent_extern_block_style.at(old.IndentExternBlock);
  newField("IndentRequires", "12", retval.IndentRequires);
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
  retval.ObjCBreakBeforeNestedBlockParam = old.ObjCBreakBeforeNestedBlockParam;
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
  newField("PenaltyIndentedWhitespace", "12", retval.PenaltyIndentedWhitespace);
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  assign(old.RawStringFormats, retval.RawStringFormats);
  retval.ReflowComments = old.ReflowComments;
  retval.SortIncludes = old.SortIncludes;
  newField("SortJavaStaticImport", "12", retval.SortJavaStaticImport);
  retval.SortUsingDeclarations = old.SortUsingDeclarations;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterLogicalNot = old.SpaceAfterLogicalNot;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  newField("SpaceAroundPointerQualifiers", "12",
           retval.SpaceAroundPointerQualifiers);
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  newField("SpaceBeforeCaseColon", "12", retval.SpaceBeforeCaseColon);
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
  retval.SpaceInEmptyBlock = old.SpaceInEmptyBlock;
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = old.SpacesInAngles;
  retval.SpacesInConditionalStatement = old.SpacesInConditionalStatement;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.SpaceBeforeSquareBrackets = old.SpaceBeforeSquareBrackets;
  newField("BitFieldColonSpacing", "12", retval.BitFieldColonSpacing);
  retval.Standard = language_standard.at(old.Standard);
  newField("StatementAttributeLikeMacros", "12",
           retval.StatementAttributeLikeMacros);
  retval.TabWidth = old.TabWidth;
  retval.UseCRLF = old.UseCRLF;
  retval.UseTab = use_tab_style.at(old.UseTab);

  return retval;
}

} // namespace clang_update_v12

namespace clang_update_v13 {

constexpr frozen::unordered_map<clang_v12::FormatStyle::BracketAlignmentStyle,
                                clang_v13::FormatStyle::BracketAlignmentStyle,
                                3>
    bracket_all_alignment_style{
        {clang_v12::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v13::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v12::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v13::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v12::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v13::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::AlignConsecutiveStyle,
                                clang_v13::FormatStyle::AlignConsecutiveStyle,
                                5>
    align_consecutive_style{
        {clang_v12::FormatStyle::AlignConsecutiveStyle::ACS_None,
         clang_v13::FormatStyle::AlignConsecutiveStyle::ACS_None},
        {clang_v12::FormatStyle::AlignConsecutiveStyle::ACS_Consecutive,
         clang_v13::FormatStyle::AlignConsecutiveStyle::ACS_Consecutive},
        {clang_v12::FormatStyle::AlignConsecutiveStyle::ACS_AcrossEmptyLines,
         clang_v13::FormatStyle::AlignConsecutiveStyle::ACS_AcrossEmptyLines},
        {clang_v12::FormatStyle::AlignConsecutiveStyle::ACS_AcrossComments,
         clang_v13::FormatStyle::AlignConsecutiveStyle::ACS_AcrossComments},
        {clang_v12::FormatStyle::AlignConsecutiveStyle::
             ACS_AcrossEmptyLinesAndComments,
         clang_v13::FormatStyle::AlignConsecutiveStyle::
             ACS_AcrossEmptyLinesAndComments}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v13::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v12::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v13::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v12::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v13::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v12::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v13::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::OperandAlignmentStyle,
                                clang_v13::FormatStyle::OperandAlignmentStyle,
                                3>
    operand_alignment_style{
        {clang_v12::FormatStyle::OperandAlignmentStyle::OAS_DontAlign,
         clang_v13::FormatStyle::OperandAlignmentStyle::OAS_DontAlign},
        {clang_v12::FormatStyle::OperandAlignmentStyle::OAS_Align,
         clang_v13::FormatStyle::OperandAlignmentStyle::OAS_Align},
        {clang_v12::FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator,
         clang_v13::FormatStyle::OperandAlignmentStyle::
             OAS_AlignAfterOperator}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::ShortBlockStyle,
                                clang_v13::FormatStyle::ShortBlockStyle, 3>
    short_block_style{{clang_v12::FormatStyle::ShortBlockStyle::SBS_Never,
                       clang_v13::FormatStyle::ShortBlockStyle::SBS_Never},
                      {clang_v12::FormatStyle::ShortBlockStyle::SBS_Empty,
                       clang_v13::FormatStyle::ShortBlockStyle::SBS_Empty},
                      {clang_v12::FormatStyle::ShortBlockStyle::SBS_Always,
                       clang_v13::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::ShortFunctionStyle,
                                clang_v13::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v12::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v13::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v12::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v13::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v12::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v13::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v12::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v13::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v12::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v13::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::ShortIfStyle,
                                clang_v13::FormatStyle::ShortIfStyle, 3>
    short_if_style{{clang_v12::FormatStyle::ShortIfStyle::SIS_Never,
                    clang_v13::FormatStyle::ShortIfStyle::SIS_Never},
                   {clang_v12::FormatStyle::ShortIfStyle::SIS_WithoutElse,
                    clang_v13::FormatStyle::ShortIfStyle::SIS_WithoutElse},
                   {clang_v12::FormatStyle::ShortIfStyle::SIS_Always,
                    clang_v13::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::ShortLambdaStyle,
                                clang_v13::FormatStyle::ShortLambdaStyle, 4>
    short_lambda_style{{clang_v12::FormatStyle::ShortLambdaStyle::SLS_None,
                        clang_v13::FormatStyle::ShortLambdaStyle::SLS_None},
                       {clang_v12::FormatStyle::ShortLambdaStyle::SLS_Empty,
                        clang_v13::FormatStyle::ShortLambdaStyle::SLS_Empty},
                       {clang_v12::FormatStyle::ShortLambdaStyle::SLS_Inline,
                        clang_v13::FormatStyle::ShortLambdaStyle::SLS_Inline},
                       {clang_v12::FormatStyle::ShortLambdaStyle::SLS_All,
                        clang_v13::FormatStyle::ShortLambdaStyle::SLS_All}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v13::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v12::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v13::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v12::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v13::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v12::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v13::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v13::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v12::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v13::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v12::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v13::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v12::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v13::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v12::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v13::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v12::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v13::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v13::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v12::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v13::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v12::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v13::FormatStyle::BreakTemplateDeclarationsStyle::
             BTDS_MultiLine},
        {clang_v12::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v13::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::TrailingCommaStyle,
                                clang_v13::FormatStyle::TrailingCommaStyle, 2>
    trailing_comma_style{
        {clang_v12::FormatStyle::TrailingCommaStyle::TCS_None,
         clang_v13::FormatStyle::TrailingCommaStyle::TCS_None},
        {clang_v12::FormatStyle::TrailingCommaStyle::TCS_Wrapped,
         clang_v13::FormatStyle::TrailingCommaStyle::TCS_Wrapped}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::BinaryOperatorStyle,
                                clang_v13::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v12::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v13::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v12::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v13::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v12::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v13::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::BraceBreakingStyle,
                                clang_v13::FormatStyle::BraceBreakingStyle, 9>
    brace_breaking_style{
        {clang_v12::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v13::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v12::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v13::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v12::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v13::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v12::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v13::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v12::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v13::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v12::FormatStyle::BraceBreakingStyle::BS_Whitesmiths,
         clang_v13::FormatStyle::BraceBreakingStyle::BS_Whitesmiths},
        {clang_v12::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v13::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v12::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v13::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v12::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v13::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::BraceWrappingAfterControlStatementStyle,
    clang_v13::FormatStyle::BraceWrappingAfterControlStatementStyle, 3>
    brace_wrapping_after_control_statement_style{
        {clang_v12::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never,
         clang_v13::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never},
        {clang_v12::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine,
         clang_v13::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine},
        {clang_v12::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always,
         clang_v13::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::BreakConstructorInitializersStyle,
    clang_v13::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v12::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v13::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v12::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v13::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v12::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v13::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::BreakInheritanceListStyle,
    clang_v13::FormatStyle::BreakInheritanceListStyle, 3>
    break_inheritance_list_style{
        {clang_v12::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v13::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v12::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v13::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v12::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v13::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::EmptyLineBeforeAccessModifierStyle,
    clang_v13::FormatStyle::EmptyLineBeforeAccessModifierStyle, 4>
    empty_line_before_access_modifier_style{
        {clang_v12::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never,
         clang_v13::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never},
        {clang_v12::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave,
         clang_v13::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave},
        {clang_v12::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock,
         clang_v13::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock},
        {clang_v12::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always,
         clang_v13::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always}};

constexpr frozen::unordered_map<clang_v12::IncludeStyle::IncludeBlocksStyle,
                                clang_v13::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v12::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v13::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v12::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v13::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v12::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v13::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v12::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v13::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v13::IncludeStyle::IncludeCategory{
        item.Regex, item.Priority, item.SortPriority, true});
  }
}

constexpr frozen::unordered_map<clang_v12::FormatStyle::PPDirectiveIndentStyle,
                                clang_v13::FormatStyle::PPDirectiveIndentStyle,
                                3>
    pp_directive_indent_style{
        {clang_v12::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v13::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v12::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v13::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash},
        {clang_v12::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash,
         clang_v13::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::IndentExternBlockStyle,
                                clang_v13::FormatStyle::IndentExternBlockStyle,
                                3>
    indent_extern_block_style{
        {clang_v12::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock,
         clang_v13::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock},
        {clang_v12::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent,
         clang_v13::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent},
        {clang_v12::FormatStyle::IndentExternBlockStyle::IEBS_Indent,
         clang_v13::FormatStyle::IndentExternBlockStyle::IEBS_Indent}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::JavaScriptQuoteStyle,
                                clang_v13::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v12::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v13::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v12::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v13::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v12::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v13::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::LanguageKind,
                                clang_v13::FormatStyle::LanguageKind, 9>
    language_king{{clang_v12::FormatStyle::LanguageKind::LK_None,
                   clang_v13::FormatStyle::LanguageKind::LK_None},
                  {clang_v12::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v13::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v12::FormatStyle::LanguageKind::LK_CSharp,
                   clang_v13::FormatStyle::LanguageKind::LK_CSharp},
                  {clang_v12::FormatStyle::LanguageKind::LK_Java,
                   clang_v13::FormatStyle::LanguageKind::LK_Java},
                  {clang_v12::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v13::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v12::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v13::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v12::FormatStyle::LanguageKind::LK_Proto,
                   clang_v13::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v12::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v13::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v12::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v13::FormatStyle::LanguageKind::LK_TextProto}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::NamespaceIndentationKind,
    clang_v13::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v12::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v13::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v12::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v13::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v12::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v13::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::BinPackStyle,
                                clang_v13::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v12::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v13::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v12::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v13::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v12::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v13::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::PointerAlignmentStyle,
                                clang_v13::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v12::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v13::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v12::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v13::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v12::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v13::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

void assign(std::vector<clang_v12::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v13::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v13::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<bool,
                                clang_v13::FormatStyle::SortIncludesOptions, 2>
    sort_includes_options{
        {false, clang_v13::FormatStyle::SortIncludesOptions::SI_Never},
        {true,
         clang_v13::FormatStyle::SortIncludesOptions::SI_CaseInsensitive}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::SortJavaStaticImportOptions,
    clang_v13::FormatStyle::SortJavaStaticImportOptions, 2>
    sort_java_static_import_options{
        {clang_v12::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before,
         clang_v13::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before},
        {clang_v12::FormatStyle::SortJavaStaticImportOptions::SJSIO_After,
         clang_v13::FormatStyle::SortJavaStaticImportOptions::SJSIO_After}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::SpaceAroundPointerQualifiersStyle,
    clang_v13::FormatStyle::SpaceAroundPointerQualifiersStyle, 4>
    space_around_pointer_qualifiers_style{
        {clang_v12::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default,
         clang_v13::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default},
        {clang_v12::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Before,
         clang_v13::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Before},
        {clang_v12::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After,
         clang_v13::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After},
        {clang_v12::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both,
         clang_v13::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::SpaceBeforeParensOptions,
    clang_v13::FormatStyle::SpaceBeforeParensOptions, 5>
    space_before_parens_options{
        {clang_v12::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v13::FormatStyle::SpaceBeforeParensOptions::SBPO_Never},
        {clang_v12::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v13::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements},
        {clang_v12::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatementsExceptForEachMacros,
         clang_v13::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatementsExceptControlMacros},
        {clang_v12::FormatStyle::SpaceBeforeParensOptions::
             SBPO_NonEmptyParentheses,
         clang_v13::FormatStyle::SpaceBeforeParensOptions::
             SBPO_NonEmptyParentheses},
        {clang_v12::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v13::FormatStyle::SpaceBeforeParensOptions::SBPO_Always}};

constexpr frozen::unordered_map<bool,
                                clang_v13::FormatStyle::SpacesInAnglesStyle, 2>
    spaces_in_angles_style{
        {false, clang_v13::FormatStyle::SpacesInAnglesStyle::SIAS_Never},
        {true, clang_v13::FormatStyle::SpacesInAnglesStyle::SIAS_Always}};

constexpr frozen::unordered_map<
    clang_v12::FormatStyle::BitFieldColonSpacingStyle,
    clang_v13::FormatStyle::BitFieldColonSpacingStyle, 4>
    bite_field_colon_spacing_style{
        {clang_v12::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both,
         clang_v13::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both},
        {clang_v12::FormatStyle::BitFieldColonSpacingStyle::BFCS_None,
         clang_v13::FormatStyle::BitFieldColonSpacingStyle::BFCS_None},
        {clang_v12::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before,
         clang_v13::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before},
        {clang_v12::FormatStyle::BitFieldColonSpacingStyle::BFCS_After,
         clang_v13::FormatStyle::BitFieldColonSpacingStyle::BFCS_After}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::LanguageStandard,
                                clang_v13::FormatStyle::LanguageStandard, 7>
    language_standard{{clang_v12::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v13::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v12::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v13::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v12::FormatStyle::LanguageStandard::LS_Cpp14,
                       clang_v13::FormatStyle::LanguageStandard::LS_Cpp14},
                      {clang_v12::FormatStyle::LanguageStandard::LS_Cpp17,
                       clang_v13::FormatStyle::LanguageStandard::LS_Cpp17},
                      {clang_v12::FormatStyle::LanguageStandard::LS_Cpp20,
                       clang_v13::FormatStyle::LanguageStandard::LS_Cpp20},
                      {clang_v12::FormatStyle::LanguageStandard::LS_Latest,
                       clang_v13::FormatStyle::LanguageStandard::LS_Latest},
                      {clang_v12::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v13::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v12::FormatStyle::UseTabStyle,
                                clang_v13::FormatStyle::UseTabStyle, 5>
    use_tab_style{
        {clang_v12::FormatStyle::UseTabStyle::UT_Never,
         clang_v13::FormatStyle::UseTabStyle::UT_Never},
        {clang_v12::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v13::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v12::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v13::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v12::FormatStyle::UseTabStyle::UT_AlignWithSpaces,
         clang_v13::FormatStyle::UseTabStyle::UT_AlignWithSpaces},
        {clang_v12::FormatStyle::UseTabStyle::UT_Always,
         clang_v13::FormatStyle::UseTabStyle::UT_Always}};

clang_v13::FormatStyle update(clang_v12::FormatStyle &old,
                              const std::string &style) {
  clang_v13::FormatStyle retval;
  if (!clang_v13::getPredefinedStyle(
          style, clang_v13::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  newField("InheritsParentConfig", "13", retval.InheritsParentConfig);
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  newField("AlignArrayOfStructures", "13", retval.AlignArrayOfStructures);
  retval.AlignConsecutiveMacros =
      align_consecutive_style.at(old.AlignConsecutiveMacros);
  retval.AlignConsecutiveAssignments =
      align_consecutive_style.at(old.AlignConsecutiveAssignments);
  retval.AlignConsecutiveBitFields =
      align_consecutive_style.at(old.AlignConsecutiveBitFields);
  retval.AlignConsecutiveDeclarations =
      align_consecutive_style.at(old.AlignConsecutiveDeclarations);
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  retval.AlignOperands = operand_alignment_style.at(old.AlignOperands);
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AllowAllArgumentsOnNextLine = old.AllowAllArgumentsOnNextLine;
  retval.AllowAllConstructorInitializersOnNextLine =
      old.AllowAllConstructorInitializersOnNextLine;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortEnumsOnASingleLine = old.AllowShortEnumsOnASingleLine;
  retval.AllowShortBlocksOnASingleLine =
      short_block_style.at(old.AllowShortBlocksOnASingleLine);
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  retval.AllowShortFunctionsOnASingleLine =
      short_function_style.at(old.AllowShortFunctionsOnASingleLine);
  retval.AllowShortIfStatementsOnASingleLine =
      short_if_style.at(old.AllowShortIfStatementsOnASingleLine);
  improveField("AllowShortIfStatementsOnASingleLine", "AllIfsAndElse", "13");
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
  retval.AttributeMacros = old.AttributeMacros;
  retval.BinPackArguments = old.BinPackArguments;
  retval.InsertTrailingCommas =
      trailing_comma_style.at(old.InsertTrailingCommas);
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
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
  retval.BraceWrapping.BeforeLambdaBody = old.BraceWrapping.BeforeLambdaBody;
  retval.BraceWrapping.BeforeWhile = old.BraceWrapping.BeforeWhile;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakBeforeConceptDeclarations = old.BreakBeforeConceptDeclarations;
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializers =
      break_constructor_initializers_style.at(old.BreakConstructorInitializers);
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.BreakInheritanceList =
      break_inheritance_list_style.at(old.BreakInheritanceList);
  improveField("BreakInheritanceList", "AfterComma", "13");
  retval.CompactNamespaces = old.CompactNamespaces;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.DeriveLineEnding = old.DeriveLineEnding;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  newField("EmptyLineAfterAccessModifier", "13",
           retval.EmptyLineAfterAccessModifier);
  retval.EmptyLineBeforeAccessModifier =
      empty_line_before_access_modifier_style.at(
          old.EmptyLineBeforeAccessModifier);
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  newField("IfMacros", "13", retval.IfMacros);
  retval.TypenameMacros = old.TypenameMacros;
  retval.StatementMacros = old.StatementMacros;
  retval.NamespaceMacros = old.NamespaceMacros;
  retval.WhitespaceSensitiveMacros = old.WhitespaceSensitiveMacros;
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  retval.IncludeStyle.IncludeIsMainSourceRegex =
      old.IncludeStyle.IncludeIsMainSourceRegex;
  newField("IndentAccessModifiers", "13", retval.IndentAccessModifiers);
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentCaseBlocks = old.IndentCaseBlocks;
  retval.IndentGotoLabels = old.IndentGotoLabels;
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  retval.IndentExternBlock =
      indent_extern_block_style.at(old.IndentExternBlock);
  retval.IndentRequires = old.IndentRequires;
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.JavaImportGroups = old.JavaImportGroups;
  retval.JavaScriptQuotes = java_script_quote_style.at(old.JavaScriptQuotes);
  retval.JavaScriptWrapImports = old.JavaScriptWrapImports;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.Language = language_king.at(old.Language);
  improveField("Language", "Json", "13");
  newField("LambdaBodyIndentation", "13", retval.LambdaBodyIndentation);
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.ObjCBinPackProtocolList =
      bin_pack_style.at(old.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCBreakBeforeNestedBlockParam = old.ObjCBreakBeforeNestedBlockParam;
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
  retval.PenaltyIndentedWhitespace = old.PenaltyIndentedWhitespace;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  newField("PPIndentWidth", "13", retval.PPIndentWidth);
  assign(old.RawStringFormats, retval.RawStringFormats);
  newField("ReferenceAlignment", "13", retval.ReferenceAlignment);
  retval.ReflowComments = old.ReflowComments;
  newField("ShortNamespaceLines", "13", retval.ShortNamespaceLines);
  retval.SortIncludes = sort_includes_options.at(old.SortIncludes);
  retval.SortJavaStaticImport =
      sort_java_static_import_options.at(old.SortJavaStaticImport);
  retval.SortUsingDeclarations = old.SortUsingDeclarations;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterLogicalNot = old.SpaceAfterLogicalNot;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceAroundPointerQualifiers =
      space_around_pointer_qualifiers_style.at(
          old.SpaceAroundPointerQualifiers);
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeCaseColon = old.SpaceBeforeCaseColon;
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
  retval.SpaceInEmptyBlock = old.SpaceInEmptyBlock;
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = spaces_in_angles_style.at(old.SpacesInAngles);
  retval.SpacesInConditionalStatement = old.SpacesInConditionalStatement;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  newField("SpacesInLineCommentPrefix.Minimum", "13",
           retval.SpacesInLineCommentPrefix.Minimum);
  newField("SpacesInLineCommentPrefix.Maximum", "13",
           retval.SpacesInLineCommentPrefix.Maximum);
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.SpaceBeforeSquareBrackets = old.SpaceBeforeSquareBrackets;
  retval.BitFieldColonSpacing =
      bite_field_colon_spacing_style.at(old.BitFieldColonSpacing);
  retval.Standard = language_standard.at(old.Standard);
  retval.StatementAttributeLikeMacros = old.StatementAttributeLikeMacros;
  retval.TabWidth = old.TabWidth;
  retval.UseCRLF = old.UseCRLF;
  retval.UseTab = use_tab_style.at(old.UseTab);

  return retval;
}

} // namespace clang_update_v13

namespace clang_update_v14 {

constexpr frozen::unordered_map<clang_v13::FormatStyle::BracketAlignmentStyle,
                                clang_v14::FormatStyle::BracketAlignmentStyle,
                                3>
    bracket_all_alignment_style{
        {clang_v13::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v14::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v13::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v14::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v13::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v14::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::ArrayInitializerAlignmentStyle,
    clang_v14::FormatStyle::ArrayInitializerAlignmentStyle, 3>
    array_initializer_alignment_style{
        {clang_v13::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left,
         clang_v14::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left},
        {clang_v13::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right,
         clang_v14::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right},
        {clang_v13::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None,
         clang_v14::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::AlignConsecutiveStyle,
                                clang_v14::FormatStyle::AlignConsecutiveStyle,
                                5>
    align_consecutive_style{
        {clang_v13::FormatStyle::AlignConsecutiveStyle::ACS_None,
         clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_None},
        {clang_v13::FormatStyle::AlignConsecutiveStyle::ACS_Consecutive,
         clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_Consecutive},
        {clang_v13::FormatStyle::AlignConsecutiveStyle::ACS_AcrossEmptyLines,
         clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_AcrossEmptyLines},
        {clang_v13::FormatStyle::AlignConsecutiveStyle::ACS_AcrossComments,
         clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_AcrossComments},
        {clang_v13::FormatStyle::AlignConsecutiveStyle::
             ACS_AcrossEmptyLinesAndComments,
         clang_v14::FormatStyle::AlignConsecutiveStyle::
             ACS_AcrossEmptyLinesAndComments}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v14::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v13::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v14::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v13::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v14::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v13::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v14::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::OperandAlignmentStyle,
                                clang_v14::FormatStyle::OperandAlignmentStyle,
                                3>
    operand_alignment_style{
        {clang_v13::FormatStyle::OperandAlignmentStyle::OAS_DontAlign,
         clang_v14::FormatStyle::OperandAlignmentStyle::OAS_DontAlign},
        {clang_v13::FormatStyle::OperandAlignmentStyle::OAS_Align,
         clang_v14::FormatStyle::OperandAlignmentStyle::OAS_Align},
        {clang_v13::FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator,
         clang_v14::FormatStyle::OperandAlignmentStyle::
             OAS_AlignAfterOperator}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::ShortBlockStyle,
                                clang_v14::FormatStyle::ShortBlockStyle, 3>
    short_block_style{{clang_v13::FormatStyle::ShortBlockStyle::SBS_Never,
                       clang_v14::FormatStyle::ShortBlockStyle::SBS_Never},
                      {clang_v13::FormatStyle::ShortBlockStyle::SBS_Empty,
                       clang_v14::FormatStyle::ShortBlockStyle::SBS_Empty},
                      {clang_v13::FormatStyle::ShortBlockStyle::SBS_Always,
                       clang_v14::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::ShortFunctionStyle,
                                clang_v14::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v13::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v14::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v13::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v14::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v13::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v14::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v13::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v14::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v13::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v14::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::ShortIfStyle,
                                clang_v14::FormatStyle::ShortIfStyle, 4>
    short_if_style{{clang_v13::FormatStyle::ShortIfStyle::SIS_Never,
                    clang_v14::FormatStyle::ShortIfStyle::SIS_Never},
                   {clang_v13::FormatStyle::ShortIfStyle::SIS_WithoutElse,
                    clang_v14::FormatStyle::ShortIfStyle::SIS_WithoutElse},
                   {clang_v13::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf,
                    clang_v14::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf},
                   {clang_v13::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse,
                    clang_v14::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::ShortLambdaStyle,
                                clang_v14::FormatStyle::ShortLambdaStyle, 4>
    short_lambda_style{{clang_v13::FormatStyle::ShortLambdaStyle::SLS_None,
                        clang_v14::FormatStyle::ShortLambdaStyle::SLS_None},
                       {clang_v13::FormatStyle::ShortLambdaStyle::SLS_Empty,
                        clang_v14::FormatStyle::ShortLambdaStyle::SLS_Empty},
                       {clang_v13::FormatStyle::ShortLambdaStyle::SLS_Inline,
                        clang_v14::FormatStyle::ShortLambdaStyle::SLS_Inline},
                       {clang_v13::FormatStyle::ShortLambdaStyle::SLS_All,
                        clang_v14::FormatStyle::ShortLambdaStyle::SLS_All}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v14::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v13::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v14::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v13::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v14::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v13::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v14::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v14::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v13::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v14::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v13::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v14::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v13::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v14::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v13::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v14::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v13::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v14::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v14::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v13::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v14::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v13::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v14::FormatStyle::BreakTemplateDeclarationsStyle::
             BTDS_MultiLine},
        {clang_v13::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v14::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::TrailingCommaStyle,
                                clang_v14::FormatStyle::TrailingCommaStyle, 2>
    trailing_comma_style{
        {clang_v13::FormatStyle::TrailingCommaStyle::TCS_None,
         clang_v14::FormatStyle::TrailingCommaStyle::TCS_None},
        {clang_v13::FormatStyle::TrailingCommaStyle::TCS_Wrapped,
         clang_v14::FormatStyle::TrailingCommaStyle::TCS_Wrapped}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::BinaryOperatorStyle,
                                clang_v14::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v13::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v14::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v13::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v14::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v13::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v14::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::BraceBreakingStyle,
                                clang_v14::FormatStyle::BraceBreakingStyle, 9>
    brace_breaking_style{
        {clang_v13::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v14::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v13::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v14::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v13::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v14::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v13::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v14::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v13::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v14::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v13::FormatStyle::BraceBreakingStyle::BS_Whitesmiths,
         clang_v14::FormatStyle::BraceBreakingStyle::BS_Whitesmiths},
        {clang_v13::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v14::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v13::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v14::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v13::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v14::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::BraceWrappingAfterControlStatementStyle,
    clang_v14::FormatStyle::BraceWrappingAfterControlStatementStyle, 3>
    brace_wrapping_after_control_statement_style{
        {clang_v13::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never,
         clang_v14::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never},
        {clang_v13::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine,
         clang_v14::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine},
        {clang_v13::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always,
         clang_v14::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::BreakConstructorInitializersStyle,
    clang_v14::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v13::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v14::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v13::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v14::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v13::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v14::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::BreakInheritanceListStyle,
    clang_v14::FormatStyle::BreakInheritanceListStyle, 4>
    break_inheritance_list_style{
        {clang_v13::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v14::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v13::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v14::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v13::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v14::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon},
        {clang_v13::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma,
         clang_v14::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::EmptyLineAfterAccessModifierStyle,
    clang_v14::FormatStyle::EmptyLineAfterAccessModifierStyle, 3>
    empty_line_after_access_modifier_style{
        {clang_v13::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never,
         clang_v14::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never},
        {clang_v13::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave,
         clang_v14::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave},
        {clang_v13::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always,
         clang_v14::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::EmptyLineBeforeAccessModifierStyle,
    clang_v14::FormatStyle::EmptyLineBeforeAccessModifierStyle, 4>
    empty_line_before_access_modifier_style{
        {clang_v13::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never,
         clang_v14::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never},
        {clang_v13::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave,
         clang_v14::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave},
        {clang_v13::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock,
         clang_v14::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock},
        {clang_v13::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always,
         clang_v14::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always}};

constexpr frozen::unordered_map<clang_v13::IncludeStyle::IncludeBlocksStyle,
                                clang_v14::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v13::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v14::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v13::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v14::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v13::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v14::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v13::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v14::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v14::IncludeStyle::IncludeCategory{
        item.Regex, item.Priority, item.SortPriority, true});
  }
}

constexpr frozen::unordered_map<clang_v13::FormatStyle::PPDirectiveIndentStyle,
                                clang_v14::FormatStyle::PPDirectiveIndentStyle,
                                3>
    pp_directive_indent_style{
        {clang_v13::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v14::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v13::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v14::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash},
        {clang_v13::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash,
         clang_v14::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::IndentExternBlockStyle,
                                clang_v14::FormatStyle::IndentExternBlockStyle,
                                3>
    indent_extern_block_style{
        {clang_v13::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock,
         clang_v14::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock},
        {clang_v13::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent,
         clang_v14::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent},
        {clang_v13::FormatStyle::IndentExternBlockStyle::IEBS_Indent,
         clang_v14::FormatStyle::IndentExternBlockStyle::IEBS_Indent}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::JavaScriptQuoteStyle,
                                clang_v14::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v13::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v14::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v13::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v14::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v13::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v14::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::LanguageKind,
                                clang_v14::FormatStyle::LanguageKind, 10>
    language_king{{clang_v13::FormatStyle::LanguageKind::LK_None,
                   clang_v14::FormatStyle::LanguageKind::LK_None},
                  {clang_v13::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v14::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v13::FormatStyle::LanguageKind::LK_CSharp,
                   clang_v14::FormatStyle::LanguageKind::LK_CSharp},
                  {clang_v13::FormatStyle::LanguageKind::LK_Java,
                   clang_v14::FormatStyle::LanguageKind::LK_Java},
                  {clang_v13::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v14::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v13::FormatStyle::LanguageKind::LK_Json,
                   clang_v14::FormatStyle::LanguageKind::LK_Json},
                  {clang_v13::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v14::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v13::FormatStyle::LanguageKind::LK_Proto,
                   clang_v14::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v13::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v14::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v13::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v14::FormatStyle::LanguageKind::LK_TextProto}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::LambdaBodyIndentationKind,
    clang_v14::FormatStyle::LambdaBodyIndentationKind, 2>
    lambda_body_indentation_king{
        {clang_v13::FormatStyle::LambdaBodyIndentationKind::LBI_Signature,
         clang_v14::FormatStyle::LambdaBodyIndentationKind::LBI_Signature},
        {clang_v13::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope,
         clang_v14::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::NamespaceIndentationKind,
    clang_v14::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v13::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v14::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v13::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v14::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v13::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v14::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::BinPackStyle,
                                clang_v14::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v13::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v14::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v13::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v14::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v13::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v14::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::PointerAlignmentStyle,
                                clang_v14::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v13::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v14::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v13::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v14::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v13::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v14::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

void assign(std::vector<clang_v13::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v14::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v14::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<clang_v13::FormatStyle::ReferenceAlignmentStyle,
                                clang_v14::FormatStyle::ReferenceAlignmentStyle,
                                4>
    reference_alignment_style{
        {clang_v13::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer,
         clang_v14::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer},
        {clang_v13::FormatStyle::ReferenceAlignmentStyle::RAS_Left,
         clang_v14::FormatStyle::ReferenceAlignmentStyle::RAS_Left},
        {clang_v13::FormatStyle::ReferenceAlignmentStyle::RAS_Right,
         clang_v14::FormatStyle::ReferenceAlignmentStyle::RAS_Right},
        {clang_v13::FormatStyle::ReferenceAlignmentStyle::RAS_Middle,
         clang_v14::FormatStyle::ReferenceAlignmentStyle::RAS_Middle}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::SortIncludesOptions,
                                clang_v14::FormatStyle::SortIncludesOptions, 3>
    sort_includes_options{
        {clang_v13::FormatStyle::SortIncludesOptions::SI_Never,
         clang_v14::FormatStyle::SortIncludesOptions::SI_Never},
        {clang_v13::FormatStyle::SortIncludesOptions::SI_CaseSensitive,
         clang_v14::FormatStyle::SortIncludesOptions::SI_CaseSensitive},
        {clang_v13::FormatStyle::SortIncludesOptions::SI_CaseInsensitive,
         clang_v14::FormatStyle::SortIncludesOptions::SI_CaseInsensitive}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::SortJavaStaticImportOptions,
    clang_v14::FormatStyle::SortJavaStaticImportOptions, 2>
    sort_java_static_import_options{
        {clang_v13::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before,
         clang_v14::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before},
        {clang_v13::FormatStyle::SortJavaStaticImportOptions::SJSIO_After,
         clang_v14::FormatStyle::SortJavaStaticImportOptions::SJSIO_After}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::SpaceAroundPointerQualifiersStyle,
    clang_v14::FormatStyle::SpaceAroundPointerQualifiersStyle, 4>
    space_around_pointer_qualifiers_style{
        {clang_v13::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default,
         clang_v14::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default},
        {clang_v13::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Before,
         clang_v14::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Before},
        {clang_v13::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After,
         clang_v14::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After},
        {clang_v13::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both,
         clang_v14::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::SpaceBeforeParensOptions,
    clang_v14::FormatStyle::SpaceBeforeParensStyle, 5>
    space_before_parens_options{
        {clang_v13::FormatStyle::SpaceBeforeParensOptions::SBPO_Never,
         clang_v14::FormatStyle::SpaceBeforeParensStyle::SBPO_Never},
        {clang_v13::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatements,
         clang_v14::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatements},
        {clang_v13::FormatStyle::SpaceBeforeParensOptions::
             SBPO_ControlStatementsExceptControlMacros,
         clang_v14::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros},
        {clang_v13::FormatStyle::SpaceBeforeParensOptions::
             SBPO_NonEmptyParentheses,
         clang_v14::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses},
        {clang_v13::FormatStyle::SpaceBeforeParensOptions::SBPO_Always,
         clang_v14::FormatStyle::SpaceBeforeParensStyle::SBPO_Always}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::SpacesInAnglesStyle,
                                clang_v14::FormatStyle::SpacesInAnglesStyle, 3>
    spaces_in_angles_style{
        {clang_v13::FormatStyle::SpacesInAnglesStyle::SIAS_Never,
         clang_v14::FormatStyle::SpacesInAnglesStyle::SIAS_Never},
        {clang_v13::FormatStyle::SpacesInAnglesStyle::SIAS_Always,
         clang_v14::FormatStyle::SpacesInAnglesStyle::SIAS_Always},
        {clang_v13::FormatStyle::SpacesInAnglesStyle::SIAS_Leave,
         clang_v14::FormatStyle::SpacesInAnglesStyle::SIAS_Leave}};

constexpr frozen::unordered_map<
    clang_v13::FormatStyle::BitFieldColonSpacingStyle,
    clang_v14::FormatStyle::BitFieldColonSpacingStyle, 4>
    bite_field_colon_spacing_style{
        {clang_v13::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both,
         clang_v14::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both},
        {clang_v13::FormatStyle::BitFieldColonSpacingStyle::BFCS_None,
         clang_v14::FormatStyle::BitFieldColonSpacingStyle::BFCS_None},
        {clang_v13::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before,
         clang_v14::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before},
        {clang_v13::FormatStyle::BitFieldColonSpacingStyle::BFCS_After,
         clang_v14::FormatStyle::BitFieldColonSpacingStyle::BFCS_After}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::LanguageStandard,
                                clang_v14::FormatStyle::LanguageStandard, 7>
    language_standard{{clang_v13::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v14::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v13::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v14::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v13::FormatStyle::LanguageStandard::LS_Cpp14,
                       clang_v14::FormatStyle::LanguageStandard::LS_Cpp14},
                      {clang_v13::FormatStyle::LanguageStandard::LS_Cpp17,
                       clang_v14::FormatStyle::LanguageStandard::LS_Cpp17},
                      {clang_v13::FormatStyle::LanguageStandard::LS_Cpp20,
                       clang_v14::FormatStyle::LanguageStandard::LS_Cpp20},
                      {clang_v13::FormatStyle::LanguageStandard::LS_Latest,
                       clang_v14::FormatStyle::LanguageStandard::LS_Latest},
                      {clang_v13::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v14::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v13::FormatStyle::UseTabStyle,
                                clang_v14::FormatStyle::UseTabStyle, 5>
    use_tab_style{
        {clang_v13::FormatStyle::UseTabStyle::UT_Never,
         clang_v14::FormatStyle::UseTabStyle::UT_Never},
        {clang_v13::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v14::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v13::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v14::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v13::FormatStyle::UseTabStyle::UT_AlignWithSpaces,
         clang_v14::FormatStyle::UseTabStyle::UT_AlignWithSpaces},
        {clang_v13::FormatStyle::UseTabStyle::UT_Always,
         clang_v14::FormatStyle::UseTabStyle::UT_Always}};

clang_v14::FormatStyle update(clang_v13::FormatStyle &old,
                              const std::string &style) {
  clang_v14::FormatStyle retval;
  if (!clang_v14::getPredefinedStyle(
          style, clang_v14::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.InheritsParentConfig = old.InheritsParentConfig;
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  improveField("AlignAfterOpenBracket", "BlockIndent", "14");
  retval.AlignArrayOfStructures =
      array_initializer_alignment_style.at(old.AlignArrayOfStructures);
  retval.AlignConsecutiveMacros =
      align_consecutive_style.at(old.AlignConsecutiveMacros);
  retval.AlignConsecutiveAssignments =
      align_consecutive_style.at(old.AlignConsecutiveAssignments);
  retval.AlignConsecutiveBitFields =
      align_consecutive_style.at(old.AlignConsecutiveBitFields);
  retval.AlignConsecutiveDeclarations =
      align_consecutive_style.at(old.AlignConsecutiveDeclarations);
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  retval.AlignOperands = operand_alignment_style.at(old.AlignOperands);
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AllowAllArgumentsOnNextLine = old.AllowAllArgumentsOnNextLine;
  retval.AllowAllConstructorInitializersOnNextLine =
      old.AllowAllConstructorInitializersOnNextLine;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortEnumsOnASingleLine = old.AllowShortEnumsOnASingleLine;
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
  retval.AttributeMacros = old.AttributeMacros;
  retval.BinPackArguments = old.BinPackArguments;
  retval.InsertTrailingCommas =
      trailing_comma_style.at(old.InsertTrailingCommas);
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
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
  retval.BraceWrapping.BeforeLambdaBody = old.BraceWrapping.BeforeLambdaBody;
  retval.BraceWrapping.BeforeWhile = old.BraceWrapping.BeforeWhile;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakBeforeConceptDeclarations = old.BreakBeforeConceptDeclarations;
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializers =
      break_constructor_initializers_style.at(old.BreakConstructorInitializers);
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  newField("QualifierAlignment", "14", retval.QualifierAlignment);
  newField("QualifierOrder", "14", retval.QualifierOrder);
  retval.BreakInheritanceList =
      break_inheritance_list_style.at(old.BreakInheritanceList);
  retval.CompactNamespaces = old.CompactNamespaces;
  retval.ConstructorInitializerAllOnOneLineOrOnePerLine =
      old.ConstructorInitializerAllOnOneLineOrOnePerLine;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.DeriveLineEnding = old.DeriveLineEnding;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.EmptyLineAfterAccessModifier =
      empty_line_after_access_modifier_style.at(
          old.EmptyLineAfterAccessModifier);
  retval.EmptyLineBeforeAccessModifier =
      empty_line_before_access_modifier_style.at(
          old.EmptyLineBeforeAccessModifier);
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  newField("PackConstructorInitializers", "14",
           retval.PackConstructorInitializers);
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  retval.IfMacros = old.IfMacros;
  retval.TypenameMacros = old.TypenameMacros;
  retval.StatementMacros = old.StatementMacros;
  retval.NamespaceMacros = old.NamespaceMacros;
  retval.WhitespaceSensitiveMacros = old.WhitespaceSensitiveMacros;
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  retval.IncludeStyle.IncludeIsMainSourceRegex =
      old.IncludeStyle.IncludeIsMainSourceRegex;
  retval.IndentAccessModifiers = old.IndentAccessModifiers;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentCaseBlocks = old.IndentCaseBlocks;
  retval.IndentGotoLabels = old.IndentGotoLabels;
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  retval.IndentExternBlock =
      indent_extern_block_style.at(old.IndentExternBlock);
  retval.IndentRequires = old.IndentRequires;
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.JavaImportGroups = old.JavaImportGroups;
  retval.JavaScriptQuotes = java_script_quote_style.at(old.JavaScriptQuotes);
  retval.JavaScriptWrapImports = old.JavaScriptWrapImports;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.Language = language_king.at(old.Language);
  retval.LambdaBodyIndentation =
      lambda_body_indentation_king.at(old.LambdaBodyIndentation);
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.ObjCBinPackProtocolList =
      bin_pack_style.at(old.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCBreakBeforeNestedBlockParam = old.ObjCBreakBeforeNestedBlockParam;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  newField("PenaltyBreakOpenParenthesis", "14",
           retval.PenaltyBreakOpenParenthesis);
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyBreakTemplateDeclaration = old.PenaltyBreakTemplateDeclaration;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PenaltyIndentedWhitespace = old.PenaltyIndentedWhitespace;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.PPIndentWidth = old.PPIndentWidth;
  assign(old.RawStringFormats, retval.RawStringFormats);
  retval.ReferenceAlignment =
      reference_alignment_style.at(old.ReferenceAlignment);
  retval.ReflowComments = old.ReflowComments;
  newField("RemoveBracesLLVM", "14", retval.RemoveBracesLLVM);
  newField("SeparateDefinitionBlocks", "14", retval.SeparateDefinitionBlocks);
  retval.ShortNamespaceLines = old.ShortNamespaceLines;
  retval.SortIncludes = sort_includes_options.at(old.SortIncludes);
  retval.SortJavaStaticImport =
      sort_java_static_import_options.at(old.SortJavaStaticImport);
  retval.SortUsingDeclarations = old.SortUsingDeclarations;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterLogicalNot = old.SpaceAfterLogicalNot;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceAroundPointerQualifiers =
      space_around_pointer_qualifiers_style.at(
          old.SpaceAroundPointerQualifiers);
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeCaseColon = old.SpaceBeforeCaseColon;
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  improveField("SpaceBeforeParens", "Custom", "14");
  newField("SpaceBeforeParensOptions.AfterControlStatements", "14",
           retval.SpaceBeforeParensOptions.AfterControlStatements);
  newField("SpaceBeforeParensOptions.AfterForeachMacros", "14",
           retval.SpaceBeforeParensOptions.AfterForeachMacros);
  newField("SpaceBeforeParensOptions.AfterFunctionDeclarationName", "14",
           retval.SpaceBeforeParensOptions.AfterFunctionDeclarationName);
  newField("SpaceBeforeParensOptions.AfterFunctionDefinitionName", "14",
           retval.SpaceBeforeParensOptions.AfterFunctionDefinitionName);
  newField("SpaceBeforeParensOptions.AfterIfMacros", "14",
           retval.SpaceBeforeParensOptions.AfterIfMacros);
  newField("SpaceBeforeParensOptions.AfterOverloadedOperator", "14",
           retval.SpaceBeforeParensOptions.AfterOverloadedOperator);
  newField("SpaceBeforeParensOptions.BeforeNonEmptyParentheses", "14",
           retval.SpaceBeforeParensOptions.BeforeNonEmptyParentheses);
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
  retval.SpaceInEmptyBlock = old.SpaceInEmptyBlock;
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = spaces_in_angles_style.at(old.SpacesInAngles);
  retval.SpacesInConditionalStatement = old.SpacesInConditionalStatement;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  retval.SpacesInLineCommentPrefix.Minimum =
      old.SpacesInLineCommentPrefix.Minimum;
  retval.SpacesInLineCommentPrefix.Maximum =
      old.SpacesInLineCommentPrefix.Maximum;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.SpaceBeforeSquareBrackets = old.SpaceBeforeSquareBrackets;
  retval.BitFieldColonSpacing =
      bite_field_colon_spacing_style.at(old.BitFieldColonSpacing);
  retval.Standard = language_standard.at(old.Standard);
  retval.StatementAttributeLikeMacros = old.StatementAttributeLikeMacros;
  retval.TabWidth = old.TabWidth;
  retval.UseCRLF = old.UseCRLF;
  retval.UseTab = use_tab_style.at(old.UseTab);

  return retval;
}

} // namespace clang_update_v14

namespace clang_update_v15 {

constexpr frozen::unordered_map<clang_v14::FormatStyle::BracketAlignmentStyle,
                                clang_v15::FormatStyle::BracketAlignmentStyle,
                                4>
    bracket_all_alignment_style{
        {clang_v14::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v15::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v14::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v15::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v14::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v15::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak},
        {clang_v14::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent,
         clang_v15::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::ArrayInitializerAlignmentStyle,
    clang_v15::FormatStyle::ArrayInitializerAlignmentStyle, 3>
    array_initializer_alignment_style{
        {clang_v14::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left,
         clang_v15::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left},
        {clang_v14::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right,
         clang_v15::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right},
        {clang_v14::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None,
         clang_v15::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::AlignConsecutiveStyle,
                                clang_v15::FormatStyle::AlignConsecutiveStyle,
                                5>
    align_consecutive_style{
        {clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_None,
         {false, false, false, false, true}},
        {clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_Consecutive,
         {true, false, false, false, true}},
        {clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_AcrossEmptyLines,
         {true, true, false, false, true}},
        {clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_AcrossComments,
         {true, false, true, false, true}},
        {clang_v14::FormatStyle::AlignConsecutiveStyle::
             ACS_AcrossEmptyLinesAndComments,
         {true, true, true, false, true}}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v15::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v14::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v15::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v14::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v15::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v14::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v15::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::OperandAlignmentStyle,
                                clang_v15::FormatStyle::OperandAlignmentStyle,
                                3>
    operand_alignment_style{
        {clang_v14::FormatStyle::OperandAlignmentStyle::OAS_DontAlign,
         clang_v15::FormatStyle::OperandAlignmentStyle::OAS_DontAlign},
        {clang_v14::FormatStyle::OperandAlignmentStyle::OAS_Align,
         clang_v15::FormatStyle::OperandAlignmentStyle::OAS_Align},
        {clang_v14::FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator,
         clang_v15::FormatStyle::OperandAlignmentStyle::
             OAS_AlignAfterOperator}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::ShortBlockStyle,
                                clang_v15::FormatStyle::ShortBlockStyle, 3>
    short_block_style{{clang_v14::FormatStyle::ShortBlockStyle::SBS_Never,
                       clang_v15::FormatStyle::ShortBlockStyle::SBS_Never},
                      {clang_v14::FormatStyle::ShortBlockStyle::SBS_Empty,
                       clang_v15::FormatStyle::ShortBlockStyle::SBS_Empty},
                      {clang_v14::FormatStyle::ShortBlockStyle::SBS_Always,
                       clang_v15::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::ShortFunctionStyle,
                                clang_v15::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v14::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v15::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v14::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v15::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v14::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v15::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v14::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v15::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v14::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v15::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::ShortIfStyle,
                                clang_v15::FormatStyle::ShortIfStyle, 4>
    short_if_style{{clang_v14::FormatStyle::ShortIfStyle::SIS_Never,
                    clang_v15::FormatStyle::ShortIfStyle::SIS_Never},
                   {clang_v14::FormatStyle::ShortIfStyle::SIS_WithoutElse,
                    clang_v15::FormatStyle::ShortIfStyle::SIS_WithoutElse},
                   {clang_v14::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf,
                    clang_v15::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf},
                   {clang_v14::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse,
                    clang_v15::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::ShortLambdaStyle,
                                clang_v15::FormatStyle::ShortLambdaStyle, 4>
    short_lambda_style{{clang_v14::FormatStyle::ShortLambdaStyle::SLS_None,
                        clang_v15::FormatStyle::ShortLambdaStyle::SLS_None},
                       {clang_v14::FormatStyle::ShortLambdaStyle::SLS_Empty,
                        clang_v15::FormatStyle::ShortLambdaStyle::SLS_Empty},
                       {clang_v14::FormatStyle::ShortLambdaStyle::SLS_Inline,
                        clang_v15::FormatStyle::ShortLambdaStyle::SLS_Inline},
                       {clang_v14::FormatStyle::ShortLambdaStyle::SLS_All,
                        clang_v15::FormatStyle::ShortLambdaStyle::SLS_All}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v15::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v14::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v15::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v14::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v15::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v14::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v15::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v15::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v14::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v15::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v14::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v15::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v14::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v15::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v14::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v15::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v14::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v15::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v15::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v14::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v15::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v14::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v15::FormatStyle::BreakTemplateDeclarationsStyle::
             BTDS_MultiLine},
        {clang_v14::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v15::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::TrailingCommaStyle,
                                clang_v15::FormatStyle::TrailingCommaStyle, 2>
    trailing_comma_style{
        {clang_v14::FormatStyle::TrailingCommaStyle::TCS_None,
         clang_v15::FormatStyle::TrailingCommaStyle::TCS_None},
        {clang_v14::FormatStyle::TrailingCommaStyle::TCS_Wrapped,
         clang_v15::FormatStyle::TrailingCommaStyle::TCS_Wrapped}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::BinaryOperatorStyle,
                                clang_v15::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v14::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v15::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v14::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v15::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v14::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v15::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::BraceBreakingStyle,
                                clang_v15::FormatStyle::BraceBreakingStyle, 9>
    brace_breaking_style{
        {clang_v14::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v15::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v14::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v15::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v14::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v15::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v14::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v15::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v14::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v15::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v14::FormatStyle::BraceBreakingStyle::BS_Whitesmiths,
         clang_v15::FormatStyle::BraceBreakingStyle::BS_Whitesmiths},
        {clang_v14::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v15::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v14::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v15::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v14::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v15::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::BraceWrappingAfterControlStatementStyle,
    clang_v15::FormatStyle::BraceWrappingAfterControlStatementStyle, 3>
    brace_wrapping_after_control_statement_style{
        {clang_v14::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never,
         clang_v15::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never},
        {clang_v14::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine,
         clang_v15::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine},
        {clang_v14::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always,
         clang_v15::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always}};

constexpr frozen::unordered_map<
    bool, clang_v15::FormatStyle::BreakBeforeConceptDeclarationsStyle, 2>
    break_before_concept_declarations_style{
        {false, clang_v15::FormatStyle::BreakBeforeConceptDeclarationsStyle::
                    BBCDS_Allowed},
        {true, clang_v15::FormatStyle::BreakBeforeConceptDeclarationsStyle::
                   BBCDS_Always}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::BreakConstructorInitializersStyle,
    clang_v15::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v14::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v15::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v14::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v15::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v14::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v15::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::QualifierAlignmentStyle,
                                clang_v15::FormatStyle::QualifierAlignmentStyle,
                                4>
    qualifier_alignment_style{
        {clang_v14::FormatStyle::QualifierAlignmentStyle::QAS_Leave,
         clang_v15::FormatStyle::QualifierAlignmentStyle::QAS_Leave},
        {clang_v14::FormatStyle::QualifierAlignmentStyle::QAS_Left,
         clang_v15::FormatStyle::QualifierAlignmentStyle::QAS_Left},
        {clang_v14::FormatStyle::QualifierAlignmentStyle::QAS_Right,
         clang_v15::FormatStyle::QualifierAlignmentStyle::QAS_Right},
        {clang_v14::FormatStyle::QualifierAlignmentStyle::QAS_Custom,
         clang_v15::FormatStyle::QualifierAlignmentStyle::QAS_Custom}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::BreakInheritanceListStyle,
    clang_v15::FormatStyle::BreakInheritanceListStyle, 4>
    break_inheritance_list_style{
        {clang_v14::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v15::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v14::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v15::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v14::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v15::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon},
        {clang_v14::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma,
         clang_v15::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::EmptyLineAfterAccessModifierStyle,
    clang_v15::FormatStyle::EmptyLineAfterAccessModifierStyle, 3>
    empty_line_after_access_modifier_style{
        {clang_v14::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never,
         clang_v15::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never},
        {clang_v14::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave,
         clang_v15::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave},
        {clang_v14::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always,
         clang_v15::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::EmptyLineBeforeAccessModifierStyle,
    clang_v15::FormatStyle::EmptyLineBeforeAccessModifierStyle, 4>
    empty_line_before_access_modifier_style{
        {clang_v14::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never,
         clang_v15::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never},
        {clang_v14::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave,
         clang_v15::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave},
        {clang_v14::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock,
         clang_v15::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock},
        {clang_v14::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always,
         clang_v15::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always}};

constexpr frozen::unordered_map<clang_v14::IncludeStyle::IncludeBlocksStyle,
                                clang_v15::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v14::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v15::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v14::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v15::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v14::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v15::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v14::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v15::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v15::IncludeStyle::IncludeCategory{
        item.Regex, item.Priority, item.SortPriority, true});
  }
}

constexpr frozen::unordered_map<clang_v14::FormatStyle::PPDirectiveIndentStyle,
                                clang_v15::FormatStyle::PPDirectiveIndentStyle,
                                3>
    pp_directive_indent_style{
        {clang_v14::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v15::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v14::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v15::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash},
        {clang_v14::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash,
         clang_v15::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::IndentExternBlockStyle,
                                clang_v15::FormatStyle::IndentExternBlockStyle,
                                3>
    indent_extern_block_style{
        {clang_v14::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock,
         clang_v15::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock},
        {clang_v14::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent,
         clang_v15::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent},
        {clang_v14::FormatStyle::IndentExternBlockStyle::IEBS_Indent,
         clang_v15::FormatStyle::IndentExternBlockStyle::IEBS_Indent}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::JavaScriptQuoteStyle,
                                clang_v15::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v14::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v15::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v14::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v15::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v14::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v15::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::LanguageKind,
                                clang_v15::FormatStyle::LanguageKind, 10>
    language_king{{clang_v14::FormatStyle::LanguageKind::LK_None,
                   clang_v15::FormatStyle::LanguageKind::LK_None},
                  {clang_v14::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v15::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v14::FormatStyle::LanguageKind::LK_CSharp,
                   clang_v15::FormatStyle::LanguageKind::LK_CSharp},
                  {clang_v14::FormatStyle::LanguageKind::LK_Java,
                   clang_v15::FormatStyle::LanguageKind::LK_Java},
                  {clang_v14::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v15::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v14::FormatStyle::LanguageKind::LK_Json,
                   clang_v15::FormatStyle::LanguageKind::LK_Json},
                  {clang_v14::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v15::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v14::FormatStyle::LanguageKind::LK_Proto,
                   clang_v15::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v14::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v15::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v14::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v15::FormatStyle::LanguageKind::LK_TextProto}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::LambdaBodyIndentationKind,
    clang_v15::FormatStyle::LambdaBodyIndentationKind, 2>
    lambda_body_indentation_king{
        {clang_v14::FormatStyle::LambdaBodyIndentationKind::LBI_Signature,
         clang_v15::FormatStyle::LambdaBodyIndentationKind::LBI_Signature},
        {clang_v14::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope,
         clang_v15::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::NamespaceIndentationKind,
    clang_v15::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v14::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v15::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v14::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v15::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v14::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v15::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::BinPackStyle,
                                clang_v15::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v14::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v15::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v14::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v15::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v14::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v15::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::PointerAlignmentStyle,
                                clang_v15::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v14::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v15::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v14::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v15::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v14::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v15::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

void assign(std::vector<clang_v14::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v15::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v15::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<clang_v14::FormatStyle::ReferenceAlignmentStyle,
                                clang_v15::FormatStyle::ReferenceAlignmentStyle,
                                4>
    reference_alignment_style{
        {clang_v14::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer,
         clang_v15::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer},
        {clang_v14::FormatStyle::ReferenceAlignmentStyle::RAS_Left,
         clang_v15::FormatStyle::ReferenceAlignmentStyle::RAS_Left},
        {clang_v14::FormatStyle::ReferenceAlignmentStyle::RAS_Right,
         clang_v15::FormatStyle::ReferenceAlignmentStyle::RAS_Right},
        {clang_v14::FormatStyle::ReferenceAlignmentStyle::RAS_Middle,
         clang_v15::FormatStyle::ReferenceAlignmentStyle::RAS_Middle}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::SeparateDefinitionStyle,
                                clang_v15::FormatStyle::SeparateDefinitionStyle,
                                3>
    separate_definitions_style{
        {clang_v14::FormatStyle::SeparateDefinitionStyle::SDS_Leave,
         clang_v15::FormatStyle::SeparateDefinitionStyle::SDS_Leave},
        {clang_v14::FormatStyle::SeparateDefinitionStyle::SDS_Always,
         clang_v15::FormatStyle::SeparateDefinitionStyle::SDS_Always},
        {clang_v14::FormatStyle::SeparateDefinitionStyle::SDS_Never,
         clang_v15::FormatStyle::SeparateDefinitionStyle::SDS_Never}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::SortIncludesOptions,
                                clang_v15::FormatStyle::SortIncludesOptions, 3>
    sort_includes_options{
        {clang_v14::FormatStyle::SortIncludesOptions::SI_Never,
         clang_v15::FormatStyle::SortIncludesOptions::SI_Never},
        {clang_v14::FormatStyle::SortIncludesOptions::SI_CaseSensitive,
         clang_v15::FormatStyle::SortIncludesOptions::SI_CaseSensitive},
        {clang_v14::FormatStyle::SortIncludesOptions::SI_CaseInsensitive,
         clang_v15::FormatStyle::SortIncludesOptions::SI_CaseInsensitive}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::SortJavaStaticImportOptions,
    clang_v15::FormatStyle::SortJavaStaticImportOptions, 2>
    sort_java_static_import_options{
        {clang_v14::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before,
         clang_v15::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before},
        {clang_v14::FormatStyle::SortJavaStaticImportOptions::SJSIO_After,
         clang_v15::FormatStyle::SortJavaStaticImportOptions::SJSIO_After}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::SpaceAroundPointerQualifiersStyle,
    clang_v15::FormatStyle::SpaceAroundPointerQualifiersStyle, 4>
    space_around_pointer_qualifiers_style{
        {clang_v14::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default,
         clang_v15::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default},
        {clang_v14::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Before,
         clang_v15::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Before},
        {clang_v14::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After,
         clang_v15::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After},
        {clang_v14::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both,
         clang_v15::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::SpaceBeforeParensStyle,
                                clang_v15::FormatStyle::SpaceBeforeParensStyle,
                                6>
    space_before_parens_options{
        {clang_v14::FormatStyle::SpaceBeforeParensStyle::SBPO_Never,
         clang_v15::FormatStyle::SpaceBeforeParensStyle::SBPO_Never},
        {clang_v14::FormatStyle::SpaceBeforeParensStyle::SBPO_ControlStatements,
         clang_v15::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatements},
        {clang_v14::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros,
         clang_v15::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros},
        {clang_v14::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses,
         clang_v15::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses},
        {clang_v14::FormatStyle::SpaceBeforeParensStyle::SBPO_Always,
         clang_v15::FormatStyle::SpaceBeforeParensStyle::SBPO_Always},
        {clang_v14::FormatStyle::SpaceBeforeParensStyle::SBPO_Custom,
         clang_v15::FormatStyle::SpaceBeforeParensStyle::SBPO_Custom}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::SpacesInAnglesStyle,
                                clang_v15::FormatStyle::SpacesInAnglesStyle, 3>
    spaces_in_angles_style{
        {clang_v14::FormatStyle::SpacesInAnglesStyle::SIAS_Never,
         clang_v15::FormatStyle::SpacesInAnglesStyle::SIAS_Never},
        {clang_v14::FormatStyle::SpacesInAnglesStyle::SIAS_Always,
         clang_v15::FormatStyle::SpacesInAnglesStyle::SIAS_Always},
        {clang_v14::FormatStyle::SpacesInAnglesStyle::SIAS_Leave,
         clang_v15::FormatStyle::SpacesInAnglesStyle::SIAS_Leave}};

constexpr frozen::unordered_map<
    clang_v14::FormatStyle::BitFieldColonSpacingStyle,
    clang_v15::FormatStyle::BitFieldColonSpacingStyle, 4>
    bite_field_colon_spacing_style{
        {clang_v14::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both,
         clang_v15::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both},
        {clang_v14::FormatStyle::BitFieldColonSpacingStyle::BFCS_None,
         clang_v15::FormatStyle::BitFieldColonSpacingStyle::BFCS_None},
        {clang_v14::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before,
         clang_v15::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before},
        {clang_v14::FormatStyle::BitFieldColonSpacingStyle::BFCS_After,
         clang_v15::FormatStyle::BitFieldColonSpacingStyle::BFCS_After}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::LanguageStandard,
                                clang_v15::FormatStyle::LanguageStandard, 7>
    language_standard{{clang_v14::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v15::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v14::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v15::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v14::FormatStyle::LanguageStandard::LS_Cpp14,
                       clang_v15::FormatStyle::LanguageStandard::LS_Cpp14},
                      {clang_v14::FormatStyle::LanguageStandard::LS_Cpp17,
                       clang_v15::FormatStyle::LanguageStandard::LS_Cpp17},
                      {clang_v14::FormatStyle::LanguageStandard::LS_Cpp20,
                       clang_v15::FormatStyle::LanguageStandard::LS_Cpp20},
                      {clang_v14::FormatStyle::LanguageStandard::LS_Latest,
                       clang_v15::FormatStyle::LanguageStandard::LS_Latest},
                      {clang_v14::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v15::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v14::FormatStyle::UseTabStyle,
                                clang_v15::FormatStyle::UseTabStyle, 5>
    use_tab_style{
        {clang_v14::FormatStyle::UseTabStyle::UT_Never,
         clang_v15::FormatStyle::UseTabStyle::UT_Never},
        {clang_v14::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v15::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v14::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v15::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v14::FormatStyle::UseTabStyle::UT_AlignWithSpaces,
         clang_v15::FormatStyle::UseTabStyle::UT_AlignWithSpaces},
        {clang_v14::FormatStyle::UseTabStyle::UT_Always,
         clang_v15::FormatStyle::UseTabStyle::UT_Always}};

clang_v15::FormatStyle update(clang_v14::FormatStyle &old,
                              const std::string &style) {
  clang_v15::FormatStyle retval;
  if (!clang_v15::getPredefinedStyle(
          style, clang_v15::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.InheritsParentConfig = old.InheritsParentConfig;
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignArrayOfStructures =
      array_initializer_alignment_style.at(old.AlignArrayOfStructures);
  retval.AlignConsecutiveMacros =
      align_consecutive_style.at(old.AlignConsecutiveMacros);
  retval.AlignConsecutiveAssignments =
      align_consecutive_style.at(old.AlignConsecutiveAssignments);
  retval.AlignConsecutiveBitFields =
      align_consecutive_style.at(old.AlignConsecutiveBitFields);
  retval.AlignConsecutiveDeclarations =
      align_consecutive_style.at(old.AlignConsecutiveDeclarations);
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  retval.AlignOperands = operand_alignment_style.at(old.AlignOperands);
  retval.AlignTrailingComments = old.AlignTrailingComments;
  retval.AllowAllArgumentsOnNextLine = old.AllowAllArgumentsOnNextLine;

  assignWithWarning("AllowAllConstructorInitializersOnNextLine",
                    old.AllowAllConstructorInitializersOnNextLine,
                    "PackConstructorInitializers",
                    retval.PackConstructorInitializers,
                    old.AllowAllConstructorInitializersOnNextLine
                        ? clang_v15::FormatStyle::
                              PackConstructorInitializersStyle::PCIS_NextLine
                        : clang_v15::FormatStyle::
                              PackConstructorInitializersStyle::PCIS_BinPack,
                    "15");
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortEnumsOnASingleLine = old.AllowShortEnumsOnASingleLine;
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
  retval.AttributeMacros = old.AttributeMacros;
  retval.BinPackArguments = old.BinPackArguments;
  retval.InsertTrailingCommas =
      trailing_comma_style.at(old.InsertTrailingCommas);
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
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
  retval.BraceWrapping.BeforeLambdaBody = old.BraceWrapping.BeforeLambdaBody;
  retval.BraceWrapping.BeforeWhile = old.BraceWrapping.BeforeWhile;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakBeforeConceptDeclarations =
      break_before_concept_declarations_style.at(
          old.BreakBeforeConceptDeclarations);
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializers =
      break_constructor_initializers_style.at(old.BreakConstructorInitializers);
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.QualifierAlignment =
      qualifier_alignment_style.at(old.QualifierAlignment);
  retval.QualifierOrder = old.QualifierOrder;
  retval.BreakInheritanceList =
      break_inheritance_list_style.at(old.BreakInheritanceList);
  retval.CompactNamespaces = old.CompactNamespaces;
  assignWithWarning("ConstructorInitializerAllOnOneLineOrOnePerLine",
                    old.ConstructorInitializerAllOnOneLineOrOnePerLine,
                    "PackConstructorInitializers",
                    retval.PackConstructorInitializers,
                    old.ConstructorInitializerAllOnOneLineOrOnePerLine
                        ? clang_v15::FormatStyle::
                              PackConstructorInitializersStyle::PCIS_NextLine
                        : clang_v15::FormatStyle::
                              PackConstructorInitializersStyle::PCIS_BinPack,
                    "15");
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.DeriveLineEnding = old.DeriveLineEnding;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.EmptyLineAfterAccessModifier =
      empty_line_after_access_modifier_style.at(
          old.EmptyLineAfterAccessModifier);
  retval.EmptyLineBeforeAccessModifier =
      empty_line_before_access_modifier_style.at(
          old.EmptyLineBeforeAccessModifier);
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  retval.IfMacros = old.IfMacros;
  retval.TypenameMacros = old.TypenameMacros;
  retval.StatementMacros = old.StatementMacros;
  retval.NamespaceMacros = old.NamespaceMacros;
  retval.WhitespaceSensitiveMacros = old.WhitespaceSensitiveMacros;
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  retval.IncludeStyle.IncludeIsMainSourceRegex =
      old.IncludeStyle.IncludeIsMainSourceRegex;
  retval.IndentAccessModifiers = old.IndentAccessModifiers;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentCaseBlocks = old.IndentCaseBlocks;
  retval.IndentGotoLabels = old.IndentGotoLabels;
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  retval.IndentExternBlock =
      indent_extern_block_style.at(old.IndentExternBlock);
  retval.IndentRequiresClause = old.IndentRequires;
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  newField("InsertBraces", "15", retval.InsertBraces);
  retval.JavaImportGroups = old.JavaImportGroups;
  retval.JavaScriptQuotes = java_script_quote_style.at(old.JavaScriptQuotes);
  retval.JavaScriptWrapImports = old.JavaScriptWrapImports;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.Language = language_king.at(old.Language);
  improveField("Language", "Verilog", "15");
  retval.LambdaBodyIndentation =
      lambda_body_indentation_king.at(old.LambdaBodyIndentation);
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.ObjCBinPackProtocolList =
      bin_pack_style.at(old.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCBreakBeforeNestedBlockParam = old.ObjCBreakBeforeNestedBlockParam;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakOpenParenthesis = old.PenaltyBreakOpenParenthesis;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyBreakTemplateDeclaration = old.PenaltyBreakTemplateDeclaration;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PenaltyIndentedWhitespace = old.PenaltyIndentedWhitespace;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.PPIndentWidth = old.PPIndentWidth;
  assign(old.RawStringFormats, retval.RawStringFormats);
  retval.ReferenceAlignment =
      reference_alignment_style.at(old.ReferenceAlignment);
  retval.ReflowComments = old.ReflowComments;
  retval.RemoveBracesLLVM = old.RemoveBracesLLVM;
  newField("RequiresClausePosition", "15", retval.RequiresClausePosition);
  retval.SeparateDefinitionBlocks =
      separate_definitions_style.at(old.SeparateDefinitionBlocks);
  retval.ShortNamespaceLines = old.ShortNamespaceLines;
  retval.SortIncludes = sort_includes_options.at(old.SortIncludes);
  retval.SortJavaStaticImport =
      sort_java_static_import_options.at(old.SortJavaStaticImport);
  retval.SortUsingDeclarations = old.SortUsingDeclarations;
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterLogicalNot = old.SpaceAfterLogicalNot;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceAroundPointerQualifiers =
      space_around_pointer_qualifiers_style.at(
          old.SpaceAroundPointerQualifiers);
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeCaseColon = old.SpaceBeforeCaseColon;
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);

  retval.SpaceBeforeParensOptions.AfterControlStatements =
      old.SpaceBeforeParensOptions.AfterControlStatements;
  retval.SpaceBeforeParensOptions.AfterForeachMacros =
      old.SpaceBeforeParensOptions.AfterForeachMacros;
  retval.SpaceBeforeParensOptions.AfterFunctionDeclarationName =
      old.SpaceBeforeParensOptions.AfterFunctionDeclarationName;
  retval.SpaceBeforeParensOptions.AfterFunctionDefinitionName =
      old.SpaceBeforeParensOptions.AfterFunctionDefinitionName;
  retval.SpaceBeforeParensOptions.AfterIfMacros =
      old.SpaceBeforeParensOptions.AfterIfMacros;
  retval.SpaceBeforeParensOptions.AfterOverloadedOperator =
      old.SpaceBeforeParensOptions.AfterOverloadedOperator;
  newField("SpaceBeforeParensOptions.AfterRequiresInClause", "15",
           retval.SpaceBeforeParensOptions.AfterRequiresInClause);
  newField("SpaceBeforeParensOptions.AfterRequiresInExpression", "15",
           retval.SpaceBeforeParensOptions.AfterRequiresInExpression);
  retval.SpaceBeforeParensOptions.BeforeNonEmptyParentheses =
      old.SpaceBeforeParensOptions.BeforeNonEmptyParentheses;
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
  retval.SpaceInEmptyBlock = old.SpaceInEmptyBlock;
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = spaces_in_angles_style.at(old.SpacesInAngles);
  retval.SpacesInConditionalStatement = old.SpacesInConditionalStatement;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  retval.SpacesInLineCommentPrefix.Minimum =
      old.SpacesInLineCommentPrefix.Minimum;
  retval.SpacesInLineCommentPrefix.Maximum =
      old.SpacesInLineCommentPrefix.Maximum;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.SpaceBeforeSquareBrackets = old.SpaceBeforeSquareBrackets;
  retval.BitFieldColonSpacing =
      bite_field_colon_spacing_style.at(old.BitFieldColonSpacing);
  retval.Standard = language_standard.at(old.Standard);
  retval.StatementAttributeLikeMacros = old.StatementAttributeLikeMacros;
  retval.TabWidth = old.TabWidth;
  retval.UseCRLF = old.UseCRLF;
  retval.UseTab = use_tab_style.at(old.UseTab);

  return retval;
}

} // namespace clang_update_v15

namespace clang_update_v16 {

constexpr frozen::unordered_map<clang_v15::FormatStyle::BracketAlignmentStyle,
                                clang_v16::FormatStyle::BracketAlignmentStyle,
                                4>
    bracket_all_alignment_style{
        {clang_v15::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v16::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v15::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v16::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v15::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v16::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak},
        {clang_v15::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent,
         clang_v16::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::ArrayInitializerAlignmentStyle,
    clang_v16::FormatStyle::ArrayInitializerAlignmentStyle, 3>
    array_initializer_alignment_style{
        {clang_v15::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left,
         clang_v16::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left},
        {clang_v15::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right,
         clang_v16::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right},
        {clang_v15::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None,
         clang_v16::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v16::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v15::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v16::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v15::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v16::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v15::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v16::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::OperandAlignmentStyle,
                                clang_v16::FormatStyle::OperandAlignmentStyle,
                                3>
    operand_alignment_style{
        {clang_v15::FormatStyle::OperandAlignmentStyle::OAS_DontAlign,
         clang_v16::FormatStyle::OperandAlignmentStyle::OAS_DontAlign},
        {clang_v15::FormatStyle::OperandAlignmentStyle::OAS_Align,
         clang_v16::FormatStyle::OperandAlignmentStyle::OAS_Align},
        {clang_v15::FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator,
         clang_v16::FormatStyle::OperandAlignmentStyle::
             OAS_AlignAfterOperator}};

constexpr frozen::unordered_map<
    bool, clang_v16::FormatStyle::TrailingCommentsAlignmentStyle, 2>
    trailing_comments_alignment_style{
        {false,
         {clang_v16::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Always,
          1}},
        {true,
         {clang_v16::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Never,
          1}}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::ShortBlockStyle,
                                clang_v16::FormatStyle::ShortBlockStyle, 3>
    short_block_style{{clang_v15::FormatStyle::ShortBlockStyle::SBS_Never,
                       clang_v16::FormatStyle::ShortBlockStyle::SBS_Never},
                      {clang_v15::FormatStyle::ShortBlockStyle::SBS_Empty,
                       clang_v16::FormatStyle::ShortBlockStyle::SBS_Empty},
                      {clang_v15::FormatStyle::ShortBlockStyle::SBS_Always,
                       clang_v16::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::ShortFunctionStyle,
                                clang_v16::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v15::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v16::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v15::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v16::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v15::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v16::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v15::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v16::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v15::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v16::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::ShortIfStyle,
                                clang_v16::FormatStyle::ShortIfStyle, 4>
    short_if_style{{clang_v15::FormatStyle::ShortIfStyle::SIS_Never,
                    clang_v16::FormatStyle::ShortIfStyle::SIS_Never},
                   {clang_v15::FormatStyle::ShortIfStyle::SIS_WithoutElse,
                    clang_v16::FormatStyle::ShortIfStyle::SIS_WithoutElse},
                   {clang_v15::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf,
                    clang_v16::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf},
                   {clang_v15::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse,
                    clang_v16::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::ShortLambdaStyle,
                                clang_v16::FormatStyle::ShortLambdaStyle, 4>
    short_lambda_style{{clang_v15::FormatStyle::ShortLambdaStyle::SLS_None,
                        clang_v16::FormatStyle::ShortLambdaStyle::SLS_None},
                       {clang_v15::FormatStyle::ShortLambdaStyle::SLS_Empty,
                        clang_v16::FormatStyle::ShortLambdaStyle::SLS_Empty},
                       {clang_v15::FormatStyle::ShortLambdaStyle::SLS_Inline,
                        clang_v16::FormatStyle::ShortLambdaStyle::SLS_Inline},
                       {clang_v15::FormatStyle::ShortLambdaStyle::SLS_All,
                        clang_v16::FormatStyle::ShortLambdaStyle::SLS_All}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v16::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v15::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v16::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v15::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v16::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v15::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v16::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v16::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v15::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v16::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v15::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v16::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v15::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v16::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v15::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v16::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v15::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v16::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v16::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v15::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v16::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v15::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v16::FormatStyle::BreakTemplateDeclarationsStyle::
             BTDS_MultiLine},
        {clang_v15::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v16::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::TrailingCommaStyle,
                                clang_v16::FormatStyle::TrailingCommaStyle, 2>
    trailing_comma_style{
        {clang_v15::FormatStyle::TrailingCommaStyle::TCS_None,
         clang_v16::FormatStyle::TrailingCommaStyle::TCS_None},
        {clang_v15::FormatStyle::TrailingCommaStyle::TCS_Wrapped,
         clang_v16::FormatStyle::TrailingCommaStyle::TCS_Wrapped}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::BinaryOperatorStyle,
                                clang_v16::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v15::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v16::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v15::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v16::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v15::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v16::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::BraceBreakingStyle,
                                clang_v16::FormatStyle::BraceBreakingStyle, 9>
    brace_breaking_style{
        {clang_v15::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v16::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v15::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v16::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v15::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v16::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v15::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v16::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v15::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v16::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v15::FormatStyle::BraceBreakingStyle::BS_Whitesmiths,
         clang_v16::FormatStyle::BraceBreakingStyle::BS_Whitesmiths},
        {clang_v15::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v16::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v15::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v16::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v15::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v16::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::BraceWrappingAfterControlStatementStyle,
    clang_v16::FormatStyle::BraceWrappingAfterControlStatementStyle, 3>
    brace_wrapping_after_control_statement_style{
        {clang_v15::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never,
         clang_v16::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never},
        {clang_v15::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine,
         clang_v16::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine},
        {clang_v15::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always,
         clang_v16::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::BreakBeforeConceptDeclarationsStyle,
    clang_v16::FormatStyle::BreakBeforeConceptDeclarationsStyle, 3>
    break_before_concept_declarations_style{
        {clang_v15::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Never,
         clang_v16::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Never},
        {clang_v15::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Allowed,
         clang_v16::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Allowed},
        {clang_v15::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Always,
         clang_v16::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Always}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::BreakConstructorInitializersStyle,
    clang_v16::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v15::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v16::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v15::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v16::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v15::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v16::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::QualifierAlignmentStyle,
                                clang_v16::FormatStyle::QualifierAlignmentStyle,
                                4>
    qualifier_alignment_style{
        {clang_v15::FormatStyle::QualifierAlignmentStyle::QAS_Leave,
         clang_v16::FormatStyle::QualifierAlignmentStyle::QAS_Leave},
        {clang_v15::FormatStyle::QualifierAlignmentStyle::QAS_Left,
         clang_v16::FormatStyle::QualifierAlignmentStyle::QAS_Left},
        {clang_v15::FormatStyle::QualifierAlignmentStyle::QAS_Right,
         clang_v16::FormatStyle::QualifierAlignmentStyle::QAS_Right},
        {clang_v15::FormatStyle::QualifierAlignmentStyle::QAS_Custom,
         clang_v16::FormatStyle::QualifierAlignmentStyle::QAS_Custom}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::BreakInheritanceListStyle,
    clang_v16::FormatStyle::BreakInheritanceListStyle, 4>
    break_inheritance_list_style{
        {clang_v15::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v16::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v15::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v16::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v15::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v16::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon},
        {clang_v15::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma,
         clang_v16::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::EmptyLineAfterAccessModifierStyle,
    clang_v16::FormatStyle::EmptyLineAfterAccessModifierStyle, 3>
    empty_line_after_access_modifier_style{
        {clang_v15::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never,
         clang_v16::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never},
        {clang_v15::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave,
         clang_v16::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave},
        {clang_v15::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always,
         clang_v16::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::EmptyLineBeforeAccessModifierStyle,
    clang_v16::FormatStyle::EmptyLineBeforeAccessModifierStyle, 4>
    empty_line_before_access_modifier_style{
        {clang_v15::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never,
         clang_v16::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never},
        {clang_v15::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave,
         clang_v16::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave},
        {clang_v15::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock,
         clang_v16::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock},
        {clang_v15::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always,
         clang_v16::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::PackConstructorInitializersStyle,
    clang_v16::FormatStyle::PackConstructorInitializersStyle, 4>
    pack_constructor_initializers_style{
        {clang_v15::FormatStyle::PackConstructorInitializersStyle::PCIS_Never,
         clang_v16::FormatStyle::PackConstructorInitializersStyle::PCIS_Never},
        {clang_v15::FormatStyle::PackConstructorInitializersStyle::PCIS_BinPack,
         clang_v16::FormatStyle::PackConstructorInitializersStyle::
             PCIS_BinPack},
        {clang_v15::FormatStyle::PackConstructorInitializersStyle::
             PCIS_CurrentLine,
         clang_v16::FormatStyle::PackConstructorInitializersStyle::
             PCIS_CurrentLine},
        {clang_v15::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLine,
         clang_v16::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLine}};

constexpr frozen::unordered_map<clang_v15::IncludeStyle::IncludeBlocksStyle,
                                clang_v16::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v15::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v16::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v15::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v16::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v15::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v16::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v15::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v16::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v16::IncludeStyle::IncludeCategory{
        item.Regex, item.Priority, item.SortPriority, true});
  }
}

constexpr frozen::unordered_map<clang_v15::FormatStyle::PPDirectiveIndentStyle,
                                clang_v16::FormatStyle::PPDirectiveIndentStyle,
                                3>
    pp_directive_indent_style{
        {clang_v15::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v16::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v15::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v16::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash},
        {clang_v15::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash,
         clang_v16::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::IndentExternBlockStyle,
                                clang_v16::FormatStyle::IndentExternBlockStyle,
                                3>
    indent_extern_block_style{
        {clang_v15::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock,
         clang_v16::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock},
        {clang_v15::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent,
         clang_v16::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent},
        {clang_v15::FormatStyle::IndentExternBlockStyle::IEBS_Indent,
         clang_v16::FormatStyle::IndentExternBlockStyle::IEBS_Indent}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::JavaScriptQuoteStyle,
                                clang_v16::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v15::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v16::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v15::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v16::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v15::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v16::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::LanguageKind,
                                clang_v16::FormatStyle::LanguageKind, 11>
    language_king{{clang_v15::FormatStyle::LanguageKind::LK_None,
                   clang_v16::FormatStyle::LanguageKind::LK_None},
                  {clang_v15::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v16::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v15::FormatStyle::LanguageKind::LK_CSharp,
                   clang_v16::FormatStyle::LanguageKind::LK_CSharp},
                  {clang_v15::FormatStyle::LanguageKind::LK_Java,
                   clang_v16::FormatStyle::LanguageKind::LK_Java},
                  {clang_v15::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v16::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v15::FormatStyle::LanguageKind::LK_Json,
                   clang_v16::FormatStyle::LanguageKind::LK_Json},
                  {clang_v15::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v16::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v15::FormatStyle::LanguageKind::LK_Proto,
                   clang_v16::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v15::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v16::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v15::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v16::FormatStyle::LanguageKind::LK_TextProto},
                  {clang_v15::FormatStyle::LanguageKind::LK_Verilog,
                   clang_v16::FormatStyle::LanguageKind::LK_Verilog}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::LambdaBodyIndentationKind,
    clang_v16::FormatStyle::LambdaBodyIndentationKind, 2>
    lambda_body_indentation_king{
        {clang_v15::FormatStyle::LambdaBodyIndentationKind::LBI_Signature,
         clang_v16::FormatStyle::LambdaBodyIndentationKind::LBI_Signature},
        {clang_v15::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope,
         clang_v16::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::NamespaceIndentationKind,
    clang_v16::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v15::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v16::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v15::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v16::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v15::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v16::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::BinPackStyle,
                                clang_v16::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v15::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v16::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v15::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v16::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v15::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v16::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::PointerAlignmentStyle,
                                clang_v16::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v15::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v16::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v15::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v16::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v15::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v16::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

void assign(std::vector<clang_v15::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v16::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v16::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<clang_v15::FormatStyle::ReferenceAlignmentStyle,
                                clang_v16::FormatStyle::ReferenceAlignmentStyle,
                                4>
    reference_alignment_style{
        {clang_v15::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer,
         clang_v16::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer},
        {clang_v15::FormatStyle::ReferenceAlignmentStyle::RAS_Left,
         clang_v16::FormatStyle::ReferenceAlignmentStyle::RAS_Left},
        {clang_v15::FormatStyle::ReferenceAlignmentStyle::RAS_Right,
         clang_v16::FormatStyle::ReferenceAlignmentStyle::RAS_Right},
        {clang_v15::FormatStyle::ReferenceAlignmentStyle::RAS_Middle,
         clang_v16::FormatStyle::ReferenceAlignmentStyle::RAS_Middle}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::RequiresClausePositionStyle,
    clang_v16::FormatStyle::RequiresClausePositionStyle, 4>
    requires_clause_position_style{
        {clang_v15::FormatStyle::RequiresClausePositionStyle::RCPS_OwnLine,
         clang_v16::FormatStyle::RequiresClausePositionStyle::RCPS_OwnLine},
        {clang_v15::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithPreceding,
         clang_v16::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithPreceding},
        {clang_v15::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithFollowing,
         clang_v16::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithFollowing},
        {clang_v15::FormatStyle::RequiresClausePositionStyle::RCPS_SingleLine,
         clang_v16::FormatStyle::RequiresClausePositionStyle::RCPS_SingleLine}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::SeparateDefinitionStyle,
                                clang_v16::FormatStyle::SeparateDefinitionStyle,
                                3>
    separate_definitions_style{
        {clang_v15::FormatStyle::SeparateDefinitionStyle::SDS_Leave,
         clang_v16::FormatStyle::SeparateDefinitionStyle::SDS_Leave},
        {clang_v15::FormatStyle::SeparateDefinitionStyle::SDS_Always,
         clang_v16::FormatStyle::SeparateDefinitionStyle::SDS_Always},
        {clang_v15::FormatStyle::SeparateDefinitionStyle::SDS_Never,
         clang_v16::FormatStyle::SeparateDefinitionStyle::SDS_Never}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::SortIncludesOptions,
                                clang_v16::FormatStyle::SortIncludesOptions, 3>
    sort_includes_options{
        {clang_v15::FormatStyle::SortIncludesOptions::SI_Never,
         clang_v16::FormatStyle::SortIncludesOptions::SI_Never},
        {clang_v15::FormatStyle::SortIncludesOptions::SI_CaseSensitive,
         clang_v16::FormatStyle::SortIncludesOptions::SI_CaseSensitive},
        {clang_v15::FormatStyle::SortIncludesOptions::SI_CaseInsensitive,
         clang_v16::FormatStyle::SortIncludesOptions::SI_CaseInsensitive}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::SortJavaStaticImportOptions,
    clang_v16::FormatStyle::SortJavaStaticImportOptions, 2>
    sort_java_static_import_options{
        {clang_v15::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before,
         clang_v16::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before},
        {clang_v15::FormatStyle::SortJavaStaticImportOptions::SJSIO_After,
         clang_v16::FormatStyle::SortJavaStaticImportOptions::SJSIO_After}};

constexpr frozen::unordered_map<
    bool, clang_v16::FormatStyle::SortUsingDeclarationsOptions, 2>
    sort_using_declarations_options{
        {false,
         clang_v16::FormatStyle::SortUsingDeclarationsOptions::SUD_Never},
        {true, clang_v16::FormatStyle::SortUsingDeclarationsOptions::
                   SUD_LexicographicNumeric}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::SpaceAroundPointerQualifiersStyle,
    clang_v16::FormatStyle::SpaceAroundPointerQualifiersStyle, 4>
    space_around_pointer_qualifiers_style{
        {clang_v15::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default,
         clang_v16::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default},
        {clang_v15::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Before,
         clang_v16::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Before},
        {clang_v15::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After,
         clang_v16::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After},
        {clang_v15::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both,
         clang_v16::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::SpaceBeforeParensStyle,
                                clang_v16::FormatStyle::SpaceBeforeParensStyle,
                                6>
    space_before_parens_options{
        {clang_v15::FormatStyle::SpaceBeforeParensStyle::SBPO_Never,
         clang_v16::FormatStyle::SpaceBeforeParensStyle::SBPO_Never},
        {clang_v15::FormatStyle::SpaceBeforeParensStyle::SBPO_ControlStatements,
         clang_v16::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatements},
        {clang_v15::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros,
         clang_v16::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros},
        {clang_v15::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses,
         clang_v16::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses},
        {clang_v15::FormatStyle::SpaceBeforeParensStyle::SBPO_Always,
         clang_v16::FormatStyle::SpaceBeforeParensStyle::SBPO_Always},
        {clang_v15::FormatStyle::SpaceBeforeParensStyle::SBPO_Custom,
         clang_v16::FormatStyle::SpaceBeforeParensStyle::SBPO_Custom}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::SpacesInAnglesStyle,
                                clang_v16::FormatStyle::SpacesInAnglesStyle, 3>
    spaces_in_angles_style{
        {clang_v15::FormatStyle::SpacesInAnglesStyle::SIAS_Never,
         clang_v16::FormatStyle::SpacesInAnglesStyle::SIAS_Never},
        {clang_v15::FormatStyle::SpacesInAnglesStyle::SIAS_Always,
         clang_v16::FormatStyle::SpacesInAnglesStyle::SIAS_Always},
        {clang_v15::FormatStyle::SpacesInAnglesStyle::SIAS_Leave,
         clang_v16::FormatStyle::SpacesInAnglesStyle::SIAS_Leave}};

constexpr frozen::unordered_map<
    clang_v15::FormatStyle::BitFieldColonSpacingStyle,
    clang_v16::FormatStyle::BitFieldColonSpacingStyle, 4>
    bite_field_colon_spacing_style{
        {clang_v15::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both,
         clang_v16::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both},
        {clang_v15::FormatStyle::BitFieldColonSpacingStyle::BFCS_None,
         clang_v16::FormatStyle::BitFieldColonSpacingStyle::BFCS_None},
        {clang_v15::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before,
         clang_v16::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before},
        {clang_v15::FormatStyle::BitFieldColonSpacingStyle::BFCS_After,
         clang_v16::FormatStyle::BitFieldColonSpacingStyle::BFCS_After}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::LanguageStandard,
                                clang_v16::FormatStyle::LanguageStandard, 7>
    language_standard{{clang_v15::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v16::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v15::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v16::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v15::FormatStyle::LanguageStandard::LS_Cpp14,
                       clang_v16::FormatStyle::LanguageStandard::LS_Cpp14},
                      {clang_v15::FormatStyle::LanguageStandard::LS_Cpp17,
                       clang_v16::FormatStyle::LanguageStandard::LS_Cpp17},
                      {clang_v15::FormatStyle::LanguageStandard::LS_Cpp20,
                       clang_v16::FormatStyle::LanguageStandard::LS_Cpp20},
                      {clang_v15::FormatStyle::LanguageStandard::LS_Latest,
                       clang_v16::FormatStyle::LanguageStandard::LS_Latest},
                      {clang_v15::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v16::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v15::FormatStyle::UseTabStyle,
                                clang_v16::FormatStyle::UseTabStyle, 5>
    use_tab_style{
        {clang_v15::FormatStyle::UseTabStyle::UT_Never,
         clang_v16::FormatStyle::UseTabStyle::UT_Never},
        {clang_v15::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v16::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v15::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v16::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v15::FormatStyle::UseTabStyle::UT_AlignWithSpaces,
         clang_v16::FormatStyle::UseTabStyle::UT_AlignWithSpaces},
        {clang_v15::FormatStyle::UseTabStyle::UT_Always,
         clang_v16::FormatStyle::UseTabStyle::UT_Always}};

clang_v16::FormatStyle update(clang_v15::FormatStyle &old,
                              const std::string &style) {
  clang_v16::FormatStyle retval;
  if (!clang_v16::getPredefinedStyle(
          style, clang_v16::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.InheritsParentConfig = old.InheritsParentConfig;
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignArrayOfStructures =
      array_initializer_alignment_style.at(old.AlignArrayOfStructures);
  retval.AlignConsecutiveMacros.Enabled = old.AlignConsecutiveMacros.Enabled;
  retval.AlignConsecutiveMacros.AcrossEmptyLines =
      old.AlignConsecutiveMacros.AcrossEmptyLines;
  retval.AlignConsecutiveMacros.AcrossComments =
      old.AlignConsecutiveMacros.AcrossComments;
  retval.AlignConsecutiveMacros.AlignCompound =
      old.AlignConsecutiveMacros.AlignCompound;
  retval.AlignConsecutiveMacros.PadOperators =
      old.AlignConsecutiveMacros.PadOperators;
  retval.AlignConsecutiveAssignments.Enabled =
      old.AlignConsecutiveAssignments.Enabled;
  retval.AlignConsecutiveAssignments.AcrossEmptyLines =
      old.AlignConsecutiveAssignments.AcrossEmptyLines;
  retval.AlignConsecutiveAssignments.AcrossComments =
      old.AlignConsecutiveAssignments.AcrossComments;
  retval.AlignConsecutiveAssignments.AlignCompound =
      old.AlignConsecutiveAssignments.AlignCompound;
  retval.AlignConsecutiveAssignments.PadOperators =
      old.AlignConsecutiveAssignments.PadOperators;
  retval.AlignConsecutiveBitFields.Enabled =
      old.AlignConsecutiveBitFields.Enabled;
  retval.AlignConsecutiveBitFields.AcrossEmptyLines =
      old.AlignConsecutiveBitFields.AcrossEmptyLines;
  retval.AlignConsecutiveBitFields.AcrossComments =
      old.AlignConsecutiveBitFields.AcrossComments;
  retval.AlignConsecutiveBitFields.AlignCompound =
      old.AlignConsecutiveBitFields.AlignCompound;
  retval.AlignConsecutiveBitFields.PadOperators =
      old.AlignConsecutiveBitFields.PadOperators;
  retval.AlignConsecutiveDeclarations.Enabled =
      old.AlignConsecutiveDeclarations.Enabled;
  retval.AlignConsecutiveDeclarations.AcrossEmptyLines =
      old.AlignConsecutiveDeclarations.AcrossEmptyLines;
  retval.AlignConsecutiveDeclarations.AcrossComments =
      old.AlignConsecutiveDeclarations.AcrossComments;
  retval.AlignConsecutiveDeclarations.AlignCompound =
      old.AlignConsecutiveDeclarations.AlignCompound;
  retval.AlignConsecutiveDeclarations.PadOperators =
      old.AlignConsecutiveDeclarations.PadOperators;
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  retval.AlignOperands = operand_alignment_style.at(old.AlignOperands);
  retval.AlignTrailingComments =
      trailing_comments_alignment_style.at(old.AlignTrailingComments);
  retval.AllowAllArgumentsOnNextLine = old.AllowAllArgumentsOnNextLine;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortEnumsOnASingleLine = old.AllowShortEnumsOnASingleLine;
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
  retval.AttributeMacros = old.AttributeMacros;
  retval.BinPackArguments = old.BinPackArguments;
  retval.InsertTrailingCommas =
      trailing_comma_style.at(old.InsertTrailingCommas);
  retval.BinPackParameters = old.BinPackParameters;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
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
  retval.BraceWrapping.BeforeLambdaBody = old.BraceWrapping.BeforeLambdaBody;
  retval.BraceWrapping.BeforeWhile = old.BraceWrapping.BeforeWhile;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakBeforeConceptDeclarations =
      break_before_concept_declarations_style.at(
          old.BreakBeforeConceptDeclarations);
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  newField("BreakBeforeInlineASMColon", "16", retval.BreakBeforeInlineASMColon);
  retval.BreakConstructorInitializers =
      break_constructor_initializers_style.at(old.BreakConstructorInitializers);
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.QualifierAlignment =
      qualifier_alignment_style.at(old.QualifierAlignment);
  retval.QualifierOrder = old.QualifierOrder;
  retval.BreakInheritanceList =
      break_inheritance_list_style.at(old.BreakInheritanceList);
  retval.CompactNamespaces = old.CompactNamespaces;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.LineEnding =
      old.DeriveLineEnding
          ? (old.UseCRLF
                 ? clang_v16::FormatStyle::LineEndingStyle::LE_DeriveCRLF
                 : clang_v16::FormatStyle::LineEndingStyle::LE_DeriveLF)
          : (old.UseCRLF ? clang_v16::FormatStyle::LineEndingStyle::LE_CRLF
                         : clang_v16::FormatStyle::LineEndingStyle::LE_LF);
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.EmptyLineAfterAccessModifier =
      empty_line_after_access_modifier_style.at(
          old.EmptyLineAfterAccessModifier);
  retval.EmptyLineBeforeAccessModifier =
      empty_line_before_access_modifier_style.at(
          old.EmptyLineBeforeAccessModifier);
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.PackConstructorInitializers =
      pack_constructor_initializers_style.at(old.PackConstructorInitializers);
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  retval.IncludeStyle.IncludeIsMainSourceRegex =
      old.IncludeStyle.IncludeIsMainSourceRegex;
  retval.IfMacros = old.IfMacros;
  retval.TypenameMacros = old.TypenameMacros;
  retval.StatementMacros = old.StatementMacros;
  retval.NamespaceMacros = old.NamespaceMacros;
  retval.WhitespaceSensitiveMacros = old.WhitespaceSensitiveMacros;
  retval.IndentAccessModifiers = old.IndentAccessModifiers;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentCaseBlocks = old.IndentCaseBlocks;
  retval.IndentGotoLabels = old.IndentGotoLabels;
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  retval.IndentExternBlock =
      indent_extern_block_style.at(old.IndentExternBlock);
  retval.IndentRequiresClause = old.IndentRequiresClause;
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.InsertBraces = old.InsertBraces;
  retval.JavaImportGroups = old.JavaImportGroups;
  retval.JavaScriptQuotes = java_script_quote_style.at(old.JavaScriptQuotes);
  retval.JavaScriptWrapImports = old.JavaScriptWrapImports;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.Language = language_king.at(old.Language);
  retval.LambdaBodyIndentation =
      lambda_body_indentation_king.at(old.LambdaBodyIndentation);
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.ObjCBinPackProtocolList =
      bin_pack_style.at(old.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCBreakBeforeNestedBlockParam = old.ObjCBreakBeforeNestedBlockParam;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakOpenParenthesis = old.PenaltyBreakOpenParenthesis;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyBreakTemplateDeclaration = old.PenaltyBreakTemplateDeclaration;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PenaltyIndentedWhitespace = old.PenaltyIndentedWhitespace;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.PPIndentWidth = old.PPIndentWidth;
  assign(old.RawStringFormats, retval.RawStringFormats);
  retval.ReferenceAlignment =
      reference_alignment_style.at(old.ReferenceAlignment);
  retval.ReflowComments = old.ReflowComments;
  retval.RemoveBracesLLVM = old.RemoveBracesLLVM;
  newField("RemoveSemicolon", "16", retval.RemoveSemicolon);
  newField("BreakAfterAttributes", "16", retval.BreakAfterAttributes);
  newField("BreakArrays", "16", retval.BreakArrays);
  newField("InsertNewlineAtEOF", "16", retval.InsertNewlineAtEOF);
  newField("IntegerLiteralSeparator.Binary", "16",
           retval.IntegerLiteralSeparator.Binary);
  newField("IntegerLiteralSeparator.BinaryMinDigits", "16",
           retval.IntegerLiteralSeparator.BinaryMinDigits);
  newField("IntegerLiteralSeparator.Decimal", "16",
           retval.IntegerLiteralSeparator.Decimal);
  newField("IntegerLiteralSeparator.DecimalMinDigits", "16",
           retval.IntegerLiteralSeparator.DecimalMinDigits);
  newField("IntegerLiteralSeparator.Hex", "16",
           retval.IntegerLiteralSeparator.Hex);
  newField("IntegerLiteralSeparator.HexMinDigits", "16",
           retval.IntegerLiteralSeparator.HexMinDigits);
  retval.RequiresClausePosition =
      requires_clause_position_style.at(old.RequiresClausePosition);
  newField("RequiresExpressionIndentation", "16",
           retval.RequiresExpressionIndentation);
  retval.SeparateDefinitionBlocks =
      separate_definitions_style.at(old.SeparateDefinitionBlocks);
  retval.ShortNamespaceLines = old.ShortNamespaceLines;
  retval.SortIncludes = sort_includes_options.at(old.SortIncludes);
  retval.SortJavaStaticImport =
      sort_java_static_import_options.at(old.SortJavaStaticImport);
  retval.SortUsingDeclarations =
      sort_using_declarations_options.at(old.SortUsingDeclarations);
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterLogicalNot = old.SpaceAfterLogicalNot;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceAroundPointerQualifiers =
      space_around_pointer_qualifiers_style.at(
          old.SpaceAroundPointerQualifiers);
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeCaseColon = old.SpaceBeforeCaseColon;
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceBeforeParensOptions.AfterControlStatements =
      old.SpaceBeforeParensOptions.AfterControlStatements;
  retval.SpaceBeforeParensOptions.AfterForeachMacros =
      old.SpaceBeforeParensOptions.AfterForeachMacros;
  retval.SpaceBeforeParensOptions.AfterFunctionDeclarationName =
      old.SpaceBeforeParensOptions.AfterFunctionDeclarationName;
  retval.SpaceBeforeParensOptions.AfterFunctionDefinitionName =
      old.SpaceBeforeParensOptions.AfterFunctionDefinitionName;
  retval.SpaceBeforeParensOptions.AfterIfMacros =
      old.SpaceBeforeParensOptions.AfterIfMacros;
  retval.SpaceBeforeParensOptions.AfterOverloadedOperator =
      old.SpaceBeforeParensOptions.AfterOverloadedOperator;
  retval.SpaceBeforeParensOptions.AfterRequiresInClause =
      old.SpaceBeforeParensOptions.AfterRequiresInClause;
  retval.SpaceBeforeParensOptions.AfterRequiresInExpression =
      old.SpaceBeforeParensOptions.AfterRequiresInExpression;
  retval.SpaceBeforeParensOptions.BeforeNonEmptyParentheses =
      old.SpaceBeforeParensOptions.BeforeNonEmptyParentheses;
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
  retval.SpaceInEmptyBlock = old.SpaceInEmptyBlock;
  retval.SpaceInEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = spaces_in_angles_style.at(old.SpacesInAngles);
  retval.SpacesInConditionalStatement = old.SpacesInConditionalStatement;
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInCStyleCastParentheses = old.SpacesInCStyleCastParentheses;
  retval.SpacesInLineCommentPrefix.Minimum =
      old.SpacesInLineCommentPrefix.Minimum;
  retval.SpacesInLineCommentPrefix.Maximum =
      old.SpacesInLineCommentPrefix.Maximum;
  retval.SpacesInParentheses = old.SpacesInParentheses;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.SpaceBeforeSquareBrackets = old.SpaceBeforeSquareBrackets;
  retval.BitFieldColonSpacing =
      bite_field_colon_spacing_style.at(old.BitFieldColonSpacing);
  retval.Standard = language_standard.at(old.Standard);
  retval.StatementAttributeLikeMacros = old.StatementAttributeLikeMacros;
  retval.TabWidth = old.TabWidth;
  retval.UseTab = use_tab_style.at(old.UseTab);

  return retval;
}

} // namespace clang_update_v16

namespace clang_update_v17 {

constexpr frozen::unordered_map<clang_v16::FormatStyle::BracketAlignmentStyle,
                                clang_v17::FormatStyle::BracketAlignmentStyle,
                                4>
    bracket_all_alignment_style{
        {clang_v16::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v17::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v16::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v17::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v16::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v17::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak},
        {clang_v16::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent,
         clang_v17::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::ArrayInitializerAlignmentStyle,
    clang_v17::FormatStyle::ArrayInitializerAlignmentStyle, 3>
    array_initializer_alignment_style{
        {clang_v16::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left,
         clang_v17::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left},
        {clang_v16::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right,
         clang_v17::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right},
        {clang_v16::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None,
         clang_v17::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v17::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v16::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v17::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v16::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v17::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v16::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v17::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::OperandAlignmentStyle,
                                clang_v17::FormatStyle::OperandAlignmentStyle,
                                3>
    operand_alignment_style{
        {clang_v16::FormatStyle::OperandAlignmentStyle::OAS_DontAlign,
         clang_v17::FormatStyle::OperandAlignmentStyle::OAS_DontAlign},
        {clang_v16::FormatStyle::OperandAlignmentStyle::OAS_Align,
         clang_v17::FormatStyle::OperandAlignmentStyle::OAS_Align},
        {clang_v16::FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator,
         clang_v17::FormatStyle::OperandAlignmentStyle::
             OAS_AlignAfterOperator}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::TrailingCommentsAlignmentKinds,
    clang_v17::FormatStyle::TrailingCommentsAlignmentKinds, 3>
    trailing_comments_alignment_kinds{
        {clang_v16::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Leave,
         clang_v17::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Leave},
        {clang_v16::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Always,
         clang_v17::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Always},
        {clang_v16::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Never,
         clang_v17::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Never}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::ShortBlockStyle,
                                clang_v17::FormatStyle::ShortBlockStyle, 3>
    short_block_style{{clang_v16::FormatStyle::ShortBlockStyle::SBS_Never,
                       clang_v17::FormatStyle::ShortBlockStyle::SBS_Never},
                      {clang_v16::FormatStyle::ShortBlockStyle::SBS_Empty,
                       clang_v17::FormatStyle::ShortBlockStyle::SBS_Empty},
                      {clang_v16::FormatStyle::ShortBlockStyle::SBS_Always,
                       clang_v17::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::ShortFunctionStyle,
                                clang_v17::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v16::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v17::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v16::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v17::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v16::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v17::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v16::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v17::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v16::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v17::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::ShortIfStyle,
                                clang_v17::FormatStyle::ShortIfStyle, 4>
    short_if_style{{clang_v16::FormatStyle::ShortIfStyle::SIS_Never,
                    clang_v17::FormatStyle::ShortIfStyle::SIS_Never},
                   {clang_v16::FormatStyle::ShortIfStyle::SIS_WithoutElse,
                    clang_v17::FormatStyle::ShortIfStyle::SIS_WithoutElse},
                   {clang_v16::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf,
                    clang_v17::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf},
                   {clang_v16::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse,
                    clang_v17::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::ShortLambdaStyle,
                                clang_v17::FormatStyle::ShortLambdaStyle, 4>
    short_lambda_style{{clang_v16::FormatStyle::ShortLambdaStyle::SLS_None,
                        clang_v17::FormatStyle::ShortLambdaStyle::SLS_None},
                       {clang_v16::FormatStyle::ShortLambdaStyle::SLS_Empty,
                        clang_v17::FormatStyle::ShortLambdaStyle::SLS_Empty},
                       {clang_v16::FormatStyle::ShortLambdaStyle::SLS_Inline,
                        clang_v17::FormatStyle::ShortLambdaStyle::SLS_Inline},
                       {clang_v16::FormatStyle::ShortLambdaStyle::SLS_All,
                        clang_v17::FormatStyle::ShortLambdaStyle::SLS_All}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v17::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v16::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v17::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v16::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v17::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v16::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v17::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v17::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v16::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v17::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v16::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v17::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v16::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v17::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v16::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v17::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v16::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v17::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v17::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v16::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v17::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v16::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v17::FormatStyle::BreakTemplateDeclarationsStyle::
             BTDS_MultiLine},
        {clang_v16::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v17::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::BitFieldColonSpacingStyle,
    clang_v17::FormatStyle::BitFieldColonSpacingStyle, 4>
    bite_field_colon_spacing_style{
        {clang_v16::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both,
         clang_v17::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both},
        {clang_v16::FormatStyle::BitFieldColonSpacingStyle::BFCS_None,
         clang_v17::FormatStyle::BitFieldColonSpacingStyle::BFCS_None},
        {clang_v16::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before,
         clang_v17::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before},
        {clang_v16::FormatStyle::BitFieldColonSpacingStyle::BFCS_After,
         clang_v17::FormatStyle::BitFieldColonSpacingStyle::BFCS_After}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::BraceWrappingAfterControlStatementStyle,
    clang_v17::FormatStyle::BraceWrappingAfterControlStatementStyle, 3>
    brace_wrapping_after_control_statement_style{
        {clang_v16::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never,
         clang_v17::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never},
        {clang_v16::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine,
         clang_v17::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine},
        {clang_v16::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always,
         clang_v17::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::AttributeBreakingStyle,
                                clang_v17::FormatStyle::AttributeBreakingStyle,
                                3>
    attribute_breaking_style{
        {clang_v16::FormatStyle::AttributeBreakingStyle::ABS_Always,
         clang_v17::FormatStyle::AttributeBreakingStyle::ABS_Always},
        {clang_v16::FormatStyle::AttributeBreakingStyle::ABS_Leave,
         clang_v17::FormatStyle::AttributeBreakingStyle::ABS_Leave},
        {clang_v16::FormatStyle::AttributeBreakingStyle::ABS_Never,
         clang_v17::FormatStyle::AttributeBreakingStyle::ABS_Never}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::BinaryOperatorStyle,
                                clang_v17::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v16::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v17::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v16::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v17::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v16::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v17::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::BraceBreakingStyle,
                                clang_v17::FormatStyle::BraceBreakingStyle, 9>
    brace_breaking_style{
        {clang_v16::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v17::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v16::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v17::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v16::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v17::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v16::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v17::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v16::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v17::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v16::FormatStyle::BraceBreakingStyle::BS_Whitesmiths,
         clang_v17::FormatStyle::BraceBreakingStyle::BS_Whitesmiths},
        {clang_v16::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v17::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v16::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v17::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v16::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v17::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::BreakBeforeConceptDeclarationsStyle,
    clang_v17::FormatStyle::BreakBeforeConceptDeclarationsStyle, 3>
    break_before_concept_declarations_style{
        {clang_v16::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Never,
         clang_v17::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Never},
        {clang_v16::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Allowed,
         clang_v17::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Allowed},
        {clang_v16::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Always,
         clang_v17::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Always}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::BreakBeforeInlineASMColonStyle,
    clang_v17::FormatStyle::BreakBeforeInlineASMColonStyle, 3>
    break_before_inline_asm_colon_style{
        {clang_v16::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Never,
         clang_v17::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Never},
        {clang_v16::FormatStyle::BreakBeforeInlineASMColonStyle::
             BBIAS_OnlyMultiline,
         clang_v17::FormatStyle::BreakBeforeInlineASMColonStyle::
             BBIAS_OnlyMultiline},
        {clang_v16::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Always,
         clang_v17::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Always}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::BreakConstructorInitializersStyle,
    clang_v17::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v16::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v17::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v16::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v17::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v16::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v17::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::BreakInheritanceListStyle,
    clang_v17::FormatStyle::BreakInheritanceListStyle, 4>
    break_inheritance_list_style{
        {clang_v16::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v17::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v16::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v17::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v16::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v17::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon},
        {clang_v16::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma,
         clang_v17::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::EmptyLineAfterAccessModifierStyle,
    clang_v17::FormatStyle::EmptyLineAfterAccessModifierStyle, 3>
    empty_line_after_access_modifier_style{
        {clang_v16::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never,
         clang_v17::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never},
        {clang_v16::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave,
         clang_v17::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave},
        {clang_v16::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always,
         clang_v17::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::EmptyLineBeforeAccessModifierStyle,
    clang_v17::FormatStyle::EmptyLineBeforeAccessModifierStyle, 4>
    empty_line_before_access_modifier_style{
        {clang_v16::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never,
         clang_v17::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never},
        {clang_v16::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave,
         clang_v17::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave},
        {clang_v16::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock,
         clang_v17::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock},
        {clang_v16::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always,
         clang_v17::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always}};

constexpr frozen::unordered_map<clang_v16::IncludeStyle::IncludeBlocksStyle,
                                clang_v17::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v16::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v17::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v16::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v17::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v16::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v17::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v16::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v17::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v17::IncludeStyle::IncludeCategory{
        item.Regex, item.Priority, item.SortPriority, true});
  }
}

constexpr frozen::unordered_map<clang_v16::FormatStyle::IndentExternBlockStyle,
                                clang_v17::FormatStyle::IndentExternBlockStyle,
                                3>
    indent_extern_block_style{
        {clang_v16::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock,
         clang_v17::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock},
        {clang_v16::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent,
         clang_v17::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent},
        {clang_v16::FormatStyle::IndentExternBlockStyle::IEBS_Indent,
         clang_v17::FormatStyle::IndentExternBlockStyle::IEBS_Indent}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::PPDirectiveIndentStyle,
                                clang_v17::FormatStyle::PPDirectiveIndentStyle,
                                3>
    pp_directive_indent_style{
        {clang_v16::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v17::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v16::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v17::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash},
        {clang_v16::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash,
         clang_v17::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::TrailingCommaStyle,
                                clang_v17::FormatStyle::TrailingCommaStyle, 2>
    trailing_comma_style{
        {clang_v16::FormatStyle::TrailingCommaStyle::TCS_None,
         clang_v17::FormatStyle::TrailingCommaStyle::TCS_None},
        {clang_v16::FormatStyle::TrailingCommaStyle::TCS_Wrapped,
         clang_v17::FormatStyle::TrailingCommaStyle::TCS_Wrapped}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::JavaScriptQuoteStyle,
                                clang_v17::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v16::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v17::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v16::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v17::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v16::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v17::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::LambdaBodyIndentationKind,
    clang_v17::FormatStyle::LambdaBodyIndentationKind, 2>
    lambda_body_indentation_king{
        {clang_v16::FormatStyle::LambdaBodyIndentationKind::LBI_Signature,
         clang_v17::FormatStyle::LambdaBodyIndentationKind::LBI_Signature},
        {clang_v16::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope,
         clang_v17::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::LanguageKind,
                                clang_v17::FormatStyle::LanguageKind, 11>
    language_king{{clang_v16::FormatStyle::LanguageKind::LK_None,
                   clang_v17::FormatStyle::LanguageKind::LK_None},
                  {clang_v16::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v17::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v16::FormatStyle::LanguageKind::LK_CSharp,
                   clang_v17::FormatStyle::LanguageKind::LK_CSharp},
                  {clang_v16::FormatStyle::LanguageKind::LK_Java,
                   clang_v17::FormatStyle::LanguageKind::LK_Java},
                  {clang_v16::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v17::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v16::FormatStyle::LanguageKind::LK_Json,
                   clang_v17::FormatStyle::LanguageKind::LK_Json},
                  {clang_v16::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v17::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v16::FormatStyle::LanguageKind::LK_Proto,
                   clang_v17::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v16::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v17::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v16::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v17::FormatStyle::LanguageKind::LK_TextProto},
                  {clang_v16::FormatStyle::LanguageKind::LK_Verilog,
                   clang_v17::FormatStyle::LanguageKind::LK_Verilog}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::LineEndingStyle,
                                clang_v17::FormatStyle::LineEndingStyle, 4>
    line_ending_style{{clang_v16::FormatStyle::LineEndingStyle::LE_LF,
                       clang_v17::FormatStyle::LineEndingStyle::LE_LF},
                      {clang_v16::FormatStyle::LineEndingStyle::LE_CRLF,
                       clang_v17::FormatStyle::LineEndingStyle::LE_CRLF},
                      {clang_v16::FormatStyle::LineEndingStyle::LE_DeriveLF,
                       clang_v17::FormatStyle::LineEndingStyle::LE_DeriveLF},
                      {clang_v16::FormatStyle::LineEndingStyle::LE_DeriveCRLF,
                       clang_v17::FormatStyle::LineEndingStyle::LE_DeriveCRLF}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::NamespaceIndentationKind,
    clang_v17::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v16::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v17::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v16::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v17::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v16::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v17::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::BinPackStyle,
                                clang_v17::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v16::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v17::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v16::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v17::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v16::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v17::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::PackConstructorInitializersStyle,
    clang_v17::FormatStyle::PackConstructorInitializersStyle, 4>
    pack_constructor_initializers_style{
        {clang_v16::FormatStyle::PackConstructorInitializersStyle::PCIS_Never,
         clang_v17::FormatStyle::PackConstructorInitializersStyle::PCIS_Never},
        {clang_v16::FormatStyle::PackConstructorInitializersStyle::PCIS_BinPack,
         clang_v17::FormatStyle::PackConstructorInitializersStyle::
             PCIS_BinPack},
        {clang_v16::FormatStyle::PackConstructorInitializersStyle::
             PCIS_CurrentLine,
         clang_v17::FormatStyle::PackConstructorInitializersStyle::
             PCIS_CurrentLine},
        {clang_v16::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLine,
         clang_v17::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLine}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::PointerAlignmentStyle,
                                clang_v17::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v16::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v17::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v16::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v17::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v16::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v17::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::QualifierAlignmentStyle,
                                clang_v17::FormatStyle::QualifierAlignmentStyle,
                                4>
    qualifier_alignment_style{
        {clang_v16::FormatStyle::QualifierAlignmentStyle::QAS_Leave,
         clang_v17::FormatStyle::QualifierAlignmentStyle::QAS_Leave},
        {clang_v16::FormatStyle::QualifierAlignmentStyle::QAS_Left,
         clang_v17::FormatStyle::QualifierAlignmentStyle::QAS_Left},
        {clang_v16::FormatStyle::QualifierAlignmentStyle::QAS_Right,
         clang_v17::FormatStyle::QualifierAlignmentStyle::QAS_Right},
        {clang_v16::FormatStyle::QualifierAlignmentStyle::QAS_Custom,
         clang_v17::FormatStyle::QualifierAlignmentStyle::QAS_Custom}};

void assign(std::vector<clang_v16::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v17::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v17::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<clang_v16::FormatStyle::ReferenceAlignmentStyle,
                                clang_v17::FormatStyle::ReferenceAlignmentStyle,
                                4>
    reference_alignment_style{
        {clang_v16::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer,
         clang_v17::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer},
        {clang_v16::FormatStyle::ReferenceAlignmentStyle::RAS_Left,
         clang_v17::FormatStyle::ReferenceAlignmentStyle::RAS_Left},
        {clang_v16::FormatStyle::ReferenceAlignmentStyle::RAS_Right,
         clang_v17::FormatStyle::ReferenceAlignmentStyle::RAS_Right},
        {clang_v16::FormatStyle::ReferenceAlignmentStyle::RAS_Middle,
         clang_v17::FormatStyle::ReferenceAlignmentStyle::RAS_Middle}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::RequiresClausePositionStyle,
    clang_v17::FormatStyle::RequiresClausePositionStyle, 4>
    requires_clause_position_style{
        {clang_v16::FormatStyle::RequiresClausePositionStyle::RCPS_OwnLine,
         clang_v17::FormatStyle::RequiresClausePositionStyle::RCPS_OwnLine},
        {clang_v16::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithPreceding,
         clang_v17::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithPreceding},
        {clang_v16::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithFollowing,
         clang_v17::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithFollowing},
        {clang_v16::FormatStyle::RequiresClausePositionStyle::RCPS_SingleLine,
         clang_v17::FormatStyle::RequiresClausePositionStyle::RCPS_SingleLine}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::RequiresExpressionIndentationKind,
    clang_v17::FormatStyle::RequiresExpressionIndentationKind, 2>
    requires_expression_indentation_kind{
        {clang_v16::FormatStyle::RequiresExpressionIndentationKind::
             REI_OuterScope,
         clang_v17::FormatStyle::RequiresExpressionIndentationKind::
             REI_OuterScope},
        {clang_v16::FormatStyle::RequiresExpressionIndentationKind::REI_Keyword,
         clang_v17::FormatStyle::RequiresExpressionIndentationKind::
             REI_Keyword}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::SeparateDefinitionStyle,
                                clang_v17::FormatStyle::SeparateDefinitionStyle,
                                3>
    separate_definitions_style{
        {clang_v16::FormatStyle::SeparateDefinitionStyle::SDS_Leave,
         clang_v17::FormatStyle::SeparateDefinitionStyle::SDS_Leave},
        {clang_v16::FormatStyle::SeparateDefinitionStyle::SDS_Always,
         clang_v17::FormatStyle::SeparateDefinitionStyle::SDS_Always},
        {clang_v16::FormatStyle::SeparateDefinitionStyle::SDS_Never,
         clang_v17::FormatStyle::SeparateDefinitionStyle::SDS_Never}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::SortIncludesOptions,
                                clang_v17::FormatStyle::SortIncludesOptions, 3>
    sort_includes_options{
        {clang_v16::FormatStyle::SortIncludesOptions::SI_Never,
         clang_v17::FormatStyle::SortIncludesOptions::SI_Never},
        {clang_v16::FormatStyle::SortIncludesOptions::SI_CaseSensitive,
         clang_v17::FormatStyle::SortIncludesOptions::SI_CaseSensitive},
        {clang_v16::FormatStyle::SortIncludesOptions::SI_CaseInsensitive,
         clang_v17::FormatStyle::SortIncludesOptions::SI_CaseInsensitive}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::SortJavaStaticImportOptions,
    clang_v17::FormatStyle::SortJavaStaticImportOptions, 2>
    sort_java_static_import_options{
        {clang_v16::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before,
         clang_v17::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before},
        {clang_v16::FormatStyle::SortJavaStaticImportOptions::SJSIO_After,
         clang_v17::FormatStyle::SortJavaStaticImportOptions::SJSIO_After}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::SortUsingDeclarationsOptions,
    clang_v17::FormatStyle::SortUsingDeclarationsOptions, 3>
    sort_using_declarations_options{
        {clang_v16::FormatStyle::SortUsingDeclarationsOptions::SUD_Never,
         clang_v17::FormatStyle::SortUsingDeclarationsOptions::SUD_Never},
        {clang_v16::FormatStyle::SortUsingDeclarationsOptions::
             SUD_Lexicographic,
         clang_v17::FormatStyle::SortUsingDeclarationsOptions::
             SUD_Lexicographic},
        {clang_v16::FormatStyle::SortUsingDeclarationsOptions::
             SUD_LexicographicNumeric,
         clang_v17::FormatStyle::SortUsingDeclarationsOptions::
             SUD_LexicographicNumeric}};

constexpr frozen::unordered_map<
    clang_v16::FormatStyle::SpaceAroundPointerQualifiersStyle,
    clang_v17::FormatStyle::SpaceAroundPointerQualifiersStyle, 4>
    space_around_pointer_qualifiers_style{
        {clang_v16::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default,
         clang_v17::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default},
        {clang_v16::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Before,
         clang_v17::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Before},
        {clang_v16::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After,
         clang_v17::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After},
        {clang_v16::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both,
         clang_v17::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::SpaceBeforeParensStyle,
                                clang_v17::FormatStyle::SpaceBeforeParensStyle,
                                6>
    space_before_parens_options{
        {clang_v16::FormatStyle::SpaceBeforeParensStyle::SBPO_Never,
         clang_v17::FormatStyle::SpaceBeforeParensStyle::SBPO_Never},
        {clang_v16::FormatStyle::SpaceBeforeParensStyle::SBPO_ControlStatements,
         clang_v17::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatements},
        {clang_v16::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros,
         clang_v17::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros},
        {clang_v16::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses,
         clang_v17::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses},
        {clang_v16::FormatStyle::SpaceBeforeParensStyle::SBPO_Always,
         clang_v17::FormatStyle::SpaceBeforeParensStyle::SBPO_Always},
        {clang_v16::FormatStyle::SpaceBeforeParensStyle::SBPO_Custom,
         clang_v17::FormatStyle::SpaceBeforeParensStyle::SBPO_Custom}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::SpacesInAnglesStyle,
                                clang_v17::FormatStyle::SpacesInAnglesStyle, 3>
    spaces_in_angles_style{
        {clang_v16::FormatStyle::SpacesInAnglesStyle::SIAS_Never,
         clang_v17::FormatStyle::SpacesInAnglesStyle::SIAS_Never},
        {clang_v16::FormatStyle::SpacesInAnglesStyle::SIAS_Always,
         clang_v17::FormatStyle::SpacesInAnglesStyle::SIAS_Always},
        {clang_v16::FormatStyle::SpacesInAnglesStyle::SIAS_Leave,
         clang_v17::FormatStyle::SpacesInAnglesStyle::SIAS_Leave}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::LanguageStandard,
                                clang_v17::FormatStyle::LanguageStandard, 7>
    language_standard{{clang_v16::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v17::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v16::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v17::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v16::FormatStyle::LanguageStandard::LS_Cpp14,
                       clang_v17::FormatStyle::LanguageStandard::LS_Cpp14},
                      {clang_v16::FormatStyle::LanguageStandard::LS_Cpp17,
                       clang_v17::FormatStyle::LanguageStandard::LS_Cpp17},
                      {clang_v16::FormatStyle::LanguageStandard::LS_Cpp20,
                       clang_v17::FormatStyle::LanguageStandard::LS_Cpp20},
                      {clang_v16::FormatStyle::LanguageStandard::LS_Latest,
                       clang_v17::FormatStyle::LanguageStandard::LS_Latest},
                      {clang_v16::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v17::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v16::FormatStyle::UseTabStyle,
                                clang_v17::FormatStyle::UseTabStyle, 5>
    use_tab_style{
        {clang_v16::FormatStyle::UseTabStyle::UT_Never,
         clang_v17::FormatStyle::UseTabStyle::UT_Never},
        {clang_v16::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v17::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v16::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v17::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v16::FormatStyle::UseTabStyle::UT_AlignWithSpaces,
         clang_v17::FormatStyle::UseTabStyle::UT_AlignWithSpaces},
        {clang_v16::FormatStyle::UseTabStyle::UT_Always,
         clang_v17::FormatStyle::UseTabStyle::UT_Always}};

clang_v17::FormatStyle update(clang_v16::FormatStyle &old,
                              const std::string &style) {
  clang_v17::FormatStyle retval;
  if (!clang_v17::getPredefinedStyle(
          style, clang_v17::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.InheritsParentConfig = old.InheritsParentConfig;
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignArrayOfStructures =
      array_initializer_alignment_style.at(old.AlignArrayOfStructures);
  retval.AlignConsecutiveMacros.Enabled = old.AlignConsecutiveMacros.Enabled;
  retval.AlignConsecutiveMacros.AcrossEmptyLines =
      old.AlignConsecutiveMacros.AcrossEmptyLines;
  retval.AlignConsecutiveMacros.AcrossComments =
      old.AlignConsecutiveMacros.AcrossComments;
  retval.AlignConsecutiveMacros.AlignCompound =
      old.AlignConsecutiveMacros.AlignCompound;
  retval.AlignConsecutiveMacros.PadOperators =
      old.AlignConsecutiveMacros.PadOperators;
  retval.AlignConsecutiveAssignments.Enabled =
      old.AlignConsecutiveAssignments.Enabled;
  retval.AlignConsecutiveAssignments.AcrossEmptyLines =
      old.AlignConsecutiveAssignments.AcrossEmptyLines;
  retval.AlignConsecutiveAssignments.AcrossComments =
      old.AlignConsecutiveAssignments.AcrossComments;
  retval.AlignConsecutiveAssignments.AlignCompound =
      old.AlignConsecutiveAssignments.AlignCompound;
  retval.AlignConsecutiveAssignments.PadOperators =
      old.AlignConsecutiveAssignments.PadOperators;
  retval.AlignConsecutiveBitFields.Enabled =
      old.AlignConsecutiveBitFields.Enabled;
  retval.AlignConsecutiveBitFields.AcrossEmptyLines =
      old.AlignConsecutiveBitFields.AcrossEmptyLines;
  retval.AlignConsecutiveBitFields.AcrossComments =
      old.AlignConsecutiveBitFields.AcrossComments;
  retval.AlignConsecutiveBitFields.AlignCompound =
      old.AlignConsecutiveBitFields.AlignCompound;
  retval.AlignConsecutiveBitFields.PadOperators =
      old.AlignConsecutiveBitFields.PadOperators;
  retval.AlignConsecutiveDeclarations.Enabled =
      old.AlignConsecutiveDeclarations.Enabled;
  retval.AlignConsecutiveDeclarations.AcrossEmptyLines =
      old.AlignConsecutiveDeclarations.AcrossEmptyLines;
  retval.AlignConsecutiveDeclarations.AcrossComments =
      old.AlignConsecutiveDeclarations.AcrossComments;
  retval.AlignConsecutiveDeclarations.AlignCompound =
      old.AlignConsecutiveDeclarations.AlignCompound;
  retval.AlignConsecutiveDeclarations.PadOperators =
      old.AlignConsecutiveDeclarations.PadOperators;
  newField("AlignConsecutiveShortCaseStatements.Enabled", "17",
           retval.AlignConsecutiveShortCaseStatements.Enabled);
  newField("AlignConsecutiveShortCaseStatements.AcrossEmptyLines", "17",
           retval.AlignConsecutiveShortCaseStatements.AcrossEmptyLines);
  newField("AlignConsecutiveShortCaseStatements.AcrossComments", "17",
           retval.AlignConsecutiveShortCaseStatements.AcrossComments);
  newField("AlignConsecutiveShortCaseStatements.AlignCaseColons", "17",
           retval.AlignConsecutiveShortCaseStatements.AlignCaseColons);
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  retval.AlignOperands = operand_alignment_style.at(old.AlignOperands);
  retval.AlignTrailingComments.Kind =
      trailing_comments_alignment_kinds.at(old.AlignTrailingComments.Kind);
  retval.AlignTrailingComments.OverEmptyLines =
      old.AlignTrailingComments.OverEmptyLines;
  retval.AllowAllArgumentsOnNextLine = old.AllowAllArgumentsOnNextLine;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowShortBlocksOnASingleLine =
      short_block_style.at(old.AllowShortBlocksOnASingleLine);
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  retval.AllowShortEnumsOnASingleLine = old.AllowShortEnumsOnASingleLine;
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
  retval.AttributeMacros = old.AttributeMacros;
  retval.BinPackArguments = old.BinPackArguments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.BitFieldColonSpacing =
      bite_field_colon_spacing_style.at(old.BitFieldColonSpacing);
  newField("BracedInitializerIndentWidth", "17",
           retval.BracedInitializerIndentWidth);
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
  retval.BraceWrapping.BeforeLambdaBody = old.BraceWrapping.BeforeLambdaBody;
  retval.BraceWrapping.BeforeWhile = old.BraceWrapping.BeforeWhile;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakAfterAttributes =
      attribute_breaking_style.at(old.BreakAfterAttributes);
  retval.BreakArrays = old.BreakArrays;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  retval.BreakBeforeConceptDeclarations =
      break_before_concept_declarations_style.at(
          old.BreakBeforeConceptDeclarations);
  retval.BreakBeforeInlineASMColon =
      break_before_inline_asm_colon_style.at(old.BreakBeforeInlineASMColon);
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
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.EmptyLineAfterAccessModifier =
      empty_line_after_access_modifier_style.at(
          old.EmptyLineAfterAccessModifier);
  retval.EmptyLineBeforeAccessModifier =
      empty_line_before_access_modifier_style.at(
          old.EmptyLineBeforeAccessModifier);
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  retval.IncludeStyle.IncludeIsMainSourceRegex =
      old.IncludeStyle.IncludeIsMainSourceRegex;
  retval.IfMacros = old.IfMacros;
  retval.IndentAccessModifiers = old.IndentAccessModifiers;
  retval.IndentCaseBlocks = old.IndentCaseBlocks;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentGotoLabels = old.IndentGotoLabels;
  retval.IndentExternBlock =
      indent_extern_block_style.at(old.IndentExternBlock);
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  retval.IndentRequiresClause = old.IndentRequiresClause;
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.InsertBraces = old.InsertBraces;
  retval.InsertNewlineAtEOF = old.InsertNewlineAtEOF;
  retval.InsertTrailingCommas =
      trailing_comma_style.at(old.InsertTrailingCommas);
  retval.IntegerLiteralSeparator.Binary = old.IntegerLiteralSeparator.Binary;
  retval.IntegerLiteralSeparator.BinaryMinDigits =
      old.IntegerLiteralSeparator.BinaryMinDigits;
  retval.IntegerLiteralSeparator.Decimal = old.IntegerLiteralSeparator.Decimal;
  retval.IntegerLiteralSeparator.DecimalMinDigits =
      old.IntegerLiteralSeparator.DecimalMinDigits;
  retval.IntegerLiteralSeparator.Hex = old.IntegerLiteralSeparator.Hex;
  retval.IntegerLiteralSeparator.HexMinDigits =
      old.IntegerLiteralSeparator.HexMinDigits;
  retval.JavaImportGroups = old.JavaImportGroups;
  retval.JavaScriptQuotes = java_script_quote_style.at(old.JavaScriptQuotes);
  retval.JavaScriptWrapImports = old.JavaScriptWrapImports;
  newField("KeepEmptyLinesAtEOF", "17", retval.KeepEmptyLinesAtEOF);
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.LambdaBodyIndentation =
      lambda_body_indentation_king.at(old.LambdaBodyIndentation);
  retval.Language = language_king.at(old.Language);
  retval.LineEnding = line_ending_style.at(old.LineEnding);
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  newField("Macros", "17", retval.Macros);
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.NamespaceMacros = old.NamespaceMacros;
  retval.ObjCBinPackProtocolList =
      bin_pack_style.at(old.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCBreakBeforeNestedBlockParam = old.ObjCBreakBeforeNestedBlockParam;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PackConstructorInitializers =
      pack_constructor_initializers_style.at(old.PackConstructorInitializers);
  improveField("PackConstructorInitializers", "NextLineOnly", "17");
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakOpenParenthesis = old.PenaltyBreakOpenParenthesis;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyBreakTemplateDeclaration = old.PenaltyBreakTemplateDeclaration;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyIndentedWhitespace = old.PenaltyIndentedWhitespace;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.PPIndentWidth = old.PPIndentWidth;
  retval.QualifierAlignment =
      qualifier_alignment_style.at(old.QualifierAlignment);
  retval.QualifierOrder = old.QualifierOrder;
  assign(old.RawStringFormats, retval.RawStringFormats);
  retval.ReferenceAlignment =
      reference_alignment_style.at(old.ReferenceAlignment);
  retval.ReflowComments = old.ReflowComments;
  retval.RemoveBracesLLVM = old.RemoveBracesLLVM;
  newField("RemoveParentheses", "17", retval.RemoveParentheses);
  retval.RemoveSemicolon = old.RemoveSemicolon;
  retval.RequiresClausePosition =
      requires_clause_position_style.at(old.RequiresClausePosition);
  retval.RequiresExpressionIndentation =
      requires_expression_indentation_kind.at(
          old.RequiresExpressionIndentation);
  retval.SeparateDefinitionBlocks =
      separate_definitions_style.at(old.SeparateDefinitionBlocks);
  retval.ShortNamespaceLines = old.ShortNamespaceLines;
  retval.SortIncludes = sort_includes_options.at(old.SortIncludes);
  retval.SortJavaStaticImport =
      sort_java_static_import_options.at(old.SortJavaStaticImport);
  retval.SortUsingDeclarations =
      sort_using_declarations_options.at(old.SortUsingDeclarations);
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterLogicalNot = old.SpaceAfterLogicalNot;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceAroundPointerQualifiers =
      space_around_pointer_qualifiers_style.at(
          old.SpaceAroundPointerQualifiers);
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeCaseColon = old.SpaceBeforeCaseColon;
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  newField("SpaceBeforeJsonColon", "17", retval.SpaceBeforeJsonColon);
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceBeforeParensOptions.AfterControlStatements =
      old.SpaceBeforeParensOptions.AfterControlStatements;
  retval.SpaceBeforeParensOptions.AfterForeachMacros =
      old.SpaceBeforeParensOptions.AfterForeachMacros;
  retval.SpaceBeforeParensOptions.AfterFunctionDeclarationName =
      old.SpaceBeforeParensOptions.AfterFunctionDeclarationName;
  retval.SpaceBeforeParensOptions.AfterFunctionDefinitionName =
      old.SpaceBeforeParensOptions.AfterFunctionDefinitionName;
  retval.SpaceBeforeParensOptions.AfterIfMacros =
      old.SpaceBeforeParensOptions.AfterIfMacros;
  retval.SpaceBeforeParensOptions.AfterOverloadedOperator =
      old.SpaceBeforeParensOptions.AfterOverloadedOperator;
  retval.SpaceBeforeParensOptions.AfterRequiresInClause =
      old.SpaceBeforeParensOptions.AfterRequiresInClause;
  retval.SpaceBeforeParensOptions.AfterRequiresInExpression =
      old.SpaceBeforeParensOptions.AfterRequiresInExpression;
  retval.SpaceBeforeParensOptions.BeforeNonEmptyParentheses =
      old.SpaceBeforeParensOptions.BeforeNonEmptyParentheses;
  retval.SpaceBeforeSquareBrackets = old.SpaceBeforeSquareBrackets;
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
  retval.SpaceInEmptyBlock = old.SpaceInEmptyBlock;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = spaces_in_angles_style.at(old.SpacesInAngles);
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInLineCommentPrefix.Minimum =
      old.SpacesInLineCommentPrefix.Minimum;
  retval.SpacesInLineCommentPrefix.Maximum =
      old.SpacesInLineCommentPrefix.Maximum;
  retval.SpacesInParens =
      old.SpacesInParentheses
          ? clang_v17::FormatStyle::SpacesInParensStyle::SIPO_Custom
          : clang_v17::FormatStyle::SpacesInParensStyle::SIPO_Never;
  retval.SpacesInParensOptions.InEmptyParentheses = old.SpaceInEmptyParentheses;
  retval.SpacesInParensOptions.InConditionalStatements =
      old.SpacesInConditionalStatement;
  retval.SpacesInParensOptions.InCStyleCasts =
      old.SpacesInCStyleCastParentheses;
  newField("SpacesInParensOptions.Other", "17",
           retval.SpacesInParensOptions.Other);
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.Standard = language_standard.at(old.Standard);
  retval.StatementAttributeLikeMacros = old.StatementAttributeLikeMacros;
  retval.StatementMacros = old.StatementMacros;
  retval.TabWidth = old.TabWidth;
  newField("TypeNames", "17", retval.TypeNames);
  retval.TypenameMacros = old.TypenameMacros;
  retval.UseTab = use_tab_style.at(old.UseTab);
  newField("VerilogBreakBetweenInstancePorts", "17",
           retval.VerilogBreakBetweenInstancePorts);
  retval.WhitespaceSensitiveMacros = old.WhitespaceSensitiveMacros;

  return retval;
}

} // namespace clang_update_v17

namespace clang_update_v18 {

constexpr frozen::unordered_map<clang_v17::FormatStyle::BracketAlignmentStyle,
                                clang_v18::FormatStyle::BracketAlignmentStyle,
                                4>
    bracket_all_alignment_style{
        {clang_v17::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v18::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v17::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v18::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v17::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v18::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak},
        {clang_v17::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent,
         clang_v18::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::ArrayInitializerAlignmentStyle,
    clang_v18::FormatStyle::ArrayInitializerAlignmentStyle, 3>
    array_initializer_alignment_style{
        {clang_v17::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left,
         clang_v18::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left},
        {clang_v17::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right,
         clang_v18::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right},
        {clang_v17::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None,
         clang_v18::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v18::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v17::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v18::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v17::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v18::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v17::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v18::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::OperandAlignmentStyle,
                                clang_v18::FormatStyle::OperandAlignmentStyle,
                                3>
    operand_alignment_style{
        {clang_v17::FormatStyle::OperandAlignmentStyle::OAS_DontAlign,
         clang_v18::FormatStyle::OperandAlignmentStyle::OAS_DontAlign},
        {clang_v17::FormatStyle::OperandAlignmentStyle::OAS_Align,
         clang_v18::FormatStyle::OperandAlignmentStyle::OAS_Align},
        {clang_v17::FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator,
         clang_v18::FormatStyle::OperandAlignmentStyle::
             OAS_AlignAfterOperator}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::TrailingCommentsAlignmentKinds,
    clang_v18::FormatStyle::TrailingCommentsAlignmentKinds, 3>
    trailing_comments_alignment_kinds{
        {clang_v17::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Leave,
         clang_v18::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Leave},
        {clang_v17::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Always,
         clang_v18::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Always},
        {clang_v17::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Never,
         clang_v18::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Never}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::ShortBlockStyle,
                                clang_v18::FormatStyle::ShortBlockStyle, 3>
    short_block_style{{clang_v17::FormatStyle::ShortBlockStyle::SBS_Never,
                       clang_v18::FormatStyle::ShortBlockStyle::SBS_Never},
                      {clang_v17::FormatStyle::ShortBlockStyle::SBS_Empty,
                       clang_v18::FormatStyle::ShortBlockStyle::SBS_Empty},
                      {clang_v17::FormatStyle::ShortBlockStyle::SBS_Always,
                       clang_v18::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::ShortFunctionStyle,
                                clang_v18::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v17::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v18::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v17::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v18::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v17::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v18::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v17::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v18::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v17::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v18::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::ShortIfStyle,
                                clang_v18::FormatStyle::ShortIfStyle, 4>
    short_if_style{{clang_v17::FormatStyle::ShortIfStyle::SIS_Never,
                    clang_v18::FormatStyle::ShortIfStyle::SIS_Never},
                   {clang_v17::FormatStyle::ShortIfStyle::SIS_WithoutElse,
                    clang_v18::FormatStyle::ShortIfStyle::SIS_WithoutElse},
                   {clang_v17::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf,
                    clang_v18::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf},
                   {clang_v17::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse,
                    clang_v18::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::ShortLambdaStyle,
                                clang_v18::FormatStyle::ShortLambdaStyle, 4>
    short_lambda_style{{clang_v17::FormatStyle::ShortLambdaStyle::SLS_None,
                        clang_v18::FormatStyle::ShortLambdaStyle::SLS_None},
                       {clang_v17::FormatStyle::ShortLambdaStyle::SLS_Empty,
                        clang_v18::FormatStyle::ShortLambdaStyle::SLS_Empty},
                       {clang_v17::FormatStyle::ShortLambdaStyle::SLS_Inline,
                        clang_v18::FormatStyle::ShortLambdaStyle::SLS_Inline},
                       {clang_v17::FormatStyle::ShortLambdaStyle::SLS_All,
                        clang_v18::FormatStyle::ShortLambdaStyle::SLS_All}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v18::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v17::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v18::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v17::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v18::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v17::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v18::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v18::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v17::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v18::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v17::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v18::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v17::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v18::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v17::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v18::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v17::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v18::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v18::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v17::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v18::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v17::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v18::FormatStyle::BreakTemplateDeclarationsStyle::
             BTDS_MultiLine},
        {clang_v17::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v18::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::BitFieldColonSpacingStyle,
    clang_v18::FormatStyle::BitFieldColonSpacingStyle, 4>
    bite_field_colon_spacing_style{
        {clang_v17::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both,
         clang_v18::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both},
        {clang_v17::FormatStyle::BitFieldColonSpacingStyle::BFCS_None,
         clang_v18::FormatStyle::BitFieldColonSpacingStyle::BFCS_None},
        {clang_v17::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before,
         clang_v18::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before},
        {clang_v17::FormatStyle::BitFieldColonSpacingStyle::BFCS_After,
         clang_v18::FormatStyle::BitFieldColonSpacingStyle::BFCS_After}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::BraceWrappingAfterControlStatementStyle,
    clang_v18::FormatStyle::BraceWrappingAfterControlStatementStyle, 3>
    brace_wrapping_after_control_statement_style{
        {clang_v17::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never,
         clang_v18::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never},
        {clang_v17::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine,
         clang_v18::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine},
        {clang_v17::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always,
         clang_v18::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::AttributeBreakingStyle,
                                clang_v18::FormatStyle::AttributeBreakingStyle,
                                3>
    attribute_breaking_style{
        {clang_v17::FormatStyle::AttributeBreakingStyle::ABS_Always,
         clang_v18::FormatStyle::AttributeBreakingStyle::ABS_Always},
        {clang_v17::FormatStyle::AttributeBreakingStyle::ABS_Leave,
         clang_v18::FormatStyle::AttributeBreakingStyle::ABS_Leave},
        {clang_v17::FormatStyle::AttributeBreakingStyle::ABS_Never,
         clang_v18::FormatStyle::AttributeBreakingStyle::ABS_Never}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::BinaryOperatorStyle,
                                clang_v18::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v17::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v18::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v17::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v18::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v17::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v18::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::BraceBreakingStyle,
                                clang_v18::FormatStyle::BraceBreakingStyle, 9>
    brace_breaking_style{
        {clang_v17::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v18::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v17::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v18::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v17::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v18::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v17::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v18::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v17::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v18::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v17::FormatStyle::BraceBreakingStyle::BS_Whitesmiths,
         clang_v18::FormatStyle::BraceBreakingStyle::BS_Whitesmiths},
        {clang_v17::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v18::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v17::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v18::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v17::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v18::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::BreakBeforeConceptDeclarationsStyle,
    clang_v18::FormatStyle::BreakBeforeConceptDeclarationsStyle, 3>
    break_before_concept_declarations_style{
        {clang_v17::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Never,
         clang_v18::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Never},
        {clang_v17::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Allowed,
         clang_v18::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Allowed},
        {clang_v17::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Always,
         clang_v18::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Always}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::BreakBeforeInlineASMColonStyle,
    clang_v18::FormatStyle::BreakBeforeInlineASMColonStyle, 3>
    break_before_inline_asm_colon_style{
        {clang_v17::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Never,
         clang_v18::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Never},
        {clang_v17::FormatStyle::BreakBeforeInlineASMColonStyle::
             BBIAS_OnlyMultiline,
         clang_v18::FormatStyle::BreakBeforeInlineASMColonStyle::
             BBIAS_OnlyMultiline},
        {clang_v17::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Always,
         clang_v18::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Always}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::BreakConstructorInitializersStyle,
    clang_v18::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v17::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v18::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v17::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v18::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v17::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v18::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::BreakInheritanceListStyle,
    clang_v18::FormatStyle::BreakInheritanceListStyle, 4>
    break_inheritance_list_style{
        {clang_v17::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v18::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v17::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v18::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v17::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v18::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon},
        {clang_v17::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma,
         clang_v18::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::EmptyLineAfterAccessModifierStyle,
    clang_v18::FormatStyle::EmptyLineAfterAccessModifierStyle, 3>
    empty_line_after_access_modifier_style{
        {clang_v17::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never,
         clang_v18::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never},
        {clang_v17::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave,
         clang_v18::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave},
        {clang_v17::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always,
         clang_v18::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::EmptyLineBeforeAccessModifierStyle,
    clang_v18::FormatStyle::EmptyLineBeforeAccessModifierStyle, 4>
    empty_line_before_access_modifier_style{
        {clang_v17::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never,
         clang_v18::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never},
        {clang_v17::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave,
         clang_v18::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave},
        {clang_v17::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock,
         clang_v18::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock},
        {clang_v17::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always,
         clang_v18::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always}};

constexpr frozen::unordered_map<clang_v17::IncludeStyle::IncludeBlocksStyle,
                                clang_v18::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v17::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v18::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v17::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v18::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v17::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v18::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v17::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v18::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v18::IncludeStyle::IncludeCategory{
        item.Regex, item.Priority, item.SortPriority, true});
  }
}

constexpr frozen::unordered_map<clang_v17::FormatStyle::IndentExternBlockStyle,
                                clang_v18::FormatStyle::IndentExternBlockStyle,
                                3>
    indent_extern_block_style{
        {clang_v17::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock,
         clang_v18::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock},
        {clang_v17::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent,
         clang_v18::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent},
        {clang_v17::FormatStyle::IndentExternBlockStyle::IEBS_Indent,
         clang_v18::FormatStyle::IndentExternBlockStyle::IEBS_Indent}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::PPDirectiveIndentStyle,
                                clang_v18::FormatStyle::PPDirectiveIndentStyle,
                                3>
    pp_directive_indent_style{
        {clang_v17::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v18::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v17::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v18::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash},
        {clang_v17::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash,
         clang_v18::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::TrailingCommaStyle,
                                clang_v18::FormatStyle::TrailingCommaStyle, 2>
    trailing_comma_style{
        {clang_v17::FormatStyle::TrailingCommaStyle::TCS_None,
         clang_v18::FormatStyle::TrailingCommaStyle::TCS_None},
        {clang_v17::FormatStyle::TrailingCommaStyle::TCS_Wrapped,
         clang_v18::FormatStyle::TrailingCommaStyle::TCS_Wrapped}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::JavaScriptQuoteStyle,
                                clang_v18::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v17::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v18::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v17::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v18::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v17::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v18::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::LambdaBodyIndentationKind,
    clang_v18::FormatStyle::LambdaBodyIndentationKind, 2>
    lambda_body_indentation_king{
        {clang_v17::FormatStyle::LambdaBodyIndentationKind::LBI_Signature,
         clang_v18::FormatStyle::LambdaBodyIndentationKind::LBI_Signature},
        {clang_v17::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope,
         clang_v18::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::LanguageKind,
                                clang_v18::FormatStyle::LanguageKind, 11>
    language_king{{clang_v17::FormatStyle::LanguageKind::LK_None,
                   clang_v18::FormatStyle::LanguageKind::LK_None},
                  {clang_v17::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v18::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v17::FormatStyle::LanguageKind::LK_CSharp,
                   clang_v18::FormatStyle::LanguageKind::LK_CSharp},
                  {clang_v17::FormatStyle::LanguageKind::LK_Java,
                   clang_v18::FormatStyle::LanguageKind::LK_Java},
                  {clang_v17::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v18::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v17::FormatStyle::LanguageKind::LK_Json,
                   clang_v18::FormatStyle::LanguageKind::LK_Json},
                  {clang_v17::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v18::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v17::FormatStyle::LanguageKind::LK_Proto,
                   clang_v18::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v17::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v18::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v17::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v18::FormatStyle::LanguageKind::LK_TextProto},
                  {clang_v17::FormatStyle::LanguageKind::LK_Verilog,
                   clang_v18::FormatStyle::LanguageKind::LK_Verilog}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::LineEndingStyle,
                                clang_v18::FormatStyle::LineEndingStyle, 4>
    line_ending_style{{clang_v17::FormatStyle::LineEndingStyle::LE_LF,
                       clang_v18::FormatStyle::LineEndingStyle::LE_LF},
                      {clang_v17::FormatStyle::LineEndingStyle::LE_CRLF,
                       clang_v18::FormatStyle::LineEndingStyle::LE_CRLF},
                      {clang_v17::FormatStyle::LineEndingStyle::LE_DeriveLF,
                       clang_v18::FormatStyle::LineEndingStyle::LE_DeriveLF},
                      {clang_v17::FormatStyle::LineEndingStyle::LE_DeriveCRLF,
                       clang_v18::FormatStyle::LineEndingStyle::LE_DeriveCRLF}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::NamespaceIndentationKind,
    clang_v18::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v17::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v18::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v17::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v18::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v17::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v18::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::BinPackStyle,
                                clang_v18::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v17::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v18::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v17::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v18::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v17::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v18::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::PackConstructorInitializersStyle,
    clang_v18::FormatStyle::PackConstructorInitializersStyle, 5>
    pack_constructor_initializers_style{
        {clang_v17::FormatStyle::PackConstructorInitializersStyle::PCIS_Never,
         clang_v18::FormatStyle::PackConstructorInitializersStyle::PCIS_Never},
        {clang_v17::FormatStyle::PackConstructorInitializersStyle::PCIS_BinPack,
         clang_v18::FormatStyle::PackConstructorInitializersStyle::
             PCIS_BinPack},
        {clang_v17::FormatStyle::PackConstructorInitializersStyle::
             PCIS_CurrentLine,
         clang_v18::FormatStyle::PackConstructorInitializersStyle::
             PCIS_CurrentLine},
        {clang_v17::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLine,
         clang_v18::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLine},
        {clang_v17::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLineOnly,
         clang_v18::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLineOnly}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::PointerAlignmentStyle,
                                clang_v18::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v17::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v18::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v17::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v18::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v17::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v18::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::QualifierAlignmentStyle,
                                clang_v18::FormatStyle::QualifierAlignmentStyle,
                                4>
    qualifier_alignment_style{
        {clang_v17::FormatStyle::QualifierAlignmentStyle::QAS_Leave,
         clang_v18::FormatStyle::QualifierAlignmentStyle::QAS_Leave},
        {clang_v17::FormatStyle::QualifierAlignmentStyle::QAS_Left,
         clang_v18::FormatStyle::QualifierAlignmentStyle::QAS_Left},
        {clang_v17::FormatStyle::QualifierAlignmentStyle::QAS_Right,
         clang_v18::FormatStyle::QualifierAlignmentStyle::QAS_Right},
        {clang_v17::FormatStyle::QualifierAlignmentStyle::QAS_Custom,
         clang_v18::FormatStyle::QualifierAlignmentStyle::QAS_Custom}};

void assign(std::vector<clang_v17::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v18::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v18::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<clang_v17::FormatStyle::ReferenceAlignmentStyle,
                                clang_v18::FormatStyle::ReferenceAlignmentStyle,
                                4>
    reference_alignment_style{
        {clang_v17::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer,
         clang_v18::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer},
        {clang_v17::FormatStyle::ReferenceAlignmentStyle::RAS_Left,
         clang_v18::FormatStyle::ReferenceAlignmentStyle::RAS_Left},
        {clang_v17::FormatStyle::ReferenceAlignmentStyle::RAS_Right,
         clang_v18::FormatStyle::ReferenceAlignmentStyle::RAS_Right},
        {clang_v17::FormatStyle::ReferenceAlignmentStyle::RAS_Middle,
         clang_v18::FormatStyle::ReferenceAlignmentStyle::RAS_Middle}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::RemoveParenthesesStyle,
                                clang_v18::FormatStyle::RemoveParenthesesStyle,
                                3>
    remove_parentheses_style{
        {clang_v17::FormatStyle::RemoveParenthesesStyle::RPS_Leave,
         clang_v18::FormatStyle::RemoveParenthesesStyle::RPS_Leave},
        {clang_v17::FormatStyle::RemoveParenthesesStyle::
             RPS_MultipleParentheses,
         clang_v18::FormatStyle::RemoveParenthesesStyle::
             RPS_MultipleParentheses},
        {clang_v17::FormatStyle::RemoveParenthesesStyle::RPS_ReturnStatement,
         clang_v18::FormatStyle::RemoveParenthesesStyle::RPS_ReturnStatement}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::RequiresClausePositionStyle,
    clang_v18::FormatStyle::RequiresClausePositionStyle, 4>
    requires_clause_position_style{
        {clang_v17::FormatStyle::RequiresClausePositionStyle::RCPS_OwnLine,
         clang_v18::FormatStyle::RequiresClausePositionStyle::RCPS_OwnLine},
        {clang_v17::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithPreceding,
         clang_v18::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithPreceding},
        {clang_v17::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithFollowing,
         clang_v18::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithFollowing},
        {clang_v17::FormatStyle::RequiresClausePositionStyle::RCPS_SingleLine,
         clang_v18::FormatStyle::RequiresClausePositionStyle::RCPS_SingleLine}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::RequiresExpressionIndentationKind,
    clang_v18::FormatStyle::RequiresExpressionIndentationKind, 2>
    requires_expression_indentation_kind{
        {clang_v17::FormatStyle::RequiresExpressionIndentationKind::
             REI_OuterScope,
         clang_v18::FormatStyle::RequiresExpressionIndentationKind::
             REI_OuterScope},
        {clang_v17::FormatStyle::RequiresExpressionIndentationKind::REI_Keyword,
         clang_v18::FormatStyle::RequiresExpressionIndentationKind::
             REI_Keyword}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::SeparateDefinitionStyle,
                                clang_v18::FormatStyle::SeparateDefinitionStyle,
                                3>
    separate_definitions_style{
        {clang_v17::FormatStyle::SeparateDefinitionStyle::SDS_Leave,
         clang_v18::FormatStyle::SeparateDefinitionStyle::SDS_Leave},
        {clang_v17::FormatStyle::SeparateDefinitionStyle::SDS_Always,
         clang_v18::FormatStyle::SeparateDefinitionStyle::SDS_Always},
        {clang_v17::FormatStyle::SeparateDefinitionStyle::SDS_Never,
         clang_v18::FormatStyle::SeparateDefinitionStyle::SDS_Never}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::SortIncludesOptions,
                                clang_v18::FormatStyle::SortIncludesOptions, 3>
    sort_includes_options{
        {clang_v17::FormatStyle::SortIncludesOptions::SI_Never,
         clang_v18::FormatStyle::SortIncludesOptions::SI_Never},
        {clang_v17::FormatStyle::SortIncludesOptions::SI_CaseSensitive,
         clang_v18::FormatStyle::SortIncludesOptions::SI_CaseSensitive},
        {clang_v17::FormatStyle::SortIncludesOptions::SI_CaseInsensitive,
         clang_v18::FormatStyle::SortIncludesOptions::SI_CaseInsensitive}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::SortJavaStaticImportOptions,
    clang_v18::FormatStyle::SortJavaStaticImportOptions, 2>
    sort_java_static_import_options{
        {clang_v17::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before,
         clang_v18::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before},
        {clang_v17::FormatStyle::SortJavaStaticImportOptions::SJSIO_After,
         clang_v18::FormatStyle::SortJavaStaticImportOptions::SJSIO_After}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::SortUsingDeclarationsOptions,
    clang_v18::FormatStyle::SortUsingDeclarationsOptions, 3>
    sort_using_declarations_options{
        {clang_v17::FormatStyle::SortUsingDeclarationsOptions::SUD_Never,
         clang_v18::FormatStyle::SortUsingDeclarationsOptions::SUD_Never},
        {clang_v17::FormatStyle::SortUsingDeclarationsOptions::
             SUD_Lexicographic,
         clang_v18::FormatStyle::SortUsingDeclarationsOptions::
             SUD_Lexicographic},
        {clang_v17::FormatStyle::SortUsingDeclarationsOptions::
             SUD_LexicographicNumeric,
         clang_v18::FormatStyle::SortUsingDeclarationsOptions::
             SUD_LexicographicNumeric}};

constexpr frozen::unordered_map<
    clang_v17::FormatStyle::SpaceAroundPointerQualifiersStyle,
    clang_v18::FormatStyle::SpaceAroundPointerQualifiersStyle, 4>
    space_around_pointer_qualifiers_style{
        {clang_v17::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default,
         clang_v18::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default},
        {clang_v17::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Before,
         clang_v18::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Before},
        {clang_v17::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After,
         clang_v18::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After},
        {clang_v17::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both,
         clang_v18::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::SpaceBeforeParensStyle,
                                clang_v18::FormatStyle::SpaceBeforeParensStyle,
                                6>
    space_before_parens_options{
        {clang_v17::FormatStyle::SpaceBeforeParensStyle::SBPO_Never,
         clang_v18::FormatStyle::SpaceBeforeParensStyle::SBPO_Never},
        {clang_v17::FormatStyle::SpaceBeforeParensStyle::SBPO_ControlStatements,
         clang_v18::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatements},
        {clang_v17::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros,
         clang_v18::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros},
        {clang_v17::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses,
         clang_v18::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses},
        {clang_v17::FormatStyle::SpaceBeforeParensStyle::SBPO_Always,
         clang_v18::FormatStyle::SpaceBeforeParensStyle::SBPO_Always},
        {clang_v17::FormatStyle::SpaceBeforeParensStyle::SBPO_Custom,
         clang_v18::FormatStyle::SpaceBeforeParensStyle::SBPO_Custom}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::SpacesInAnglesStyle,
                                clang_v18::FormatStyle::SpacesInAnglesStyle, 3>
    spaces_in_angles_style{
        {clang_v17::FormatStyle::SpacesInAnglesStyle::SIAS_Never,
         clang_v18::FormatStyle::SpacesInAnglesStyle::SIAS_Never},
        {clang_v17::FormatStyle::SpacesInAnglesStyle::SIAS_Always,
         clang_v18::FormatStyle::SpacesInAnglesStyle::SIAS_Always},
        {clang_v17::FormatStyle::SpacesInAnglesStyle::SIAS_Leave,
         clang_v18::FormatStyle::SpacesInAnglesStyle::SIAS_Leave}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::SpacesInParensStyle,
                                clang_v18::FormatStyle::SpacesInParensStyle, 2>
    spaces_in_parens_style{
        {clang_v17::FormatStyle::SpacesInParensStyle::SIPO_Never,
         clang_v18::FormatStyle::SpacesInParensStyle::SIPO_Never},
        {clang_v17::FormatStyle::SpacesInParensStyle::SIPO_Custom,
         clang_v18::FormatStyle::SpacesInParensStyle::SIPO_Custom}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::LanguageStandard,
                                clang_v18::FormatStyle::LanguageStandard, 7>
    language_standard{{clang_v17::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v18::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v17::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v18::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v17::FormatStyle::LanguageStandard::LS_Cpp14,
                       clang_v18::FormatStyle::LanguageStandard::LS_Cpp14},
                      {clang_v17::FormatStyle::LanguageStandard::LS_Cpp17,
                       clang_v18::FormatStyle::LanguageStandard::LS_Cpp17},
                      {clang_v17::FormatStyle::LanguageStandard::LS_Cpp20,
                       clang_v18::FormatStyle::LanguageStandard::LS_Cpp20},
                      {clang_v17::FormatStyle::LanguageStandard::LS_Latest,
                       clang_v18::FormatStyle::LanguageStandard::LS_Latest},
                      {clang_v17::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v18::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v17::FormatStyle::UseTabStyle,
                                clang_v18::FormatStyle::UseTabStyle, 5>
    use_tab_style{
        {clang_v17::FormatStyle::UseTabStyle::UT_Never,
         clang_v18::FormatStyle::UseTabStyle::UT_Never},
        {clang_v17::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v18::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v17::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v18::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v17::FormatStyle::UseTabStyle::UT_AlignWithSpaces,
         clang_v18::FormatStyle::UseTabStyle::UT_AlignWithSpaces},
        {clang_v17::FormatStyle::UseTabStyle::UT_Always,
         clang_v18::FormatStyle::UseTabStyle::UT_Always}};

clang_v18::FormatStyle update(clang_v17::FormatStyle &old,
                              const std::string &style) {
  clang_v18::FormatStyle retval;
  if (!clang_v18::getPredefinedStyle(
          style, clang_v18::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.InheritsParentConfig = old.InheritsParentConfig;
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignArrayOfStructures =
      array_initializer_alignment_style.at(old.AlignArrayOfStructures);
  retval.AlignConsecutiveMacros.Enabled = old.AlignConsecutiveMacros.Enabled;
  retval.AlignConsecutiveMacros.AcrossEmptyLines =
      old.AlignConsecutiveMacros.AcrossEmptyLines;
  retval.AlignConsecutiveMacros.AcrossComments =
      old.AlignConsecutiveMacros.AcrossComments;
  retval.AlignConsecutiveMacros.AlignCompound =
      old.AlignConsecutiveMacros.AlignCompound;
  newField("AlignConsecutiveMacros.AlignFunctionPointers", "18",
           retval.AlignConsecutiveMacros.AlignFunctionPointers);
  retval.AlignConsecutiveMacros.PadOperators =
      old.AlignConsecutiveMacros.PadOperators;
  retval.AlignConsecutiveAssignments.Enabled =
      old.AlignConsecutiveAssignments.Enabled;
  retval.AlignConsecutiveAssignments.AcrossEmptyLines =
      old.AlignConsecutiveAssignments.AcrossEmptyLines;
  retval.AlignConsecutiveAssignments.AcrossComments =
      old.AlignConsecutiveAssignments.AcrossComments;
  retval.AlignConsecutiveAssignments.AlignCompound =
      old.AlignConsecutiveAssignments.AlignCompound;
  newField("AlignConsecutiveAssignments.AlignFunctionPointers", "18",
           retval.AlignConsecutiveAssignments.AlignFunctionPointers);
  retval.AlignConsecutiveAssignments.PadOperators =
      old.AlignConsecutiveAssignments.PadOperators;
  retval.AlignConsecutiveBitFields.Enabled =
      old.AlignConsecutiveBitFields.Enabled;
  retval.AlignConsecutiveBitFields.AcrossEmptyLines =
      old.AlignConsecutiveBitFields.AcrossEmptyLines;
  retval.AlignConsecutiveBitFields.AcrossComments =
      old.AlignConsecutiveBitFields.AcrossComments;
  retval.AlignConsecutiveBitFields.AlignCompound =
      old.AlignConsecutiveBitFields.AlignCompound;
  newField("AlignConsecutiveBitFields.AlignFunctionPointers", "18",
           retval.AlignConsecutiveBitFields.AlignFunctionPointers);
  retval.AlignConsecutiveBitFields.PadOperators =
      old.AlignConsecutiveBitFields.PadOperators;
  retval.AlignConsecutiveDeclarations.Enabled =
      old.AlignConsecutiveDeclarations.Enabled;
  retval.AlignConsecutiveDeclarations.AcrossEmptyLines =
      old.AlignConsecutiveDeclarations.AcrossEmptyLines;
  retval.AlignConsecutiveDeclarations.AcrossComments =
      old.AlignConsecutiveDeclarations.AcrossComments;
  retval.AlignConsecutiveDeclarations.AlignCompound =
      old.AlignConsecutiveDeclarations.AlignCompound;
  newField("AlignConsecutiveDeclarations.AlignFunctionPointers", "18",
           retval.AlignConsecutiveDeclarations.AlignFunctionPointers);
  retval.AlignConsecutiveDeclarations.PadOperators =
      old.AlignConsecutiveDeclarations.PadOperators;
  retval.AlignConsecutiveShortCaseStatements.Enabled =
      old.AlignConsecutiveShortCaseStatements.Enabled;
  retval.AlignConsecutiveShortCaseStatements.AcrossEmptyLines =
      old.AlignConsecutiveShortCaseStatements.AcrossEmptyLines;
  retval.AlignConsecutiveShortCaseStatements.AcrossComments =
      old.AlignConsecutiveShortCaseStatements.AcrossComments;
  retval.AlignConsecutiveShortCaseStatements.AlignCaseColons =
      old.AlignConsecutiveShortCaseStatements.AlignCaseColons;
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  retval.AlignOperands = operand_alignment_style.at(old.AlignOperands);
  retval.AlignTrailingComments.Kind =
      trailing_comments_alignment_kinds.at(old.AlignTrailingComments.Kind);
  retval.AlignTrailingComments.OverEmptyLines =
      old.AlignTrailingComments.OverEmptyLines;
  retval.AllowAllArgumentsOnNextLine = old.AllowAllArgumentsOnNextLine;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  newField("AllowBreakBeforeNoexceptSpecifier", "18",
           retval.AllowBreakBeforeNoexceptSpecifier);
  retval.AllowShortBlocksOnASingleLine =
      short_block_style.at(old.AllowShortBlocksOnASingleLine);
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  newField("AllowShortCompoundRequirementOnASingleLine", "18",
           retval.AllowShortCompoundRequirementOnASingleLine);
  retval.AllowShortEnumsOnASingleLine = old.AllowShortEnumsOnASingleLine;
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
  retval.AttributeMacros = old.AttributeMacros;
  retval.BinPackArguments = old.BinPackArguments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.BitFieldColonSpacing =
      bite_field_colon_spacing_style.at(old.BitFieldColonSpacing);
  retval.BracedInitializerIndentWidth = old.BracedInitializerIndentWidth;
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
  retval.BraceWrapping.BeforeLambdaBody = old.BraceWrapping.BeforeLambdaBody;
  retval.BraceWrapping.BeforeWhile = old.BraceWrapping.BeforeWhile;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  newField("BreakAdjacentStringLiterals", "18",
           retval.BreakAdjacentStringLiterals);
  retval.BreakAfterAttributes =
      attribute_breaking_style.at(old.BreakAfterAttributes);
  retval.BreakArrays = old.BreakArrays;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  retval.BreakBeforeConceptDeclarations =
      break_before_concept_declarations_style.at(
          old.BreakBeforeConceptDeclarations);
  retval.BreakBeforeInlineASMColon =
      break_before_inline_asm_colon_style.at(old.BreakBeforeInlineASMColon);
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
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.EmptyLineAfterAccessModifier =
      empty_line_after_access_modifier_style.at(
          old.EmptyLineAfterAccessModifier);
  retval.EmptyLineBeforeAccessModifier =
      empty_line_before_access_modifier_style.at(
          old.EmptyLineBeforeAccessModifier);
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  retval.IncludeStyle.IncludeIsMainSourceRegex =
      old.IncludeStyle.IncludeIsMainSourceRegex;
  retval.IfMacros = old.IfMacros;
  retval.IndentAccessModifiers = old.IndentAccessModifiers;
  retval.IndentCaseBlocks = old.IndentCaseBlocks;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentGotoLabels = old.IndentGotoLabels;
  retval.IndentExternBlock =
      indent_extern_block_style.at(old.IndentExternBlock);
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  retval.IndentRequiresClause = old.IndentRequiresClause;
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.InsertBraces = old.InsertBraces;
  retval.InsertNewlineAtEOF = old.InsertNewlineAtEOF;
  retval.InsertTrailingCommas =
      trailing_comma_style.at(old.InsertTrailingCommas);
  retval.IntegerLiteralSeparator.Binary = old.IntegerLiteralSeparator.Binary;
  retval.IntegerLiteralSeparator.BinaryMinDigits =
      old.IntegerLiteralSeparator.BinaryMinDigits;
  retval.IntegerLiteralSeparator.Decimal = old.IntegerLiteralSeparator.Decimal;
  retval.IntegerLiteralSeparator.DecimalMinDigits =
      old.IntegerLiteralSeparator.DecimalMinDigits;
  retval.IntegerLiteralSeparator.Hex = old.IntegerLiteralSeparator.Hex;
  retval.IntegerLiteralSeparator.HexMinDigits =
      old.IntegerLiteralSeparator.HexMinDigits;
  retval.JavaImportGroups = old.JavaImportGroups;
  retval.JavaScriptQuotes = java_script_quote_style.at(old.JavaScriptQuotes);
  retval.JavaScriptWrapImports = old.JavaScriptWrapImports;
  retval.KeepEmptyLinesAtEOF = old.KeepEmptyLinesAtEOF;
  retval.KeepEmptyLinesAtTheStartOfBlocks =
      old.KeepEmptyLinesAtTheStartOfBlocks;
  retval.LambdaBodyIndentation =
      lambda_body_indentation_king.at(old.LambdaBodyIndentation);
  retval.Language = language_king.at(old.Language);
  retval.LineEnding = line_ending_style.at(old.LineEnding);
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.Macros = old.Macros;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;
  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.NamespaceMacros = old.NamespaceMacros;
  retval.ObjCBinPackProtocolList =
      bin_pack_style.at(old.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCBreakBeforeNestedBlockParam = old.ObjCBreakBeforeNestedBlockParam;
  newField("ObjCPropertyAttributeOrder", "18",
           retval.ObjCPropertyAttributeOrder);
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PackConstructorInitializers =
      pack_constructor_initializers_style.at(old.PackConstructorInitializers);
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakOpenParenthesis = old.PenaltyBreakOpenParenthesis;
  newField("PenaltyBreakScopeResolution", "18",
           retval.PenaltyBreakScopeResolution);
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyBreakTemplateDeclaration = old.PenaltyBreakTemplateDeclaration;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyIndentedWhitespace = old.PenaltyIndentedWhitespace;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.PPIndentWidth = old.PPIndentWidth;
  retval.QualifierAlignment =
      qualifier_alignment_style.at(old.QualifierAlignment);
  retval.QualifierOrder = old.QualifierOrder;
  assign(old.RawStringFormats, retval.RawStringFormats);
  retval.ReferenceAlignment =
      reference_alignment_style.at(old.ReferenceAlignment);
  retval.ReflowComments = old.ReflowComments;
  retval.RemoveBracesLLVM = old.RemoveBracesLLVM;
  retval.RemoveParentheses = remove_parentheses_style.at(old.RemoveParentheses);
  retval.RemoveSemicolon = old.RemoveSemicolon;
  retval.RequiresClausePosition =
      requires_clause_position_style.at(old.RequiresClausePosition);
  retval.RequiresExpressionIndentation =
      requires_expression_indentation_kind.at(
          old.RequiresExpressionIndentation);
  retval.SeparateDefinitionBlocks =
      separate_definitions_style.at(old.SeparateDefinitionBlocks);
  retval.ShortNamespaceLines = old.ShortNamespaceLines;
  newField("SkipMacroDefinitionBody", "18", retval.SkipMacroDefinitionBody);
  retval.SortIncludes = sort_includes_options.at(old.SortIncludes);
  retval.SortJavaStaticImport =
      sort_java_static_import_options.at(old.SortJavaStaticImport);
  retval.SortUsingDeclarations =
      sort_using_declarations_options.at(old.SortUsingDeclarations);
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterLogicalNot = old.SpaceAfterLogicalNot;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceAroundPointerQualifiers =
      space_around_pointer_qualifiers_style.at(
          old.SpaceAroundPointerQualifiers);
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeCaseColon = old.SpaceBeforeCaseColon;
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  retval.SpaceBeforeJsonColon = old.SpaceBeforeJsonColon;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceBeforeParensOptions.AfterControlStatements =
      old.SpaceBeforeParensOptions.AfterControlStatements;
  retval.SpaceBeforeParensOptions.AfterForeachMacros =
      old.SpaceBeforeParensOptions.AfterForeachMacros;
  retval.SpaceBeforeParensOptions.AfterFunctionDeclarationName =
      old.SpaceBeforeParensOptions.AfterFunctionDeclarationName;
  retval.SpaceBeforeParensOptions.AfterFunctionDefinitionName =
      old.SpaceBeforeParensOptions.AfterFunctionDefinitionName;
  retval.SpaceBeforeParensOptions.AfterIfMacros =
      old.SpaceBeforeParensOptions.AfterIfMacros;
  retval.SpaceBeforeParensOptions.AfterOverloadedOperator =
      old.SpaceBeforeParensOptions.AfterOverloadedOperator;
  newField("SpaceBeforeParensOptions.AfterPlacementOperator", "18",
           retval.SpaceBeforeParensOptions.AfterPlacementOperator);
  retval.SpaceBeforeParensOptions.AfterRequiresInClause =
      old.SpaceBeforeParensOptions.AfterRequiresInClause;
  retval.SpaceBeforeParensOptions.AfterRequiresInExpression =
      old.SpaceBeforeParensOptions.AfterRequiresInExpression;
  retval.SpaceBeforeParensOptions.BeforeNonEmptyParentheses =
      old.SpaceBeforeParensOptions.BeforeNonEmptyParentheses;
  retval.SpaceBeforeSquareBrackets = old.SpaceBeforeSquareBrackets;
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
  retval.SpaceInEmptyBlock = old.SpaceInEmptyBlock;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = spaces_in_angles_style.at(old.SpacesInAngles);
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInLineCommentPrefix.Minimum =
      old.SpacesInLineCommentPrefix.Minimum;
  retval.SpacesInLineCommentPrefix.Maximum =
      old.SpacesInLineCommentPrefix.Maximum;
  retval.SpacesInParens = spaces_in_parens_style.at(old.SpacesInParens);
  retval.SpacesInParensOptions.InConditionalStatements =
      old.SpacesInParensOptions.InConditionalStatements;
  retval.SpacesInParensOptions.InCStyleCasts =
      old.SpacesInParensOptions.InCStyleCasts;
  retval.SpacesInParensOptions.InEmptyParentheses =
      old.SpacesInParensOptions.InEmptyParentheses;
  retval.SpacesInParensOptions.Other = old.SpacesInParensOptions.Other;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.Standard = language_standard.at(old.Standard);
  retval.StatementAttributeLikeMacros = old.StatementAttributeLikeMacros;
  retval.StatementMacros = old.StatementMacros;
  retval.TabWidth = old.TabWidth;
  retval.TypeNames = old.TypeNames;
  retval.TypenameMacros = old.TypenameMacros;
  retval.UseTab = use_tab_style.at(old.UseTab);
  retval.VerilogBreakBetweenInstancePorts =
      old.VerilogBreakBetweenInstancePorts;
  retval.WhitespaceSensitiveMacros = old.WhitespaceSensitiveMacros;

  return retval;
}

} // namespace clang_update_v18

namespace clang_update_v19 {

constexpr frozen::unordered_map<clang_v18::FormatStyle::BracketAlignmentStyle,
                                clang_v19::FormatStyle::BracketAlignmentStyle,
                                4>
    bracket_all_alignment_style{
        {clang_v18::FormatStyle::BracketAlignmentStyle::BAS_Align,
         clang_v19::FormatStyle::BracketAlignmentStyle::BAS_Align},
        {clang_v18::FormatStyle::BracketAlignmentStyle::BAS_DontAlign,
         clang_v19::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {clang_v18::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak,
         clang_v19::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak},
        {clang_v18::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent,
         clang_v19::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::ArrayInitializerAlignmentStyle,
    clang_v19::FormatStyle::ArrayInitializerAlignmentStyle, 3>
    array_initializer_alignment_style{
        {clang_v18::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left,
         clang_v19::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left},
        {clang_v18::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right,
         clang_v19::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Right},
        {clang_v18::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None,
         clang_v19::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_None}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::EscapedNewlineAlignmentStyle,
    clang_v19::FormatStyle::EscapedNewlineAlignmentStyle, 3>
    escaped_new_line_alignment_style{
        {clang_v18::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign,
         clang_v19::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_DontAlign},
        {clang_v18::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left,
         clang_v19::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left},
        {clang_v18::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right,
         clang_v19::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::OperandAlignmentStyle,
                                clang_v19::FormatStyle::OperandAlignmentStyle,
                                3>
    operand_alignment_style{
        {clang_v18::FormatStyle::OperandAlignmentStyle::OAS_DontAlign,
         clang_v19::FormatStyle::OperandAlignmentStyle::OAS_DontAlign},
        {clang_v18::FormatStyle::OperandAlignmentStyle::OAS_Align,
         clang_v19::FormatStyle::OperandAlignmentStyle::OAS_Align},
        {clang_v18::FormatStyle::OperandAlignmentStyle::OAS_AlignAfterOperator,
         clang_v19::FormatStyle::OperandAlignmentStyle::
             OAS_AlignAfterOperator}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::TrailingCommentsAlignmentKinds,
    clang_v19::FormatStyle::TrailingCommentsAlignmentKinds, 3>
    trailing_comments_alignment_kinds{
        {clang_v18::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Leave,
         clang_v19::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Leave},
        {clang_v18::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Always,
         clang_v19::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Always},
        {clang_v18::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Never,
         clang_v19::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Never}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::BreakBeforeNoexceptSpecifierStyle,
    clang_v19::FormatStyle::BreakBeforeNoexceptSpecifierStyle, 3>
    break_before_noexcept_specifier_style{
        {clang_v18::FormatStyle::BreakBeforeNoexceptSpecifierStyle::BBNSS_Never,
         clang_v19::FormatStyle::BreakBeforeNoexceptSpecifierStyle::
             BBNSS_Never},
        {clang_v18::FormatStyle::BreakBeforeNoexceptSpecifierStyle::
             BBNSS_OnlyWithParen,
         clang_v19::FormatStyle::BreakBeforeNoexceptSpecifierStyle::
             BBNSS_OnlyWithParen},
        {clang_v18::FormatStyle::BreakBeforeNoexceptSpecifierStyle::
             BBNSS_Always,
         clang_v19::FormatStyle::BreakBeforeNoexceptSpecifierStyle::
             BBNSS_Always}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::ShortBlockStyle,
                                clang_v19::FormatStyle::ShortBlockStyle, 3>
    short_block_style{{clang_v18::FormatStyle::ShortBlockStyle::SBS_Never,
                       clang_v19::FormatStyle::ShortBlockStyle::SBS_Never},
                      {clang_v18::FormatStyle::ShortBlockStyle::SBS_Empty,
                       clang_v19::FormatStyle::ShortBlockStyle::SBS_Empty},
                      {clang_v18::FormatStyle::ShortBlockStyle::SBS_Always,
                       clang_v19::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::ShortFunctionStyle,
                                clang_v19::FormatStyle::ShortFunctionStyle, 5>
    short_function_style{
        {clang_v18::FormatStyle::ShortFunctionStyle::SFS_None,
         clang_v19::FormatStyle::ShortFunctionStyle::SFS_None},
        {clang_v18::FormatStyle::ShortFunctionStyle::SFS_InlineOnly,
         clang_v19::FormatStyle::ShortFunctionStyle::SFS_InlineOnly},
        {clang_v18::FormatStyle::ShortFunctionStyle::SFS_Empty,
         clang_v19::FormatStyle::ShortFunctionStyle::SFS_Empty},
        {clang_v18::FormatStyle::ShortFunctionStyle::SFS_Inline,
         clang_v19::FormatStyle::ShortFunctionStyle::SFS_Inline},
        {clang_v18::FormatStyle::ShortFunctionStyle::SFS_All,
         clang_v19::FormatStyle::ShortFunctionStyle::SFS_All}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::ShortIfStyle,
                                clang_v19::FormatStyle::ShortIfStyle, 4>
    short_if_style{{clang_v18::FormatStyle::ShortIfStyle::SIS_Never,
                    clang_v19::FormatStyle::ShortIfStyle::SIS_Never},
                   {clang_v18::FormatStyle::ShortIfStyle::SIS_WithoutElse,
                    clang_v19::FormatStyle::ShortIfStyle::SIS_WithoutElse},
                   {clang_v18::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf,
                    clang_v19::FormatStyle::ShortIfStyle::SIS_OnlyFirstIf},
                   {clang_v18::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse,
                    clang_v19::FormatStyle::ShortIfStyle::SIS_AllIfsAndElse}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::ShortLambdaStyle,
                                clang_v19::FormatStyle::ShortLambdaStyle, 4>
    short_lambda_style{{clang_v18::FormatStyle::ShortLambdaStyle::SLS_None,
                        clang_v19::FormatStyle::ShortLambdaStyle::SLS_None},
                       {clang_v18::FormatStyle::ShortLambdaStyle::SLS_Empty,
                        clang_v19::FormatStyle::ShortLambdaStyle::SLS_Empty},
                       {clang_v18::FormatStyle::ShortLambdaStyle::SLS_Inline,
                        clang_v19::FormatStyle::ShortLambdaStyle::SLS_Inline},
                       {clang_v18::FormatStyle::ShortLambdaStyle::SLS_All,
                        clang_v19::FormatStyle::ShortLambdaStyle::SLS_All}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::DefinitionReturnTypeBreakingStyle,
    clang_v19::FormatStyle::DefinitionReturnTypeBreakingStyle, 3>
    definition_return_type_breaking_style{
        {clang_v18::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None,
         clang_v19::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None},
        {clang_v18::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All,
         clang_v19::FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_All},
        {clang_v18::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel,
         clang_v19::FormatStyle::DefinitionReturnTypeBreakingStyle::
             DRTBS_TopLevel}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::BitFieldColonSpacingStyle,
    clang_v19::FormatStyle::BitFieldColonSpacingStyle, 4>
    bite_field_colon_spacing_style{
        {clang_v18::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both,
         clang_v19::FormatStyle::BitFieldColonSpacingStyle::BFCS_Both},
        {clang_v18::FormatStyle::BitFieldColonSpacingStyle::BFCS_None,
         clang_v19::FormatStyle::BitFieldColonSpacingStyle::BFCS_None},
        {clang_v18::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before,
         clang_v19::FormatStyle::BitFieldColonSpacingStyle::BFCS_Before},
        {clang_v18::FormatStyle::BitFieldColonSpacingStyle::BFCS_After,
         clang_v19::FormatStyle::BitFieldColonSpacingStyle::BFCS_After}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::BraceWrappingAfterControlStatementStyle,
    clang_v19::FormatStyle::BraceWrappingAfterControlStatementStyle, 3>
    brace_wrapping_after_control_statement_style{
        {clang_v18::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never,
         clang_v19::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Never},
        {clang_v18::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine,
         clang_v19::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_MultiLine},
        {clang_v18::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always,
         clang_v19::FormatStyle::BraceWrappingAfterControlStatementStyle::
             BWACS_Always}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::AttributeBreakingStyle,
                                clang_v19::FormatStyle::AttributeBreakingStyle,
                                3>
    attribute_breaking_style{
        {clang_v18::FormatStyle::AttributeBreakingStyle::ABS_Always,
         clang_v19::FormatStyle::AttributeBreakingStyle::ABS_Always},
        {clang_v18::FormatStyle::AttributeBreakingStyle::ABS_Leave,
         clang_v19::FormatStyle::AttributeBreakingStyle::ABS_Leave},
        {clang_v18::FormatStyle::AttributeBreakingStyle::ABS_Never,
         clang_v19::FormatStyle::AttributeBreakingStyle::ABS_Never}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::ReturnTypeBreakingStyle,
                                clang_v19::FormatStyle::ReturnTypeBreakingStyle,
                                5>
    return_type_breaking_style{
        {clang_v18::FormatStyle::ReturnTypeBreakingStyle::RTBS_None,
         clang_v19::FormatStyle::ReturnTypeBreakingStyle::RTBS_None},
        {clang_v18::FormatStyle::ReturnTypeBreakingStyle::RTBS_All,
         clang_v19::FormatStyle::ReturnTypeBreakingStyle::RTBS_All},
        {clang_v18::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel,
         clang_v19::FormatStyle::ReturnTypeBreakingStyle::RTBS_TopLevel},
        {clang_v18::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions,
         clang_v19::FormatStyle::ReturnTypeBreakingStyle::RTBS_AllDefinitions},
        {clang_v18::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions,
         clang_v19::FormatStyle::ReturnTypeBreakingStyle::
             RTBS_TopLevelDefinitions}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::BinaryOperatorStyle,
                                clang_v19::FormatStyle::BinaryOperatorStyle, 3>
    binary_operator_style{
        {clang_v18::FormatStyle::BinaryOperatorStyle::BOS_None,
         clang_v19::FormatStyle::BinaryOperatorStyle::BOS_None},
        {clang_v18::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment,
         clang_v19::FormatStyle::BinaryOperatorStyle::BOS_NonAssignment},
        {clang_v18::FormatStyle::BinaryOperatorStyle::BOS_All,
         clang_v19::FormatStyle::BinaryOperatorStyle::BOS_All}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::BraceBreakingStyle,
                                clang_v19::FormatStyle::BraceBreakingStyle, 9>
    brace_breaking_style{
        {clang_v18::FormatStyle::BraceBreakingStyle::BS_Attach,
         clang_v19::FormatStyle::BraceBreakingStyle::BS_Attach},
        {clang_v18::FormatStyle::BraceBreakingStyle::BS_Linux,
         clang_v19::FormatStyle::BraceBreakingStyle::BS_Linux},
        {clang_v18::FormatStyle::BraceBreakingStyle::BS_Mozilla,
         clang_v19::FormatStyle::BraceBreakingStyle::BS_Mozilla},
        {clang_v18::FormatStyle::BraceBreakingStyle::BS_Stroustrup,
         clang_v19::FormatStyle::BraceBreakingStyle::BS_Stroustrup},
        {clang_v18::FormatStyle::BraceBreakingStyle::BS_Allman,
         clang_v19::FormatStyle::BraceBreakingStyle::BS_Allman},
        {clang_v18::FormatStyle::BraceBreakingStyle::BS_Whitesmiths,
         clang_v19::FormatStyle::BraceBreakingStyle::BS_Whitesmiths},
        {clang_v18::FormatStyle::BraceBreakingStyle::BS_GNU,
         clang_v19::FormatStyle::BraceBreakingStyle::BS_GNU},
        {clang_v18::FormatStyle::BraceBreakingStyle::BS_WebKit,
         clang_v19::FormatStyle::BraceBreakingStyle::BS_WebKit},
        {clang_v18::FormatStyle::BraceBreakingStyle::BS_Custom,
         clang_v19::FormatStyle::BraceBreakingStyle::BS_Custom}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::BreakBeforeConceptDeclarationsStyle,
    clang_v19::FormatStyle::BreakBeforeConceptDeclarationsStyle, 3>
    break_before_concept_declarations_style{
        {clang_v18::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Never,
         clang_v19::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Never},
        {clang_v18::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Allowed,
         clang_v19::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Allowed},
        {clang_v18::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Always,
         clang_v19::FormatStyle::BreakBeforeConceptDeclarationsStyle::
             BBCDS_Always}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::BreakBeforeInlineASMColonStyle,
    clang_v19::FormatStyle::BreakBeforeInlineASMColonStyle, 3>
    break_before_inline_asm_colon_style{
        {clang_v18::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Never,
         clang_v19::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Never},
        {clang_v18::FormatStyle::BreakBeforeInlineASMColonStyle::
             BBIAS_OnlyMultiline,
         clang_v19::FormatStyle::BreakBeforeInlineASMColonStyle::
             BBIAS_OnlyMultiline},
        {clang_v18::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Always,
         clang_v19::FormatStyle::BreakBeforeInlineASMColonStyle::BBIAS_Always}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::BreakConstructorInitializersStyle,
    clang_v19::FormatStyle::BreakConstructorInitializersStyle, 3>
    break_constructor_initializers_style{
        {clang_v18::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon,
         clang_v19::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeColon},
        {clang_v18::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma,
         clang_v19::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_BeforeComma},
        {clang_v18::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon,
         clang_v19::FormatStyle::BreakConstructorInitializersStyle::
             BCIS_AfterColon}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::BreakInheritanceListStyle,
    clang_v19::FormatStyle::BreakInheritanceListStyle, 4>
    break_inheritance_list_style{
        {clang_v18::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon,
         clang_v19::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {clang_v18::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma,
         clang_v19::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma},
        {clang_v18::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon,
         clang_v19::FormatStyle::BreakInheritanceListStyle::BILS_AfterColon},
        {clang_v18::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma,
         clang_v19::FormatStyle::BreakInheritanceListStyle::BILS_AfterComma}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::BreakTemplateDeclarationsStyle,
    clang_v19::FormatStyle::BreakTemplateDeclarationsStyle, 3>
    break_template_declarations_style{
        {clang_v18::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No,
         clang_v19::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_No},
        {clang_v18::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine,
         clang_v19::FormatStyle::BreakTemplateDeclarationsStyle::
             BTDS_MultiLine},
        {clang_v18::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes,
         clang_v19::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::EmptyLineAfterAccessModifierStyle,
    clang_v19::FormatStyle::EmptyLineAfterAccessModifierStyle, 3>
    empty_line_after_access_modifier_style{
        {clang_v18::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never,
         clang_v19::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Never},
        {clang_v18::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave,
         clang_v19::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Leave},
        {clang_v18::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always,
         clang_v19::FormatStyle::EmptyLineAfterAccessModifierStyle::
             ELAAMS_Always}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::EmptyLineBeforeAccessModifierStyle,
    clang_v19::FormatStyle::EmptyLineBeforeAccessModifierStyle, 4>
    empty_line_before_access_modifier_style{
        {clang_v18::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never,
         clang_v19::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Never},
        {clang_v18::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave,
         clang_v19::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Leave},
        {clang_v18::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock,
         clang_v19::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_LogicalBlock},
        {clang_v18::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always,
         clang_v19::FormatStyle::EmptyLineBeforeAccessModifierStyle::
             ELBAMS_Always}};

constexpr frozen::unordered_map<clang_v18::IncludeStyle::IncludeBlocksStyle,
                                clang_v19::IncludeStyle::IncludeBlocksStyle, 3>
    include_blocks_style{
        {clang_v18::IncludeStyle::IncludeBlocksStyle::IBS_Preserve,
         clang_v19::IncludeStyle::IncludeBlocksStyle::IBS_Preserve},
        {clang_v18::IncludeStyle::IncludeBlocksStyle::IBS_Merge,
         clang_v19::IncludeStyle::IncludeBlocksStyle::IBS_Merge},
        {clang_v18::IncludeStyle::IncludeBlocksStyle::IBS_Regroup,
         clang_v19::IncludeStyle::IncludeBlocksStyle::IBS_Regroup}};

void assign(std::vector<clang_v18::IncludeStyle::IncludeCategory> &lhs,
            std::vector<clang_v19::IncludeStyle::IncludeCategory> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v19::IncludeStyle::IncludeCategory{
        item.Regex, item.Priority, item.SortPriority, true});
  }
}

constexpr frozen::unordered_map<clang_v18::FormatStyle::IndentExternBlockStyle,
                                clang_v19::FormatStyle::IndentExternBlockStyle,
                                3>
    indent_extern_block_style{
        {clang_v18::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock,
         clang_v19::FormatStyle::IndentExternBlockStyle::IEBS_AfterExternBlock},
        {clang_v18::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent,
         clang_v19::FormatStyle::IndentExternBlockStyle::IEBS_NoIndent},
        {clang_v18::FormatStyle::IndentExternBlockStyle::IEBS_Indent,
         clang_v19::FormatStyle::IndentExternBlockStyle::IEBS_Indent}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::PPDirectiveIndentStyle,
                                clang_v19::FormatStyle::PPDirectiveIndentStyle,
                                3>
    pp_directive_indent_style{
        {clang_v18::FormatStyle::PPDirectiveIndentStyle::PPDIS_None,
         clang_v19::FormatStyle::PPDirectiveIndentStyle::PPDIS_None},
        {clang_v18::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash,
         clang_v19::FormatStyle::PPDirectiveIndentStyle::PPDIS_AfterHash},
        {clang_v18::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash,
         clang_v19::FormatStyle::PPDirectiveIndentStyle::PPDIS_BeforeHash}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::TrailingCommaStyle,
                                clang_v19::FormatStyle::TrailingCommaStyle, 2>
    trailing_comma_style{
        {clang_v18::FormatStyle::TrailingCommaStyle::TCS_None,
         clang_v19::FormatStyle::TrailingCommaStyle::TCS_None},
        {clang_v18::FormatStyle::TrailingCommaStyle::TCS_Wrapped,
         clang_v19::FormatStyle::TrailingCommaStyle::TCS_Wrapped}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::JavaScriptQuoteStyle,
                                clang_v19::FormatStyle::JavaScriptQuoteStyle, 3>
    java_script_quote_style{
        {clang_v18::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave,
         clang_v19::FormatStyle::JavaScriptQuoteStyle::JSQS_Leave},
        {clang_v18::FormatStyle::JavaScriptQuoteStyle::JSQS_Single,
         clang_v19::FormatStyle::JavaScriptQuoteStyle::JSQS_Single},
        {clang_v18::FormatStyle::JavaScriptQuoteStyle::JSQS_Double,
         clang_v19::FormatStyle::JavaScriptQuoteStyle::JSQS_Double}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::LambdaBodyIndentationKind,
    clang_v19::FormatStyle::LambdaBodyIndentationKind, 2>
    lambda_body_indentation_king{
        {clang_v18::FormatStyle::LambdaBodyIndentationKind::LBI_Signature,
         clang_v19::FormatStyle::LambdaBodyIndentationKind::LBI_Signature},
        {clang_v18::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope,
         clang_v19::FormatStyle::LambdaBodyIndentationKind::LBI_OuterScope}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::LanguageKind,
                                clang_v19::FormatStyle::LanguageKind, 11>
    language_king{{clang_v18::FormatStyle::LanguageKind::LK_None,
                   clang_v19::FormatStyle::LanguageKind::LK_None},
                  {clang_v18::FormatStyle::LanguageKind::LK_Cpp,
                   clang_v19::FormatStyle::LanguageKind::LK_Cpp},
                  {clang_v18::FormatStyle::LanguageKind::LK_CSharp,
                   clang_v19::FormatStyle::LanguageKind::LK_CSharp},
                  {clang_v18::FormatStyle::LanguageKind::LK_Java,
                   clang_v19::FormatStyle::LanguageKind::LK_Java},
                  {clang_v18::FormatStyle::LanguageKind::LK_JavaScript,
                   clang_v19::FormatStyle::LanguageKind::LK_JavaScript},
                  {clang_v18::FormatStyle::LanguageKind::LK_Json,
                   clang_v19::FormatStyle::LanguageKind::LK_Json},
                  {clang_v18::FormatStyle::LanguageKind::LK_ObjC,
                   clang_v19::FormatStyle::LanguageKind::LK_ObjC},
                  {clang_v18::FormatStyle::LanguageKind::LK_Proto,
                   clang_v19::FormatStyle::LanguageKind::LK_Proto},
                  {clang_v18::FormatStyle::LanguageKind::LK_TableGen,
                   clang_v19::FormatStyle::LanguageKind::LK_TableGen},
                  {clang_v18::FormatStyle::LanguageKind::LK_TextProto,
                   clang_v19::FormatStyle::LanguageKind::LK_TextProto},
                  {clang_v18::FormatStyle::LanguageKind::LK_Verilog,
                   clang_v19::FormatStyle::LanguageKind::LK_Verilog}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::LineEndingStyle,
                                clang_v19::FormatStyle::LineEndingStyle, 4>
    line_ending_style{{clang_v18::FormatStyle::LineEndingStyle::LE_LF,
                       clang_v19::FormatStyle::LineEndingStyle::LE_LF},
                      {clang_v18::FormatStyle::LineEndingStyle::LE_CRLF,
                       clang_v19::FormatStyle::LineEndingStyle::LE_CRLF},
                      {clang_v18::FormatStyle::LineEndingStyle::LE_DeriveLF,
                       clang_v19::FormatStyle::LineEndingStyle::LE_DeriveLF},
                      {clang_v18::FormatStyle::LineEndingStyle::LE_DeriveCRLF,
                       clang_v19::FormatStyle::LineEndingStyle::LE_DeriveCRLF}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::NamespaceIndentationKind,
    clang_v19::FormatStyle::NamespaceIndentationKind, 3>
    namespace_indentation_kind{
        {clang_v18::FormatStyle::NamespaceIndentationKind::NI_None,
         clang_v19::FormatStyle::NamespaceIndentationKind::NI_None},
        {clang_v18::FormatStyle::NamespaceIndentationKind::NI_Inner,
         clang_v19::FormatStyle::NamespaceIndentationKind::NI_Inner},
        {clang_v18::FormatStyle::NamespaceIndentationKind::NI_All,
         clang_v19::FormatStyle::NamespaceIndentationKind::NI_All}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::BinPackStyle,
                                clang_v19::FormatStyle::BinPackStyle, 3>
    bin_pack_style{{clang_v18::FormatStyle::BinPackStyle::BPS_Auto,
                    clang_v19::FormatStyle::BinPackStyle::BPS_Auto},
                   {clang_v18::FormatStyle::BinPackStyle::BPS_Always,
                    clang_v19::FormatStyle::BinPackStyle::BPS_Always},
                   {clang_v18::FormatStyle::BinPackStyle::BPS_Never,
                    clang_v19::FormatStyle::BinPackStyle::BPS_Never}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::PackConstructorInitializersStyle,
    clang_v19::FormatStyle::PackConstructorInitializersStyle, 5>
    pack_constructor_initializers_style{
        {clang_v18::FormatStyle::PackConstructorInitializersStyle::PCIS_Never,
         clang_v19::FormatStyle::PackConstructorInitializersStyle::PCIS_Never},
        {clang_v18::FormatStyle::PackConstructorInitializersStyle::PCIS_BinPack,
         clang_v19::FormatStyle::PackConstructorInitializersStyle::
             PCIS_BinPack},
        {clang_v18::FormatStyle::PackConstructorInitializersStyle::
             PCIS_CurrentLine,
         clang_v19::FormatStyle::PackConstructorInitializersStyle::
             PCIS_CurrentLine},
        {clang_v18::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLine,
         clang_v19::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLine},
        {clang_v18::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLineOnly,
         clang_v19::FormatStyle::PackConstructorInitializersStyle::
             PCIS_NextLineOnly}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::PointerAlignmentStyle,
                                clang_v19::FormatStyle::PointerAlignmentStyle,
                                3>
    pointer_alignment_style{
        {clang_v18::FormatStyle::PointerAlignmentStyle::PAS_Left,
         clang_v19::FormatStyle::PointerAlignmentStyle::PAS_Left},
        {clang_v18::FormatStyle::PointerAlignmentStyle::PAS_Right,
         clang_v19::FormatStyle::PointerAlignmentStyle::PAS_Right},
        {clang_v18::FormatStyle::PointerAlignmentStyle::PAS_Middle,
         clang_v19::FormatStyle::PointerAlignmentStyle::PAS_Middle}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::QualifierAlignmentStyle,
                                clang_v19::FormatStyle::QualifierAlignmentStyle,
                                4>
    qualifier_alignment_style{
        {clang_v18::FormatStyle::QualifierAlignmentStyle::QAS_Leave,
         clang_v19::FormatStyle::QualifierAlignmentStyle::QAS_Leave},
        {clang_v18::FormatStyle::QualifierAlignmentStyle::QAS_Left,
         clang_v19::FormatStyle::QualifierAlignmentStyle::QAS_Left},
        {clang_v18::FormatStyle::QualifierAlignmentStyle::QAS_Right,
         clang_v19::FormatStyle::QualifierAlignmentStyle::QAS_Right},
        {clang_v18::FormatStyle::QualifierAlignmentStyle::QAS_Custom,
         clang_v19::FormatStyle::QualifierAlignmentStyle::QAS_Custom}};

void assign(std::vector<clang_v18::FormatStyle::RawStringFormat> &lhs,
            std::vector<clang_v19::FormatStyle::RawStringFormat> &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());
  for (const auto &item : lhs) {
    rhs.emplace_back(clang_v19::FormatStyle::RawStringFormat{
        language_king.at(item.Language), item.Delimiters,
        item.EnclosingFunctions, item.CanonicalDelimiter, item.BasedOnStyle});
  }
}

constexpr frozen::unordered_map<clang_v18::FormatStyle::ReferenceAlignmentStyle,
                                clang_v19::FormatStyle::ReferenceAlignmentStyle,
                                4>
    reference_alignment_style{
        {clang_v18::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer,
         clang_v19::FormatStyle::ReferenceAlignmentStyle::RAS_Pointer},
        {clang_v18::FormatStyle::ReferenceAlignmentStyle::RAS_Left,
         clang_v19::FormatStyle::ReferenceAlignmentStyle::RAS_Left},
        {clang_v18::FormatStyle::ReferenceAlignmentStyle::RAS_Right,
         clang_v19::FormatStyle::ReferenceAlignmentStyle::RAS_Right},
        {clang_v18::FormatStyle::ReferenceAlignmentStyle::RAS_Middle,
         clang_v19::FormatStyle::ReferenceAlignmentStyle::RAS_Middle}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::RemoveParenthesesStyle,
                                clang_v19::FormatStyle::RemoveParenthesesStyle,
                                3>
    remove_parentheses_style{
        {clang_v18::FormatStyle::RemoveParenthesesStyle::RPS_Leave,
         clang_v19::FormatStyle::RemoveParenthesesStyle::RPS_Leave},
        {clang_v18::FormatStyle::RemoveParenthesesStyle::
             RPS_MultipleParentheses,
         clang_v19::FormatStyle::RemoveParenthesesStyle::
             RPS_MultipleParentheses},
        {clang_v18::FormatStyle::RemoveParenthesesStyle::RPS_ReturnStatement,
         clang_v19::FormatStyle::RemoveParenthesesStyle::RPS_ReturnStatement}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::RequiresClausePositionStyle,
    clang_v19::FormatStyle::RequiresClausePositionStyle, 4>
    requires_clause_position_style{
        {clang_v18::FormatStyle::RequiresClausePositionStyle::RCPS_OwnLine,
         clang_v19::FormatStyle::RequiresClausePositionStyle::RCPS_OwnLine},
        {clang_v18::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithPreceding,
         clang_v19::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithPreceding},
        {clang_v18::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithFollowing,
         clang_v19::FormatStyle::RequiresClausePositionStyle::
             RCPS_WithFollowing},
        {clang_v18::FormatStyle::RequiresClausePositionStyle::RCPS_SingleLine,
         clang_v19::FormatStyle::RequiresClausePositionStyle::RCPS_SingleLine}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::RequiresExpressionIndentationKind,
    clang_v19::FormatStyle::RequiresExpressionIndentationKind, 2>
    requires_expression_indentation_kind{
        {clang_v18::FormatStyle::RequiresExpressionIndentationKind::
             REI_OuterScope,
         clang_v19::FormatStyle::RequiresExpressionIndentationKind::
             REI_OuterScope},
        {clang_v18::FormatStyle::RequiresExpressionIndentationKind::REI_Keyword,
         clang_v19::FormatStyle::RequiresExpressionIndentationKind::
             REI_Keyword}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::SeparateDefinitionStyle,
                                clang_v19::FormatStyle::SeparateDefinitionStyle,
                                3>
    separate_definitions_style{
        {clang_v18::FormatStyle::SeparateDefinitionStyle::SDS_Leave,
         clang_v19::FormatStyle::SeparateDefinitionStyle::SDS_Leave},
        {clang_v18::FormatStyle::SeparateDefinitionStyle::SDS_Always,
         clang_v19::FormatStyle::SeparateDefinitionStyle::SDS_Always},
        {clang_v18::FormatStyle::SeparateDefinitionStyle::SDS_Never,
         clang_v19::FormatStyle::SeparateDefinitionStyle::SDS_Never}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::SortIncludesOptions,
                                clang_v19::FormatStyle::SortIncludesOptions, 3>
    sort_includes_options{
        {clang_v18::FormatStyle::SortIncludesOptions::SI_Never,
         clang_v19::FormatStyle::SortIncludesOptions::SI_Never},
        {clang_v18::FormatStyle::SortIncludesOptions::SI_CaseSensitive,
         clang_v19::FormatStyle::SortIncludesOptions::SI_CaseSensitive},
        {clang_v18::FormatStyle::SortIncludesOptions::SI_CaseInsensitive,
         clang_v19::FormatStyle::SortIncludesOptions::SI_CaseInsensitive}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::SortJavaStaticImportOptions,
    clang_v19::FormatStyle::SortJavaStaticImportOptions, 2>
    sort_java_static_import_options{
        {clang_v18::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before,
         clang_v19::FormatStyle::SortJavaStaticImportOptions::SJSIO_Before},
        {clang_v18::FormatStyle::SortJavaStaticImportOptions::SJSIO_After,
         clang_v19::FormatStyle::SortJavaStaticImportOptions::SJSIO_After}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::SortUsingDeclarationsOptions,
    clang_v19::FormatStyle::SortUsingDeclarationsOptions, 3>
    sort_using_declarations_options{
        {clang_v18::FormatStyle::SortUsingDeclarationsOptions::SUD_Never,
         clang_v19::FormatStyle::SortUsingDeclarationsOptions::SUD_Never},
        {clang_v18::FormatStyle::SortUsingDeclarationsOptions::
             SUD_Lexicographic,
         clang_v19::FormatStyle::SortUsingDeclarationsOptions::
             SUD_Lexicographic},
        {clang_v18::FormatStyle::SortUsingDeclarationsOptions::
             SUD_LexicographicNumeric,
         clang_v19::FormatStyle::SortUsingDeclarationsOptions::
             SUD_LexicographicNumeric}};

constexpr frozen::unordered_map<
    clang_v18::FormatStyle::SpaceAroundPointerQualifiersStyle,
    clang_v19::FormatStyle::SpaceAroundPointerQualifiersStyle, 4>
    space_around_pointer_qualifiers_style{
        {clang_v18::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default,
         clang_v19::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Default},
        {clang_v18::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Before,
         clang_v19::FormatStyle::SpaceAroundPointerQualifiersStyle::
             SAPQ_Before},
        {clang_v18::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After,
         clang_v19::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_After},
        {clang_v18::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both,
         clang_v19::FormatStyle::SpaceAroundPointerQualifiersStyle::SAPQ_Both}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::SpaceBeforeParensStyle,
                                clang_v19::FormatStyle::SpaceBeforeParensStyle,
                                6>
    space_before_parens_options{
        {clang_v18::FormatStyle::SpaceBeforeParensStyle::SBPO_Never,
         clang_v19::FormatStyle::SpaceBeforeParensStyle::SBPO_Never},
        {clang_v18::FormatStyle::SpaceBeforeParensStyle::SBPO_ControlStatements,
         clang_v19::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatements},
        {clang_v18::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros,
         clang_v19::FormatStyle::SpaceBeforeParensStyle::
             SBPO_ControlStatementsExceptControlMacros},
        {clang_v18::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses,
         clang_v19::FormatStyle::SpaceBeforeParensStyle::
             SBPO_NonEmptyParentheses},
        {clang_v18::FormatStyle::SpaceBeforeParensStyle::SBPO_Always,
         clang_v19::FormatStyle::SpaceBeforeParensStyle::SBPO_Always},
        {clang_v18::FormatStyle::SpaceBeforeParensStyle::SBPO_Custom,
         clang_v19::FormatStyle::SpaceBeforeParensStyle::SBPO_Custom}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::SpacesInAnglesStyle,
                                clang_v19::FormatStyle::SpacesInAnglesStyle, 3>
    spaces_in_angles_style{
        {clang_v18::FormatStyle::SpacesInAnglesStyle::SIAS_Never,
         clang_v19::FormatStyle::SpacesInAnglesStyle::SIAS_Never},
        {clang_v18::FormatStyle::SpacesInAnglesStyle::SIAS_Always,
         clang_v19::FormatStyle::SpacesInAnglesStyle::SIAS_Always},
        {clang_v18::FormatStyle::SpacesInAnglesStyle::SIAS_Leave,
         clang_v19::FormatStyle::SpacesInAnglesStyle::SIAS_Leave}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::SpacesInParensStyle,
                                clang_v19::FormatStyle::SpacesInParensStyle, 2>
    spaces_in_parens_style{
        {clang_v18::FormatStyle::SpacesInParensStyle::SIPO_Never,
         clang_v19::FormatStyle::SpacesInParensStyle::SIPO_Never},
        {clang_v18::FormatStyle::SpacesInParensStyle::SIPO_Custom,
         clang_v19::FormatStyle::SpacesInParensStyle::SIPO_Custom}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::LanguageStandard,
                                clang_v19::FormatStyle::LanguageStandard, 7>
    language_standard{{clang_v18::FormatStyle::LanguageStandard::LS_Cpp03,
                       clang_v19::FormatStyle::LanguageStandard::LS_Cpp03},
                      {clang_v18::FormatStyle::LanguageStandard::LS_Cpp11,
                       clang_v19::FormatStyle::LanguageStandard::LS_Cpp11},
                      {clang_v18::FormatStyle::LanguageStandard::LS_Cpp14,
                       clang_v19::FormatStyle::LanguageStandard::LS_Cpp14},
                      {clang_v18::FormatStyle::LanguageStandard::LS_Cpp17,
                       clang_v19::FormatStyle::LanguageStandard::LS_Cpp17},
                      {clang_v18::FormatStyle::LanguageStandard::LS_Cpp20,
                       clang_v19::FormatStyle::LanguageStandard::LS_Cpp20},
                      {clang_v18::FormatStyle::LanguageStandard::LS_Latest,
                       clang_v19::FormatStyle::LanguageStandard::LS_Latest},
                      {clang_v18::FormatStyle::LanguageStandard::LS_Auto,
                       clang_v19::FormatStyle::LanguageStandard::LS_Auto}};

constexpr frozen::unordered_map<clang_v18::FormatStyle::UseTabStyle,
                                clang_v19::FormatStyle::UseTabStyle, 5>
    use_tab_style{
        {clang_v18::FormatStyle::UseTabStyle::UT_Never,
         clang_v19::FormatStyle::UseTabStyle::UT_Never},
        {clang_v18::FormatStyle::UseTabStyle::UT_ForIndentation,
         clang_v19::FormatStyle::UseTabStyle::UT_ForIndentation},
        {clang_v18::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation,
         clang_v19::FormatStyle::UseTabStyle::UT_ForContinuationAndIndentation},
        {clang_v18::FormatStyle::UseTabStyle::UT_AlignWithSpaces,
         clang_v19::FormatStyle::UseTabStyle::UT_AlignWithSpaces},
        {clang_v18::FormatStyle::UseTabStyle::UT_Always,
         clang_v19::FormatStyle::UseTabStyle::UT_Always}};

clang_v19::FormatStyle update(clang_v18::FormatStyle &old,
                              const std::string &style) {
  clang_v19::FormatStyle retval;
  if (!clang_v19::getPredefinedStyle(
          style, clang_v19::FormatStyle::LanguageKind::LK_Cpp, &retval)) {
    throw std::runtime_error("Failed to load " + style + " style.");
  }

  retval.InheritsParentConfig = old.InheritsParentConfig;
  retval.AccessModifierOffset = old.AccessModifierOffset;
  retval.AlignAfterOpenBracket =
      bracket_all_alignment_style.at(old.AlignAfterOpenBracket);
  retval.AlignArrayOfStructures =
      array_initializer_alignment_style.at(old.AlignArrayOfStructures);
  retval.AlignConsecutiveMacros.Enabled = old.AlignConsecutiveMacros.Enabled;
  retval.AlignConsecutiveMacros.AcrossEmptyLines =
      old.AlignConsecutiveMacros.AcrossEmptyLines;
  retval.AlignConsecutiveMacros.AcrossComments =
      old.AlignConsecutiveMacros.AcrossComments;
  retval.AlignConsecutiveMacros.AlignCompound =
      old.AlignConsecutiveMacros.AlignCompound;
  retval.AlignConsecutiveMacros.AlignFunctionPointers =
      old.AlignConsecutiveMacros.AlignFunctionPointers;
  retval.AlignConsecutiveMacros.PadOperators =
      old.AlignConsecutiveMacros.PadOperators;
  retval.AlignConsecutiveAssignments.Enabled =
      old.AlignConsecutiveAssignments.Enabled;
  retval.AlignConsecutiveAssignments.AcrossEmptyLines =
      old.AlignConsecutiveAssignments.AcrossEmptyLines;
  retval.AlignConsecutiveAssignments.AcrossComments =
      old.AlignConsecutiveAssignments.AcrossComments;
  retval.AlignConsecutiveAssignments.AlignCompound =
      old.AlignConsecutiveAssignments.AlignCompound;
  retval.AlignConsecutiveAssignments.AlignFunctionPointers =
      old.AlignConsecutiveAssignments.AlignFunctionPointers;
  retval.AlignConsecutiveAssignments.PadOperators =
      old.AlignConsecutiveAssignments.PadOperators;
  retval.AlignConsecutiveBitFields.Enabled =
      old.AlignConsecutiveBitFields.Enabled;
  retval.AlignConsecutiveBitFields.AcrossEmptyLines =
      old.AlignConsecutiveBitFields.AcrossEmptyLines;
  retval.AlignConsecutiveBitFields.AcrossComments =
      old.AlignConsecutiveBitFields.AcrossComments;
  retval.AlignConsecutiveBitFields.AlignCompound =
      old.AlignConsecutiveBitFields.AlignCompound;
  retval.AlignConsecutiveBitFields.AlignFunctionPointers =
      old.AlignConsecutiveBitFields.AlignFunctionPointers;
  retval.AlignConsecutiveBitFields.PadOperators =
      old.AlignConsecutiveBitFields.PadOperators;
  retval.AlignConsecutiveDeclarations.Enabled =
      old.AlignConsecutiveDeclarations.Enabled;
  retval.AlignConsecutiveDeclarations.AcrossEmptyLines =
      old.AlignConsecutiveDeclarations.AcrossEmptyLines;
  retval.AlignConsecutiveDeclarations.AcrossComments =
      old.AlignConsecutiveDeclarations.AcrossComments;
  retval.AlignConsecutiveDeclarations.AlignCompound =
      old.AlignConsecutiveDeclarations.AlignCompound;
  retval.AlignConsecutiveDeclarations.AlignFunctionPointers =
      old.AlignConsecutiveDeclarations.AlignFunctionPointers;
  retval.AlignConsecutiveDeclarations.PadOperators =
      old.AlignConsecutiveDeclarations.PadOperators;
  retval.AlignConsecutiveShortCaseStatements.Enabled =
      old.AlignConsecutiveShortCaseStatements.Enabled;
  retval.AlignConsecutiveShortCaseStatements.AcrossEmptyLines =
      old.AlignConsecutiveShortCaseStatements.AcrossEmptyLines;
  retval.AlignConsecutiveShortCaseStatements.AcrossComments =
      old.AlignConsecutiveShortCaseStatements.AcrossComments;
  newField("AlignConsecutiveShortCaseStatements.AlignCaseArrows", "19",
           retval.AlignConsecutiveShortCaseStatements.AlignCaseArrows);
  retval.AlignConsecutiveShortCaseStatements.AlignCaseColons =
      old.AlignConsecutiveShortCaseStatements.AlignCaseColons;
  newField("AlignConsecutiveTableGenBreakingDAGArgColons.Enabled", "19",
           retval.AlignConsecutiveTableGenBreakingDAGArgColons.Enabled);
  newField(
      "AlignConsecutiveTableGenBreakingDAGArgColons.AcrossEmptyLines", "19",
      retval.AlignConsecutiveTableGenBreakingDAGArgColons.AcrossEmptyLines);
  newField("AlignConsecutiveTableGenBreakingDAGArgColons.AcrossComments", "19",
           retval.AlignConsecutiveTableGenBreakingDAGArgColons.AcrossComments);
  newField("AlignConsecutiveTableGenBreakingDAGArgColons.AlignCompound", "19",
           retval.AlignConsecutiveTableGenBreakingDAGArgColons.AlignCompound);
  newField("AlignConsecutiveTableGenBreakingDAGArgColons.AlignFunctionPointers",
           "19",
           retval.AlignConsecutiveTableGenBreakingDAGArgColons
               .AlignFunctionPointers);
  newField("AlignConsecutiveTableGenBreakingDAGArgColons.PadOperators", "19",
           retval.AlignConsecutiveTableGenBreakingDAGArgColons.PadOperators);
  newField("AlignConsecutiveTableGenCondOperatorColons.Enabled", "19",
           retval.AlignConsecutiveTableGenCondOperatorColons.Enabled);
  newField("AlignConsecutiveTableGenCondOperatorColons.AcrossEmptyLines", "19",
           retval.AlignConsecutiveTableGenCondOperatorColons.AcrossEmptyLines);
  newField("AlignConsecutiveTableGenCondOperatorColons.AcrossComments", "19",
           retval.AlignConsecutiveTableGenCondOperatorColons.AcrossComments);
  newField("AlignConsecutiveTableGenCondOperatorColons.AlignCompound", "19",
           retval.AlignConsecutiveTableGenCondOperatorColons.AlignCompound);
  newField(
      "AlignConsecutiveTableGenCondOperatorColons.AlignFunctionPointers", "19",
      retval.AlignConsecutiveTableGenCondOperatorColons.AlignFunctionPointers);
  newField("AlignConsecutiveTableGenCondOperatorColons.PadOperators", "19",
           retval.AlignConsecutiveTableGenCondOperatorColons.PadOperators);
  newField("AlignConsecutiveTableGenDefinitionColons.Enabled", "19",
           retval.AlignConsecutiveTableGenDefinitionColons.Enabled);
  newField("AlignConsecutiveTableGenDefinitionColons.AcrossEmptyLines", "19",
           retval.AlignConsecutiveTableGenDefinitionColons.AcrossEmptyLines);
  newField("AlignConsecutiveTableGenDefinitionColons.AcrossComments", "19",
           retval.AlignConsecutiveTableGenDefinitionColons.AcrossComments);
  newField("AlignConsecutiveTableGenDefinitionColons.AlignCompound", "19",
           retval.AlignConsecutiveTableGenDefinitionColons.AlignCompound);
  newField(
      "AlignConsecutiveTableGenDefinitionColons.AlignFunctionPointers", "19",
      retval.AlignConsecutiveTableGenDefinitionColons.AlignFunctionPointers);
  newField("AlignConsecutiveTableGenDefinitionColons.PadOperators", "19",
           retval.AlignConsecutiveTableGenDefinitionColons.PadOperators);
  retval.AlignEscapedNewlines =
      escaped_new_line_alignment_style.at(old.AlignEscapedNewlines);
  improveField("AlignEscapedNewlines", "LeftWithLastLine", "19");
  retval.AlignOperands = operand_alignment_style.at(old.AlignOperands);
  retval.AlignTrailingComments.Kind =
      trailing_comments_alignment_kinds.at(old.AlignTrailingComments.Kind);
  retval.AlignTrailingComments.OverEmptyLines =
      old.AlignTrailingComments.OverEmptyLines;
  retval.AllowAllArgumentsOnNextLine = old.AllowAllArgumentsOnNextLine;
  retval.AllowAllParametersOfDeclarationOnNextLine =
      old.AllowAllParametersOfDeclarationOnNextLine;
  retval.AllowBreakBeforeNoexceptSpecifier =
      break_before_noexcept_specifier_style.at(
          old.AllowBreakBeforeNoexceptSpecifier);
  retval.AllowShortBlocksOnASingleLine =
      short_block_style.at(old.AllowShortBlocksOnASingleLine);
  newField("AllowShortCaseExpressionOnASingleLine", "19",
           retval.AllowShortCaseExpressionOnASingleLine);
  retval.AllowShortCaseLabelsOnASingleLine =
      old.AllowShortCaseLabelsOnASingleLine;
  retval.AllowShortCompoundRequirementOnASingleLine =
      old.AllowShortCompoundRequirementOnASingleLine;
  retval.AllowShortEnumsOnASingleLine = old.AllowShortEnumsOnASingleLine;
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
  retval.AlwaysBreakBeforeMultilineStrings =
      old.AlwaysBreakBeforeMultilineStrings;
  retval.AttributeMacros = old.AttributeMacros;
  retval.BinPackArguments = old.BinPackArguments;
  retval.BinPackParameters = old.BinPackParameters;
  retval.BitFieldColonSpacing =
      bite_field_colon_spacing_style.at(old.BitFieldColonSpacing);
  retval.BracedInitializerIndentWidth = old.BracedInitializerIndentWidth;
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
  retval.BraceWrapping.BeforeLambdaBody = old.BraceWrapping.BeforeLambdaBody;
  retval.BraceWrapping.BeforeWhile = old.BraceWrapping.BeforeWhile;
  retval.BraceWrapping.IndentBraces = old.BraceWrapping.IndentBraces;
  retval.BraceWrapping.SplitEmptyFunction =
      old.BraceWrapping.SplitEmptyFunction;
  retval.BraceWrapping.SplitEmptyRecord = old.BraceWrapping.SplitEmptyRecord;
  retval.BraceWrapping.SplitEmptyNamespace =
      old.BraceWrapping.SplitEmptyNamespace;
  retval.BreakAdjacentStringLiterals = old.BreakAdjacentStringLiterals;
  retval.BreakAfterAttributes =
      attribute_breaking_style.at(old.BreakAfterAttributes);
  assignWithWarning(
      "AlwaysBreakAfterReturnType", old.AlwaysBreakAfterReturnType,
      "BreakAfterReturnType", retval.BreakAfterReturnType,
      return_type_breaking_style.at(old.AlwaysBreakAfterReturnType), "19");
  improveField("BreakAfterReturnType", "Automatic", "19");
  improveField("BreakAfterReturnType", "ExceptShortType", "19");
  retval.BreakArrays = old.BreakArrays;
  retval.BreakBeforeBinaryOperators =
      binary_operator_style.at(old.BreakBeforeBinaryOperators);
  retval.BreakBeforeBraces = brace_breaking_style.at(old.BreakBeforeBraces);
  retval.BreakBeforeConceptDeclarations =
      break_before_concept_declarations_style.at(
          old.BreakBeforeConceptDeclarations);
  retval.BreakBeforeInlineASMColon =
      break_before_inline_asm_colon_style.at(old.BreakBeforeInlineASMColon);
  retval.BreakBeforeTernaryOperators = old.BreakBeforeTernaryOperators;
  retval.BreakConstructorInitializers =
      break_constructor_initializers_style.at(old.BreakConstructorInitializers);
  newField("BreakFunctionDefinitionParameters", "19",
           retval.BreakFunctionDefinitionParameters);
  retval.BreakAfterJavaFieldAnnotations = old.BreakAfterJavaFieldAnnotations;
  retval.BreakStringLiterals = old.BreakStringLiterals;
  retval.ColumnLimit = old.ColumnLimit;
  retval.CommentPragmas = old.CommentPragmas;
  retval.BreakInheritanceList =
      break_inheritance_list_style.at(old.BreakInheritanceList);
  assignWithWarning(
      "AlwaysBreakTemplateDeclarations", old.AlwaysBreakTemplateDeclarations,
      "BreakTemplateDeclarations", retval.BreakTemplateDeclarations,
      break_template_declarations_style.at(old.AlwaysBreakTemplateDeclarations),
      "19");
  improveField("BreakTemplateDeclarations", "Leave", "19");
  retval.CompactNamespaces = old.CompactNamespaces;
  retval.ConstructorInitializerIndentWidth =
      old.ConstructorInitializerIndentWidth;
  retval.ContinuationIndentWidth = old.ContinuationIndentWidth;
  retval.Cpp11BracedListStyle = old.Cpp11BracedListStyle;
  retval.DerivePointerAlignment = old.DerivePointerAlignment;
  retval.DisableFormat = old.DisableFormat;
  retval.EmptyLineAfterAccessModifier =
      empty_line_after_access_modifier_style.at(
          old.EmptyLineAfterAccessModifier);
  retval.EmptyLineBeforeAccessModifier =
      empty_line_before_access_modifier_style.at(
          old.EmptyLineBeforeAccessModifier);
  retval.ExperimentalAutoDetectBinPacking =
      old.ExperimentalAutoDetectBinPacking;
  retval.FixNamespaceComments = old.FixNamespaceComments;
  retval.ForEachMacros = old.ForEachMacros;
  retval.IncludeStyle.IncludeBlocks =
      include_blocks_style.at(old.IncludeStyle.IncludeBlocks);
  assign(old.IncludeStyle.IncludeCategories,
         retval.IncludeStyle.IncludeCategories);
  retval.IncludeStyle.IncludeIsMainRegex = old.IncludeStyle.IncludeIsMainRegex;
  retval.IncludeStyle.IncludeIsMainSourceRegex =
      old.IncludeStyle.IncludeIsMainSourceRegex;
  newField("IncludeStyle.MainIncludeChar", "19",
           retval.IncludeStyle.MainIncludeChar);
  retval.IfMacros = old.IfMacros;
  retval.IndentAccessModifiers = old.IndentAccessModifiers;
  retval.IndentCaseBlocks = old.IndentCaseBlocks;
  retval.IndentCaseLabels = old.IndentCaseLabels;
  retval.IndentGotoLabels = old.IndentGotoLabels;
  retval.IndentExternBlock =
      indent_extern_block_style.at(old.IndentExternBlock);
  retval.IndentPPDirectives =
      pp_directive_indent_style.at(old.IndentPPDirectives);
  retval.IndentRequiresClause = old.IndentRequiresClause;
  retval.IndentWidth = old.IndentWidth;
  retval.IndentWrappedFunctionNames = old.IndentWrappedFunctionNames;
  retval.InsertBraces = old.InsertBraces;
  retval.InsertNewlineAtEOF = old.InsertNewlineAtEOF;
  retval.InsertTrailingCommas =
      trailing_comma_style.at(old.InsertTrailingCommas);
  retval.IntegerLiteralSeparator.Binary = old.IntegerLiteralSeparator.Binary;
  retval.IntegerLiteralSeparator.BinaryMinDigits =
      old.IntegerLiteralSeparator.BinaryMinDigits;
  retval.IntegerLiteralSeparator.Decimal = old.IntegerLiteralSeparator.Decimal;
  retval.IntegerLiteralSeparator.DecimalMinDigits =
      old.IntegerLiteralSeparator.DecimalMinDigits;
  retval.IntegerLiteralSeparator.Hex = old.IntegerLiteralSeparator.Hex;
  retval.IntegerLiteralSeparator.HexMinDigits =
      old.IntegerLiteralSeparator.HexMinDigits;
  retval.JavaImportGroups = old.JavaImportGroups;
  retval.JavaScriptQuotes = java_script_quote_style.at(old.JavaScriptQuotes);
  retval.JavaScriptWrapImports = old.JavaScriptWrapImports;
  assignWithWarning("KeepEmptyLinesAtEOF", old.KeepEmptyLinesAtEOF,
                    "KeepEmptyLines.AtEndOfFile",
                    retval.KeepEmptyLines.AtEndOfFile, old.KeepEmptyLinesAtEOF,
                    "19");
  assignWithWarning(
      "KeepEmptyLinesAtTheStartOfBlocks", old.KeepEmptyLinesAtTheStartOfBlocks,
      "KeepEmptyLines.AtStartOfBlock", retval.KeepEmptyLines.AtStartOfBlock,
      old.KeepEmptyLinesAtTheStartOfBlocks, "19");
  newField("KeepEmptyLines.AtStartOfFile", "19",
           retval.KeepEmptyLines.AtStartOfFile);
  retval.LambdaBodyIndentation =
      lambda_body_indentation_king.at(old.LambdaBodyIndentation);
  retval.Language = language_king.at(old.Language);
  retval.LineEnding = line_ending_style.at(old.LineEnding);
  retval.MacroBlockBegin = old.MacroBlockBegin;
  retval.MacroBlockEnd = old.MacroBlockEnd;
  retval.Macros = old.Macros;
  retval.MaxEmptyLinesToKeep = old.MaxEmptyLinesToKeep;

  retval.NamespaceIndentation =
      namespace_indentation_kind.at(old.NamespaceIndentation);
  retval.NamespaceMacros = old.NamespaceMacros;
  retval.ObjCBinPackProtocolList =
      bin_pack_style.at(old.ObjCBinPackProtocolList);
  retval.ObjCBlockIndentWidth = old.ObjCBlockIndentWidth;
  retval.ObjCBreakBeforeNestedBlockParam = old.ObjCBreakBeforeNestedBlockParam;
  retval.ObjCPropertyAttributeOrder = old.ObjCPropertyAttributeOrder;
  retval.ObjCSpaceAfterProperty = old.ObjCSpaceAfterProperty;
  retval.ObjCSpaceBeforeProtocolList = old.ObjCSpaceBeforeProtocolList;
  retval.PackConstructorInitializers =
      pack_constructor_initializers_style.at(old.PackConstructorInitializers);
  retval.PenaltyBreakAssignment = old.PenaltyBreakAssignment;
  retval.PenaltyBreakBeforeFirstCallParameter =
      old.PenaltyBreakBeforeFirstCallParameter;
  retval.PenaltyBreakComment = old.PenaltyBreakComment;
  retval.PenaltyBreakFirstLessLess = old.PenaltyBreakFirstLessLess;
  retval.PenaltyBreakOpenParenthesis = old.PenaltyBreakOpenParenthesis;
  retval.PenaltyBreakScopeResolution = old.PenaltyBreakScopeResolution;
  retval.PenaltyBreakString = old.PenaltyBreakString;
  retval.PenaltyBreakTemplateDeclaration = old.PenaltyBreakTemplateDeclaration;
  retval.PenaltyExcessCharacter = old.PenaltyExcessCharacter;
  retval.PenaltyIndentedWhitespace = old.PenaltyIndentedWhitespace;
  retval.PenaltyReturnTypeOnItsOwnLine = old.PenaltyReturnTypeOnItsOwnLine;
  retval.PointerAlignment = pointer_alignment_style.at(old.PointerAlignment);
  retval.PPIndentWidth = old.PPIndentWidth;
  retval.QualifierAlignment =
      qualifier_alignment_style.at(old.QualifierAlignment);
  retval.QualifierOrder = old.QualifierOrder;
  assign(old.RawStringFormats, retval.RawStringFormats);
  retval.ReferenceAlignment =
      reference_alignment_style.at(old.ReferenceAlignment);
  retval.ReflowComments = old.ReflowComments;
  retval.RemoveBracesLLVM = old.RemoveBracesLLVM;
  retval.RemoveParentheses = remove_parentheses_style.at(old.RemoveParentheses);
  retval.RemoveSemicolon = old.RemoveSemicolon;
  retval.RequiresClausePosition =
      requires_clause_position_style.at(old.RequiresClausePosition);
  retval.RequiresExpressionIndentation =
      requires_expression_indentation_kind.at(
          old.RequiresExpressionIndentation);
  retval.SeparateDefinitionBlocks =
      separate_definitions_style.at(old.SeparateDefinitionBlocks);
  retval.ShortNamespaceLines = old.ShortNamespaceLines;
  retval.SkipMacroDefinitionBody = old.SkipMacroDefinitionBody;
  retval.SortIncludes = sort_includes_options.at(old.SortIncludes);
  retval.SortJavaStaticImport =
      sort_java_static_import_options.at(old.SortJavaStaticImport);
  retval.SortUsingDeclarations =
      sort_using_declarations_options.at(old.SortUsingDeclarations);
  retval.SpaceAfterCStyleCast = old.SpaceAfterCStyleCast;
  retval.SpaceAfterLogicalNot = old.SpaceAfterLogicalNot;
  retval.SpaceAfterTemplateKeyword = old.SpaceAfterTemplateKeyword;
  retval.SpaceAroundPointerQualifiers =
      space_around_pointer_qualifiers_style.at(
          old.SpaceAroundPointerQualifiers);
  retval.SpaceBeforeAssignmentOperators = old.SpaceBeforeAssignmentOperators;
  retval.SpaceBeforeCaseColon = old.SpaceBeforeCaseColon;
  retval.SpaceBeforeCpp11BracedList = old.SpaceBeforeCpp11BracedList;
  retval.SpaceBeforeCtorInitializerColon = old.SpaceBeforeCtorInitializerColon;
  retval.SpaceBeforeInheritanceColon = old.SpaceBeforeInheritanceColon;
  retval.SpaceBeforeJsonColon = old.SpaceBeforeJsonColon;
  retval.SpaceBeforeParens =
      space_before_parens_options.at(old.SpaceBeforeParens);
  retval.SpaceBeforeParensOptions.AfterControlStatements =
      old.SpaceBeforeParensOptions.AfterControlStatements;
  retval.SpaceBeforeParensOptions.AfterForeachMacros =
      old.SpaceBeforeParensOptions.AfterForeachMacros;
  retval.SpaceBeforeParensOptions.AfterFunctionDeclarationName =
      old.SpaceBeforeParensOptions.AfterFunctionDeclarationName;
  retval.SpaceBeforeParensOptions.AfterFunctionDefinitionName =
      old.SpaceBeforeParensOptions.AfterFunctionDefinitionName;
  retval.SpaceBeforeParensOptions.AfterIfMacros =
      old.SpaceBeforeParensOptions.AfterIfMacros;
  retval.SpaceBeforeParensOptions.AfterOverloadedOperator =
      old.SpaceBeforeParensOptions.AfterOverloadedOperator;
  retval.SpaceBeforeParensOptions.AfterPlacementOperator =
      old.SpaceBeforeParensOptions.AfterPlacementOperator;
  retval.SpaceBeforeParensOptions.AfterRequiresInClause =
      old.SpaceBeforeParensOptions.AfterRequiresInClause;
  retval.SpaceBeforeParensOptions.AfterRequiresInExpression =
      old.SpaceBeforeParensOptions.AfterRequiresInExpression;
  retval.SpaceBeforeParensOptions.BeforeNonEmptyParentheses =
      old.SpaceBeforeParensOptions.BeforeNonEmptyParentheses;
  retval.SpaceBeforeSquareBrackets = old.SpaceBeforeSquareBrackets;
  retval.SpaceBeforeRangeBasedForLoopColon =
      old.SpaceBeforeRangeBasedForLoopColon;
  retval.SpaceInEmptyBlock = old.SpaceInEmptyBlock;
  retval.SpacesBeforeTrailingComments = old.SpacesBeforeTrailingComments;
  retval.SpacesInAngles = spaces_in_angles_style.at(old.SpacesInAngles);
  retval.SpacesInContainerLiterals = old.SpacesInContainerLiterals;
  retval.SpacesInLineCommentPrefix.Minimum =
      old.SpacesInLineCommentPrefix.Minimum;
  retval.SpacesInLineCommentPrefix.Maximum =
      old.SpacesInLineCommentPrefix.Maximum;
  retval.SpacesInParens = spaces_in_parens_style.at(old.SpacesInParens);
  newField("SpacesInParensOptions.ExceptDoubleParentheses", "19",
           retval.SpacesInParensOptions.ExceptDoubleParentheses);
  retval.SpacesInParensOptions.InConditionalStatements =
      old.SpacesInParensOptions.InConditionalStatements;
  retval.SpacesInParensOptions.InCStyleCasts =
      old.SpacesInParensOptions.InCStyleCasts;
  retval.SpacesInParensOptions.InEmptyParentheses =
      old.SpacesInParensOptions.InEmptyParentheses;
  retval.SpacesInParensOptions.Other = old.SpacesInParensOptions.Other;
  retval.SpacesInSquareBrackets = old.SpacesInSquareBrackets;
  retval.Standard = language_standard.at(old.Standard);
  retval.StatementAttributeLikeMacros = old.StatementAttributeLikeMacros;
  retval.StatementMacros = old.StatementMacros;
  newField("TableGenBreakingDAGArgOperators", "19",
           retval.TableGenBreakingDAGArgOperators);
  newField("TableGenBreakInsideDAGArg", "19", retval.TableGenBreakInsideDAGArg);
  retval.TabWidth = old.TabWidth;
  retval.TypeNames = old.TypeNames;
  retval.TypenameMacros = old.TypenameMacros;
  retval.UseTab = use_tab_style.at(old.UseTab);
  retval.VerilogBreakBetweenInstancePorts =
      old.VerilogBreakBetweenInstancePorts;
  retval.WhitespaceSensitiveMacros = old.WhitespaceSensitiveMacros;

  return retval;
}

} // namespace clang_update_v19
