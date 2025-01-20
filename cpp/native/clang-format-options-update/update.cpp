#include "update.h"
#include <frozen/unordered_map.h>
#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <stdexcept>
#include <string_view>

#define XSTR(S) STR(S)
#define STR(S) #S

template <typename K, typename V, size_t SIZE>
std::optional<K> getKeyFromValue(const frozen::unordered_map<K, V, SIZE> &myMap,
                                 const V &valueToFind) {
  for (const auto &pair : myMap) {
    if (pair.second == valueToFind) {
      return pair.first;
    }
  }
  return std::nullopt;
}

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
bool Enabled;
bool AcrossEmptyLines;
bool AcrossComments;
bool AlignCompound;
bool PadOperators;

std::ostream &
operator<<(std::ostream &os,
           const clang_v15::FormatStyle::AlignConsecutiveStyle &data) {
  os << "{" << "Enabled: " << data.Enabled
     << ", AcrossEmptyLines: " << data.AcrossEmptyLines
     << ", AcrossComments: " << data.AcrossComments
     << ", AlignCompound: " << data.AlignCompound
     << ", PadOperators: " << data.PadOperators << "}";
  return os;
}

std::ostream &
operator<<(std::ostream &os,
           const clang_v16::FormatStyle::TrailingCommentsAlignmentStyle &data) {
  os << "{" << "Kind: " << data.Kind
     << ", OverEmptyLines: " << data.OverEmptyLines << "}";
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

template <clang_vx::Update Upgrade, typename T, typename U>
void renameField(std::string_view old_field_name, T &old_field,
                 std::string_view new_field_name, U &new_field,
                 std::string_view version) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    std::cout << "Info when upgrading to version " << version << ". Old field "
              << old_field_name << " has been renamed to " << new_field_name
              << ".";
    new_field = old_field;
  } else {
    std::cout << "Info when downgrading to version " << version
              << ". New field " << new_field_name << " has been renamed to "
              << old_field_name << ".\n";
    old_field = new_field;
  }
}

template <typename T, typename U, size_t SIZE>
void renameAndSwitchToEnumUpgrade(std::string_view old_field_name, T &old_field,
                                  std::string_view new_field_name, U &new_field,
                                  const frozen::unordered_map<T, U, SIZE> &map,
                                  std::string_view version) {
  bool renamed = old_field_name != new_field_name;
  bool prefix = false;
  if (renamed) {
    std::cout << "Info when upgrading to version " << version << ". Old field "
              << old_field_name << " has been renamed to " << new_field_name
              << ".";
    prefix = true;
  }
  if constexpr (std::is_enum_v<U>) {
    for (auto ls1 : magic_enum::enum_values<U>()) {
      if (!getKeyFromValue(map, ls1)) {
        if (!prefix) {
          std::cout << "Info when upgrading to version " << version << ".";
          prefix = true;
        }
        std::cout << "\nField " << new_field_name << " has a new value "
                  << magic_enum::enum_name(ls1) << ".";
      }
    }
  }
  new_field = map.at(old_field);
  std::cout << "\n";
}

template <typename T, typename U, size_t SIZE>
void renameAndSwitchToEnumDowngrade(
    std::string_view old_field_name, T &old_field,
    std::string_view new_field_name, U &new_field,
    const frozen::unordered_map<T, U, SIZE> &map, std::string_view version) {
  bool renamed = old_field_name != new_field_name;
  std::optional<T> old_value = getKeyFromValue(map, new_field);
  if (!old_value) {
    std::cout << "Error when downgrading from version " << version << ".";
    if (renamed) {
      std::cout << " New field " << new_field_name << " has been renamed to "
                << old_field_name << ".";
    }
    std::cout << " Can't find a match of the new value ";
    if constexpr (std::is_enum_v<U>) {
      std::cout << magic_enum::enum_name(new_field);
    } else {
      std::cout << new_field;
    }
    std::cout << " to old field. Default old value " << old_field
              << " has been kept.\n";
  } else if (old_field != *old_value) {
    std::cout << "Info when downgrading from version " << version << ".";
    if (renamed) {
      std::cout << " New field " << new_field_name << " has been renamed to "
                << old_field_name << ".";
    }
    std::cout << " Overriding old field from value " << old_field << " to "
              << *old_value << " based on new value ";
    if constexpr (std::is_enum_v<U>) {
      std::cout << magic_enum::enum_name(new_field);
    } else {
      std::cout << new_field;
    }
    std::cout << ".\n";
    old_field = *old_value;
  } else {
    std::cout << "Info when downgrading to version " << version << ".";
    if (renamed) {
      std::cout << " New field " << new_field_name << " has been renamed to "
                << old_field_name << ".";
    }
    std::cout << "\n";
  }
  if constexpr (std::is_enum_v<U>) {
    for (auto ls1 : magic_enum::enum_values<U>()) {
      if (!getKeyFromValue(map, ls1)) {
        std::cout << "Field " << old_field_name << " has dropped value "
                  << magic_enum::enum_name(ls1) << ".\n";
      }
    }
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
  std::cout << " field " << field_name << " with value ";
  if constexpr (std::is_enum_v<T>) {
    std::cout << magic_enum::enum_name(field_value);
  } else {
    std::cout << field_value;
  }
  std::cout << ".\n";
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

template <clang_vx::Update Upgrade, typename T, typename U>
void assignMagicEnum(T &old_field, U &new_field, std::string_view version) {
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
    bool missing_enum = false;
    for (auto ls1 : magic_enum::enum_values<U>()) {
      if (!magic_enum::enum_cast<T>(magic_enum::enum_name(ls1)).has_value()) {
        if (new_field == ls1) {
          missing_enum = true;
          std::cout << "Error when downgrading from version " << version
                    << ". Enum " << magic_enum::enum_type_name<U>()
                    << "::" << magic_enum::enum_name(ls1)
                    << " is removed and was used. Value "
                    << magic_enum::enum_name(old_field) << " will be used\n";
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
    if (!missing_enum) {
      old_field =
          magic_enum::enum_cast<T>(magic_enum::enum_name(new_field)).value();
    }
  }
}

template <clang_vx::Update Upgrade, typename T, typename U>
void assignIncludeCategory(std::vector<T> &old_field,
                           std::vector<U> &new_field) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    new_field.clear();
    new_field.reserve(old_field.size());
    for (const auto &item : old_field) {
      new_field.emplace_back(U{item.Regex, item.Priority});
    }
  } else {
    old_field.clear();
    old_field.reserve(new_field.size());
    for (const auto &item : new_field) {
      old_field.emplace_back(T{item.Regex, item.Priority});
    }
  }
}

template <clang_vx::Update Upgrade, typename T, typename U>
void assignIncludeCategory2(std::vector<T> &old_field,
                            std::vector<U> &new_field) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    new_field.clear();
    new_field.reserve(old_field.size());
    for (const auto &item : old_field) {
      new_field.emplace_back(U{item.Regex, item.Priority, item.SortPriority});
    }
  } else {
    old_field.clear();
    old_field.reserve(new_field.size());
    for (const auto &item : new_field) {
      old_field.emplace_back(T{item.Regex, item.Priority, item.SortPriority});
    }
  }
}

template <clang_vx::Update Upgrade, typename T, typename U>
void assignIncludeCategory3(std::vector<T> &old_field,
                            std::vector<U> &new_field) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    new_field.clear();
    new_field.reserve(old_field.size());
    for (const auto &item : old_field) {
      new_field.emplace_back(U{item.Regex, item.Priority, item.SortPriority,
                               item.RegexIsCaseSensitive});
    }
  } else {
    old_field.clear();
    old_field.reserve(new_field.size());
    for (const auto &item : new_field) {
      old_field.emplace_back(T{item.Regex, item.Priority, item.SortPriority,
                               item.RegexIsCaseSensitive});
    }
  }
}

template <clang_vx::Update Upgrade, typename T, typename U>
void assignRawStringFormat(
    std::vector<typename T::RawStringFormat> &old_field,
    std::vector<typename U::RawStringFormat> &new_field) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    new_field.clear();
    new_field.reserve(old_field.size());
    for (const auto &item : old_field) {
      new_field.emplace_back(typename U::RawStringFormat{
          magic_enum::enum_cast<typename U::LanguageKind>(
              magic_enum::enum_name(item.Language))
              .value(),
          item.Delimiters, item.EnclosingFunctions, item.CanonicalDelimiter,
          item.BasedOnStyle});
    }
  } else {
    old_field.clear();
    old_field.reserve(new_field.size());
    for (const auto &item : new_field) {
      old_field.emplace_back(typename T::RawStringFormat{
          magic_enum::enum_cast<typename T::LanguageKind>(
              magic_enum::enum_name(item.Language))
              .value(),
          item.Delimiters, item.EnclosingFunctions, item.CanonicalDelimiter,
          item.BasedOnStyle});
    }
  }
}

} // namespace

#define ASSIGN_SAME_FIELD(FIELD)                                               \
  assignSameField<Upgrade>(prev.FIELD, next.FIELD)
#define ASSIGN_MAGIC_ENUM(FIELD)                                               \
  assignMagicEnum<Upgrade>(prev.FIELD, next.FIELD, next_version)
#define RENAME_MAGIC_ENUM(OLD_FIELD, NEW_FIELD)                                \
  assignMagicEnum<Upgrade>(prev.OLD_FIELD, next.NEW_FIELD, next_version)
#define NEW_FIELD(FIELD) newField<Upgrade>(STR(FIELD), next_version, next.FIELD)
#define RENAME_AND_SWITCH_TO_ENUM(OLD_FIELD, NEW_FIELD, MAP)                   \
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {                        \
    renameAndSwitchToEnumUpgrade(STR(OLD_FIELD), prev.OLD_FIELD,               \
                                 STR(NEW_FIELD), next.NEW_FIELD, MAP,          \
                                 next_version);                                \
  } else {                                                                     \
    renameAndSwitchToEnumDowngrade(STR(OLD_FIELD), prev.OLD_FIELD,             \
                                   STR(NEW_FIELD), next.NEW_FIELD, MAP,        \
                                   next_version);                              \
  }                                                                            \
  static_assert(true)
#define SWITCH_TO_ENUM(FIELD, MAP)                                             \
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {                        \
    renameAndSwitchToEnumUpgrade(STR(FIELD), prev.FIELD, STR(FIELD),           \
                                 next.FIELD, MAP, next_version);               \
  } else {                                                                     \
    renameAndSwitchToEnumDowngrade(STR(FIELD), prev.FIELD, STR(FIELD),         \
                                   next.FIELD, MAP, next_version);             \
  }                                                                            \
  static_assert(true)
#define RENAME_FIELD(OLD_FIELD, NEW_FIELD)                                     \
  renameField<Upgrade>(STR(OLD_FIELD), prev.OLD_FIELD, STR(NEW_FIELD),         \
                       next.NEW_FIELD, next_version)
#define ASSIGN_INCLUDE_CATEGORY(FIELD)                                         \
  assignIncludeCategory<Upgrade>(prev.FIELD, next.FIELD)
#define ASSIGN_INCLUDE_CATEGORY_RENAME(OLD_FIELD, NEW_FIELD)                   \
  assignIncludeCategory<Upgrade>(prev.OLD_FIELD, next.NEW_FIELD)
#define ASSIGN_INCLUDE_CATEGORY2(FIELD)                                        \
  assignIncludeCategory2<Upgrade>(prev.FIELD, next.FIELD)
#define ASSIGN_INCLUDE_CATEGORY3(FIELD)                                        \
  assignIncludeCategory3<Upgrade>(prev.FIELD, next.FIELD)
#define ASSIGN_RAW_STRING_FORMAT(FIELD, OLD_VERSION, NEW_VERSION)              \
  assignRawStringFormat<Upgrade, clang_v##OLD_VERSION::FormatStyle,            \
                        clang_v##NEW_VERSION::FormatStyle>(prev.FIELD,         \
                                                           next.FIELD)

namespace clang_update_v3_4 {

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
  ASSIGN_MAGIC_ENUM(Standard);
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

template <clang_vx::Update Upgrade>
void update(clang_v3_4::FormatStyle &prev, clang_v3_5::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v3_5::getPredefinedStyle(
            style, clang_v3_5::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v3_4::getPredefinedStyle(style, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "3.4";
  std::string_view next_version = "3.5";

  NEW_FIELD(Language);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  NEW_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  RENAME_AND_SWITCH_TO_ENUM(PointerBindsToType, PointerAlignment,
                            pointer_alignment);
  RENAME_FIELD(DerivePointerBinding, DerivePointerAlignment);
  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  RENAME_FIELD(IndentFunctionDeclarationAfterType, IndentWrappedFunctionNames);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(BreakConstructorInitializersBeforeComma);
  NEW_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  NEW_FIELD(AllowShortFunctionsOnASingleLine);
  NEW_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AlignEscapedNewlinesLeft);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(UseTab);
  ASSIGN_SAME_FIELD(BreakBeforeBinaryOperators);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  NEW_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  RENAME_AND_SWITCH_TO_ENUM(SpaceAfterControlStatementKeyword,
                            SpaceBeforeParens, space_before_parens_options);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  NEW_FIELD(CommentPragmas);
  NEW_FIELD(DisableFormat);
  NEW_FIELD(ForEachMacros);
}

template void update<clang_vx::Update::UPGRADE>(clang_v3_4::FormatStyle &prev,
                                                clang_v3_5::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v3_4::FormatStyle &prev,
                                                  clang_v3_5::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v3_5

namespace clang_update_v3_6 {

constexpr frozen::unordered_map<bool,
                                clang_v3_6::FormatStyle::BinaryOperatorStyle, 2>
    binary_operator_style{
        {false, clang_v3_6::FormatStyle::BinaryOperatorStyle::BOS_None},
        {true, clang_v3_6::FormatStyle::BinaryOperatorStyle::BOS_All}};

template <clang_vx::Update Upgrade>
void update(clang_v3_5::FormatStyle &prev, clang_v3_6::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v3_6::getPredefinedStyle(
            style, clang_v3_6::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v3_5::getPredefinedStyle(
            style, clang_v3_5::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "3.5";
  std::string_view next_version = "3.6";

  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  NEW_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(BreakConstructorInitializersBeforeComma);
  ASSIGN_SAME_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  NEW_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  NEW_FIELD(AlignAfterOpenBracket);
  NEW_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AlignEscapedNewlinesLeft);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  NEW_FIELD(ObjCBlockIndentWidth);
  NEW_FIELD(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(UseTab);
  SWITCH_TO_ENUM(BreakBeforeBinaryOperators, binary_operator_style);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  NEW_FIELD(SpacesInSquareBrackets);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  NEW_FIELD(SpaceAfterCStyleCast);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ForEachMacros);
}

template void update<clang_vx::Update::UPGRADE>(clang_v3_5::FormatStyle &prev,
                                                clang_v3_6::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v3_5::FormatStyle &prev,
                                                  clang_v3_6::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v3_6

namespace clang_update_v3_7 {

constexpr frozen::unordered_map<
    bool, clang_v3_7::FormatStyle::DefinitionReturnTypeBreakingStyle, 2>
    definition_return_type_breaking_style{
        {false, clang_v3_7::FormatStyle::DefinitionReturnTypeBreakingStyle::
                    DRTBS_None},
        {true, clang_v3_7::FormatStyle::DefinitionReturnTypeBreakingStyle::
                   DRTBS_All}};

template <clang_vx::Update Upgrade>
void update(clang_v3_6::FormatStyle &prev, clang_v3_7::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v3_7::getPredefinedStyle(
            style, clang_v3_7::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v3_6::getPredefinedStyle(
            style, clang_v3_6::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "3.6";
  std::string_view next_version = "3.7";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_SAME_FIELD(AlignAfterOpenBracket);
  NEW_FIELD(AlignConsecutiveAssignments);
  ASSIGN_SAME_FIELD(AlignEscapedNewlinesLeft);
  ASSIGN_SAME_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  SWITCH_TO_ENUM(AlwaysBreakAfterDefinitionReturnType,
                 definition_return_type_breaking_style);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_SAME_FIELD(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_SAME_FIELD(BreakConstructorInitializersBeforeComma);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  NEW_FIELD(MacroBlockBegin);
  NEW_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v3_6::FormatStyle &prev,
                                                clang_v3_7::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v3_6::FormatStyle &prev,
                                                  clang_v3_7::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v3_7

namespace clang_update_v3_8 {

constexpr frozen::unordered_map<
    bool, clang_v3_8::FormatStyle::BracketAlignmentStyle, 2>
    bracket_all_alignment_style{
        {false, clang_v3_8::FormatStyle::BracketAlignmentStyle::BAS_DontAlign},
        {true, clang_v3_8::FormatStyle::BracketAlignmentStyle::BAS_Align}};

template <clang_vx::Update Upgrade>
void update(clang_v3_7::FormatStyle &prev, clang_v3_8::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v3_8::getPredefinedStyle(
            style, clang_v3_8::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v3_7::getPredefinedStyle(
            style, clang_v3_7::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "3.7";
  std::string_view next_version = "3.8";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  SWITCH_TO_ENUM(AlignAfterOpenBracket, bracket_all_alignment_style);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments);
  NEW_FIELD(AlignConsecutiveDeclarations);
  ASSIGN_SAME_FIELD(AlignEscapedNewlinesLeft);
  ASSIGN_SAME_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  NEW_FIELD(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_SAME_FIELD(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  NEW_FIELD(BraceWrapping.AfterClass);
  NEW_FIELD(BraceWrapping.AfterControlStatement);
  NEW_FIELD(BraceWrapping.AfterEnum);
  NEW_FIELD(BraceWrapping.AfterFunction);
  NEW_FIELD(BraceWrapping.AfterNamespace);
  NEW_FIELD(BraceWrapping.AfterObjCDeclaration);
  NEW_FIELD(BraceWrapping.AfterStruct);
  NEW_FIELD(BraceWrapping.AfterUnion);
  NEW_FIELD(BraceWrapping.BeforeCatch);
  NEW_FIELD(BraceWrapping.BeforeElse);
  NEW_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_SAME_FIELD(BreakConstructorInitializersBeforeComma);
  NEW_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(ForEachMacros);
  NEW_FIELD(IncludeCategories);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  NEW_FIELD(ReflowComments);
  NEW_FIELD(SortIncludes);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v3_7::FormatStyle &prev,
                                                clang_v3_8::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v3_7::FormatStyle &prev,
                                                  clang_v3_8::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v3_8

namespace clang_update_v3_9 {

template <clang_vx::Update Upgrade>
void update(clang_v3_8::FormatStyle &prev, clang_v3_9::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v3_9::getPredefinedStyle(
            style, clang_v3_9::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v3_8::getPredefinedStyle(
            style, clang_v3_8::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "3.8";
  std::string_view next_version = "3.9";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations);
  ASSIGN_SAME_FIELD(AlignEscapedNewlinesLeft);
  ASSIGN_SAME_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_SAME_FIELD(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_SAME_FIELD(BreakConstructorInitializersBeforeComma);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  NEW_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_INCLUDE_CATEGORY(IncludeCategories);
  NEW_FIELD(IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  NEW_FIELD(JavaScriptQuotes);
  NEW_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(SortIncludes);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v3_8::FormatStyle &prev,
                                                clang_v3_9::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v3_8::FormatStyle &prev,
                                                  clang_v3_9::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v3_9

namespace clang_update_v4 {

template <clang_vx::Update Upgrade>
void update(clang_v3_9::FormatStyle &prev, clang_v4::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v4::getPredefinedStyle(
            style, clang_v4::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v3_9::getPredefinedStyle(
            style, clang_v3_9::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "3.9";
  std::string_view next_version = "4";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations);
  ASSIGN_SAME_FIELD(AlignEscapedNewlinesLeft);
  ASSIGN_SAME_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_SAME_FIELD(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_SAME_FIELD(BreakConstructorInitializersBeforeComma);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_INCLUDE_CATEGORY(IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(SortIncludes);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  NEW_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v3_9::FormatStyle &prev,
                                                clang_v4::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v3_9::FormatStyle &prev,
                                                  clang_v4::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v4

namespace clang_update_v5 {

constexpr frozen::unordered_map<
    bool, clang_v5::FormatStyle::EscapedNewlineAlignmentStyle, 2>
    escaped_new_line_alignment_style{
        {false,
         clang_v5::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Right},
        {true, clang_v5::FormatStyle::EscapedNewlineAlignmentStyle::ENAS_Left}};

constexpr frozen::unordered_map<
    bool, clang_v5::FormatStyle::BreakConstructorInitializersStyle, 2>
    break_constructor_initializers_style{
        {false, clang_v5::FormatStyle::BreakConstructorInitializersStyle::
                    BCIS_AfterColon},
        {true, clang_v5::FormatStyle::BreakConstructorInitializersStyle::
                   BCIS_BeforeComma}};

template <clang_vx::Update Upgrade>
void update(clang_v4::FormatStyle &prev, clang_v5::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v5::getPredefinedStyle(
            style, clang_v5::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v4::getPredefinedStyle(
            style, clang_v4::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "4";
  std::string_view next_version = "5";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations);
  RENAME_AND_SWITCH_TO_ENUM(AlignEscapedNewlinesLeft, AlignEscapedNewlines,
                            escaped_new_line_alignment_style);
  ASSIGN_SAME_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_SAME_FIELD(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  NEW_FIELD(BraceWrapping.SplitEmptyFunction);
  NEW_FIELD(BraceWrapping.SplitEmptyRecord);
  NEW_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  RENAME_AND_SWITCH_TO_ENUM(BreakConstructorInitializersBeforeComma,
                            BreakConstructorInitializers,
                            break_constructor_initializers_style);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  NEW_FIELD(BreakBeforeInheritanceComma);
  NEW_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  NEW_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_INCLUDE_CATEGORY(IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  NEW_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(SortIncludes);
  NEW_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v4::FormatStyle &prev,
                                                clang_v5::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v4::FormatStyle &prev,
                                                  clang_v5::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v5

namespace clang_update_v6 {

template <clang_vx::Update Upgrade>
void update(clang_v5::FormatStyle &prev, clang_v6::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v6::getPredefinedStyle(
            style, clang_v6::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v5::getPredefinedStyle(
            style, clang_v5::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "5";
  std::string_view next_version = "6";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_SAME_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_SAME_FIELD(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  NEW_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  NEW_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_SAME_FIELD(BreakBeforeInheritanceComma);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  NEW_FIELD(IncludeBlocks);
  ASSIGN_INCLUDE_CATEGORY(IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  NEW_FIELD(IndentPPDirectives);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  NEW_FIELD(RawStringFormats);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(SortIncludes);
  ASSIGN_SAME_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v5::FormatStyle &prev,
                                                clang_v6::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v5::FormatStyle &prev,
                                                  clang_v6::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v6

namespace clang_update_v7 {

constexpr frozen::unordered_map<
    bool, clang_v7::FormatStyle::BreakTemplateDeclarationsStyle, 2>
    break_template_declarations_style{
        {false,
         clang_v7::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_MultiLine},
        {true,
         clang_v7::FormatStyle::BreakTemplateDeclarationsStyle::BTDS_Yes}};

constexpr frozen::unordered_map<
    bool, clang_v7::FormatStyle::BreakInheritanceListStyle, 2>
    break_inheritance_list_style{
        {false,
         clang_v7::FormatStyle::BreakInheritanceListStyle::BILS_BeforeColon},
        {true,
         clang_v7::FormatStyle::BreakInheritanceListStyle::BILS_BeforeComma}};

template <clang_vx::Update Upgrade>
void assign(std::vector<clang_v6::FormatStyle::RawStringFormat> &old_field,
            std::vector<clang_v7::FormatStyle::RawStringFormat> &new_field) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    new_field.clear();
    new_field.reserve(old_field.size());
    for (const auto &item : old_field) {
      new_field.emplace_back(clang_v7::FormatStyle::RawStringFormat{
          magic_enum::enum_cast<clang_v7::FormatStyle::LanguageKind>(
              magic_enum::enum_name(item.Language))
              .value(),
          {item.Delimiter},
          {},
          {},
          item.BasedOnStyle});
    }
  } else {
    old_field.clear();
    old_field.reserve(new_field.size());
    for (const auto &item : new_field) {
      old_field.emplace_back(clang_v6::FormatStyle::RawStringFormat{
          item.Delimiters.empty() ? "" : item.Delimiters.front(),
          magic_enum::enum_cast<clang_v6::FormatStyle::LanguageKind>(
              magic_enum::enum_name(item.Language))
              .value(),
          item.BasedOnStyle});
      std::cout << "Warning when downgrading from version 7, fields "
                   "RawStringFormats.EnclosingFunctions ("
                << item.EnclosingFunctions
                << "), RawStringFormats.CanonicalDelimiter ("
                << item.CanonicalDelimiter << ") has been dropped.";
      if (item.Delimiters.size() > 1) {
        std::vector<std::string> delimiters_truncated = item.Delimiters;
        delimiters_truncated.erase(delimiters_truncated.begin());
        std::cout << " Values from RawStringFormats.Delimiters ("
                  << delimiters_truncated << ") is also dropped.";
      }
      std::cout << "\n";
    }
  }
}

template <clang_vx::Update Upgrade>
void update(clang_v6::FormatStyle &prev, clang_v7::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v7::getPredefinedStyle(
            style, clang_v7::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v6::getPredefinedStyle(
            style, clang_v6::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "6";
  std::string_view next_version = "7";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_SAME_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  SWITCH_TO_ENUM(AlwaysBreakTemplateDeclarations,
                 break_template_declarations_style);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  RENAME_AND_SWITCH_TO_ENUM(BreakBeforeInheritanceComma, BreakInheritanceList,
                            break_inheritance_list_style);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_INCLUDE_CATEGORY_RENAME(IncludeCategories,
                                 IncludeStyle.IncludeCategories);
  RENAME_MAGIC_ENUM(IncludeBlocks, IncludeStyle.IncludeBlocks);
  RENAME_FIELD(IncludeIsMainRegex, IncludeStyle.IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  NEW_FIELD(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  NEW_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  assign<Upgrade>(prev.RawStringFormats, next.RawStringFormats);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(SortIncludes);
  ASSIGN_SAME_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  NEW_FIELD(SpaceBeforeCpp11BracedList);
  NEW_FIELD(SpaceBeforeCtorInitializerColon);
  NEW_FIELD(SpaceBeforeInheritanceColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  NEW_FIELD(SpaceBeforeRangeBasedForLoopColon);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v6::FormatStyle &prev,
                                                clang_v7::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v6::FormatStyle &prev,
                                                  clang_v7::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v7

namespace clang_update_v8 {

template <clang_vx::Update Upgrade>
void update(clang_v7::FormatStyle &prev, clang_v8::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v8::getPredefinedStyle(
            style, clang_v8::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v7::getPredefinedStyle(
            style, clang_v7::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "7";
  std::string_view next_version = "8";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_SAME_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortIfStatementsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  NEW_FIELD(StatementMacros);
  ASSIGN_MAGIC_ENUM(IncludeStyle.IncludeBlocks);
  ASSIGN_INCLUDE_CATEGORY(IncludeStyle.IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  NEW_FIELD(JavaImportGroups);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  NEW_FIELD(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  NEW_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 7, 8);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(SortIncludes);
  ASSIGN_SAME_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(SpaceBeforeCpp11BracedList);
  ASSIGN_SAME_FIELD(SpaceBeforeCtorInitializerColon);
  ASSIGN_SAME_FIELD(SpaceBeforeInheritanceColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeRangeBasedForLoopColon);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v7::FormatStyle &prev,
                                                clang_v8::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v7::FormatStyle &prev,
                                                  clang_v8::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v8

namespace clang_update_v9 {

constexpr frozen::unordered_map<bool, clang_v9::FormatStyle::ShortIfStyle, 2>
    short_if_style{
        {false, clang_v9::FormatStyle::ShortIfStyle::SIS_Never},
        {true, clang_v9::FormatStyle::ShortIfStyle::SIS_WithoutElse}};

template <clang_vx::Update Upgrade>
void update(clang_v8::FormatStyle &prev, clang_v9::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v9::getPredefinedStyle(
            style, clang_v9::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v8::getPredefinedStyle(
            style, clang_v8::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "8";
  std::string_view next_version = "9";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  NEW_FIELD(AlignConsecutiveMacros);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_SAME_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  NEW_FIELD(AllowAllArgumentsOnNextLine);
  NEW_FIELD(AllowAllConstructorInitializersOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  SWITCH_TO_ENUM(AllowShortIfStatementsOnASingleLine, short_if_style);
  NEW_FIELD(AllowShortLambdasOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  NEW_FIELD(BraceWrapping.AfterCaseLabel);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  NEW_FIELD(TypenameMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  NEW_FIELD(NamespaceMacros);
  ASSIGN_MAGIC_ENUM(IncludeStyle.IncludeBlocks);
  ASSIGN_INCLUDE_CATEGORY(IncludeStyle.IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_SAME_FIELD(JavaImportGroups);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 8, 9);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(SortIncludes);
  ASSIGN_SAME_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  NEW_FIELD(SpaceAfterLogicalNot);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(SpaceBeforeCpp11BracedList);
  ASSIGN_SAME_FIELD(SpaceBeforeCtorInitializerColon);
  ASSIGN_SAME_FIELD(SpaceBeforeInheritanceColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeRangeBasedForLoopColon);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v8::FormatStyle &prev,
                                                clang_v9::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v8::FormatStyle &prev,
                                                  clang_v9::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v9

namespace clang_update_v10 {

constexpr frozen::unordered_map<bool, clang_v10::FormatStyle::ShortBlockStyle,
                                2>
    short_block_style{
        {false, clang_v10::FormatStyle::ShortBlockStyle::SBS_Never},
        {true, clang_v10::FormatStyle::ShortBlockStyle::SBS_Always}};

constexpr frozen::unordered_map<
    bool, clang_v10::FormatStyle::BraceWrappingAfterControlStatementStyle, 2>
    brace_wrapping_after_control_statement_style{
        {false, clang_v10::FormatStyle::
                    BraceWrappingAfterControlStatementStyle::BWACS_Never},
        {true, clang_v10::FormatStyle::BraceWrappingAfterControlStatementStyle::
                   BWACS_Always}};

template <clang_vx::Update Upgrade>
void assign(std::vector<clang_v9::IncludeStyle::IncludeCategory> &old_field,
            std::vector<clang_v10::IncludeStyle::IncludeCategory> &new_field) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    new_field.clear();
    new_field.reserve(old_field.size());
    for (const auto &item : old_field) {
      new_field.emplace_back(clang_v10::IncludeStyle::IncludeCategory{
          item.Regex, item.Priority, {}});
    }
  } else {
    old_field.clear();
    old_field.reserve(new_field.size());
    for (const auto &item : new_field) {
      old_field.emplace_back(
          clang_v9::IncludeStyle::IncludeCategory{item.Regex, item.Priority});
      std::cout << "Warning when downgrading from version 10, field "
                   "IncludeStyle.SortPriority ("
                << item.SortPriority << ") have been dropped.\n";
    }
  }
}

template <clang_vx::Update Upgrade>
void update(clang_v9::FormatStyle &prev, clang_v10::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v10::getPredefinedStyle(
            style, clang_v10::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v9::getPredefinedStyle(
            style, clang_v9::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "9";
  std::string_view next_version = "10";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_SAME_FIELD(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllArgumentsOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllConstructorInitializersOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  SWITCH_TO_ENUM(AllowShortBlocksOnASingleLine, short_block_style);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortIfStatementsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortLambdasOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterCaseLabel);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  SWITCH_TO_ENUM(BraceWrapping.AfterControlStatement,
                 brace_wrapping_after_control_statement_style);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  NEW_FIELD(DeriveLineEnding);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_SAME_FIELD(TypenameMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  ASSIGN_SAME_FIELD(NamespaceMacros);
  ASSIGN_MAGIC_ENUM(IncludeStyle.IncludeBlocks);
  assign<Upgrade>(prev.IncludeStyle.IncludeCategories,
                  next.IncludeStyle.IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainRegex);
  NEW_FIELD(IncludeStyle.IncludeIsMainSourceRegex);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  NEW_FIELD(IndentGotoLabels);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_SAME_FIELD(JavaImportGroups);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 9, 10);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(SortIncludes);
  ASSIGN_SAME_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterLogicalNot);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(SpaceBeforeCpp11BracedList);
  ASSIGN_SAME_FIELD(SpaceBeforeCtorInitializerColon);
  ASSIGN_SAME_FIELD(SpaceBeforeInheritanceColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeRangeBasedForLoopColon);
  NEW_FIELD(SpaceInEmptyBlock);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  NEW_FIELD(SpacesInConditionalStatement);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  NEW_FIELD(SpaceBeforeSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  NEW_FIELD(UseCRLF);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v9::FormatStyle &prev,
                                                clang_v10::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v9::FormatStyle &prev,
                                                  clang_v10::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v10

namespace clang_update_v11 {

constexpr frozen::unordered_map<
    bool, clang_v11::FormatStyle::OperandAlignmentStyle, 2>
    operand_alignment_style{
        {false, clang_v11::FormatStyle::OperandAlignmentStyle::OAS_DontAlign},
        {true, clang_v11::FormatStyle::OperandAlignmentStyle::OAS_Align}};

template <clang_vx::Update Upgrade>
void update(clang_v10::FormatStyle &prev, clang_v11::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v11::getPredefinedStyle(
            style, clang_v11::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v10::getPredefinedStyle(
            style, clang_v10::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "10";
  std::string_view next_version = "11";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments);
  NEW_FIELD(AlignConsecutiveBitFields);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  SWITCH_TO_ENUM(AlignOperands, operand_alignment_style);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllArgumentsOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllConstructorInitializersOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  NEW_FIELD(AllowShortEnumsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortIfStatementsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortLambdasOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(BinPackArguments);
  NEW_FIELD(InsertTrailingCommas);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterCaseLabel);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_MAGIC_ENUM(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  NEW_FIELD(BraceWrapping.BeforeLambdaBody);
  NEW_FIELD(BraceWrapping.BeforeWhile);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DeriveLineEnding);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_SAME_FIELD(TypenameMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  ASSIGN_SAME_FIELD(NamespaceMacros);
  NEW_FIELD(WhitespaceSensitiveMacros);
  ASSIGN_MAGIC_ENUM(IncludeStyle.IncludeBlocks);
  ASSIGN_INCLUDE_CATEGORY2(IncludeStyle.IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainRegex);
  NEW_FIELD(IncludeStyle.IncludeIsMainSourceRegex);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  NEW_FIELD(IndentCaseBlocks);
  ASSIGN_SAME_FIELD(IndentGotoLabels);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  NEW_FIELD(IndentExternBlock);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_SAME_FIELD(JavaImportGroups);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  NEW_FIELD(ObjCBreakBeforeNestedBlockParam);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 10, 11);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(SortIncludes);
  ASSIGN_SAME_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterLogicalNot);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(SpaceBeforeCpp11BracedList);
  ASSIGN_SAME_FIELD(SpaceBeforeCtorInitializerColon);
  ASSIGN_SAME_FIELD(SpaceBeforeInheritanceColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeRangeBasedForLoopColon);
  ASSIGN_SAME_FIELD(SpaceInEmptyBlock);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInConditionalStatement);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_SAME_FIELD(SpaceBeforeSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_SAME_FIELD(UseCRLF);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v10::FormatStyle &prev,
                                                clang_v11::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v10::FormatStyle &prev,
                                                  clang_v11::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v11

namespace clang_update_v12 {

constexpr frozen::unordered_map<
    bool, clang_v12::FormatStyle::AlignConsecutiveStyle, 2>
    align_consecutive_style{
        {false, clang_v12::FormatStyle::AlignConsecutiveStyle::ACS_None},
        {true, clang_v12::FormatStyle::AlignConsecutiveStyle::ACS_Consecutive}};

template <clang_vx::Update Upgrade>
void assign(std::vector<clang_v11::IncludeStyle::IncludeCategory> &old_field,
            std::vector<clang_v12::IncludeStyle::IncludeCategory> &new_field) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    new_field.clear();
    new_field.reserve(old_field.size());
    for (const auto &item : old_field) {
      new_field.emplace_back(clang_v12::IncludeStyle::IncludeCategory{
          item.Regex, item.Priority, item.SortPriority, false});
    }
  } else {
    old_field.clear();
    old_field.reserve(new_field.size());
    for (const auto &item : new_field) {
      old_field.emplace_back(clang_v11::IncludeStyle::IncludeCategory{
          item.Regex, item.Priority, item.SortPriority});
      std::cout << "Warning when downgrading from version 12, field "
                   "IncludeStyle.RegexIsCaseSensitive ("
                << item.RegexIsCaseSensitive << ") have been dropped.\n";
    }
  }
}

template <clang_vx::Update Upgrade>
void update(clang_v11::FormatStyle &prev, clang_v12::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v12::getPredefinedStyle(
            style, clang_v12::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v11::getPredefinedStyle(
            style, clang_v11::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "11";
  std::string_view next_version = "12";

  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  SWITCH_TO_ENUM(AlignConsecutiveMacros, align_consecutive_style);
  SWITCH_TO_ENUM(AlignConsecutiveAssignments, align_consecutive_style);
  SWITCH_TO_ENUM(AlignConsecutiveBitFields, align_consecutive_style);
  SWITCH_TO_ENUM(AlignConsecutiveDeclarations, align_consecutive_style);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_MAGIC_ENUM(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllArgumentsOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllConstructorInitializersOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortEnumsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortIfStatementsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortLambdasOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(AlwaysBreakTemplateDeclarations);
  NEW_FIELD(AttributeMacros);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_MAGIC_ENUM(InsertTrailingCommas);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterCaseLabel);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_MAGIC_ENUM(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeLambdaBody);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeWhile);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  NEW_FIELD(BreakBeforeConceptDeclarations);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DeriveLineEnding);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  NEW_FIELD(EmptyLineBeforeAccessModifier);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_SAME_FIELD(TypenameMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  ASSIGN_SAME_FIELD(NamespaceMacros);
  ASSIGN_SAME_FIELD(WhitespaceSensitiveMacros);
  ASSIGN_MAGIC_ENUM(IncludeStyle.IncludeBlocks);
  assign<Upgrade>(prev.IncludeStyle.IncludeCategories,
                  next.IncludeStyle.IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainSourceRegex);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentCaseBlocks);
  ASSIGN_SAME_FIELD(IndentGotoLabels);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  ASSIGN_MAGIC_ENUM(IndentExternBlock);
  NEW_FIELD(IndentRequires);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_SAME_FIELD(JavaImportGroups);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCBreakBeforeNestedBlockParam);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  NEW_FIELD(PenaltyIndentedWhitespace);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 11, 12);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(SortIncludes);
  NEW_FIELD(SortJavaStaticImport);
  ASSIGN_SAME_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterLogicalNot);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  NEW_FIELD(SpaceAroundPointerQualifiers);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  NEW_FIELD(SpaceBeforeCaseColon);
  ASSIGN_SAME_FIELD(SpaceBeforeCpp11BracedList);
  ASSIGN_SAME_FIELD(SpaceBeforeCtorInitializerColon);
  ASSIGN_SAME_FIELD(SpaceBeforeInheritanceColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeRangeBasedForLoopColon);
  ASSIGN_SAME_FIELD(SpaceInEmptyBlock);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_SAME_FIELD(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInConditionalStatement);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_SAME_FIELD(SpaceBeforeSquareBrackets);
  NEW_FIELD(BitFieldColonSpacing);
  ASSIGN_MAGIC_ENUM(Standard);
  NEW_FIELD(StatementAttributeLikeMacros);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_SAME_FIELD(UseCRLF);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v11::FormatStyle &prev,
                                                clang_v12::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v11::FormatStyle &prev,
                                                  clang_v12::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v12

namespace clang_update_v13 {

constexpr frozen::unordered_map<bool,
                                clang_v13::FormatStyle::SortIncludesOptions, 2>
    sort_includes_options{
        {false, clang_v13::FormatStyle::SortIncludesOptions::SI_Never},
        {true,
         clang_v13::FormatStyle::SortIncludesOptions::SI_CaseInsensitive}};

constexpr frozen::unordered_map<bool,
                                clang_v13::FormatStyle::SpacesInAnglesStyle, 2>
    spaces_in_angles_style{
        {false, clang_v13::FormatStyle::SpacesInAnglesStyle::SIAS_Never},
        {true, clang_v13::FormatStyle::SpacesInAnglesStyle::SIAS_Always}};

template <clang_vx::Update Upgrade>
void update(clang_v12::FormatStyle &prev, clang_v13::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v13::getPredefinedStyle(
            style, clang_v13::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v12::getPredefinedStyle(
            style, clang_v12::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "12";
  std::string_view next_version = "13";

  NEW_FIELD(InheritsParentConfig);
  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  NEW_FIELD(AlignArrayOfStructures);
  ASSIGN_MAGIC_ENUM(AlignConsecutiveMacros);
  ASSIGN_MAGIC_ENUM(AlignConsecutiveAssignments);
  ASSIGN_MAGIC_ENUM(AlignConsecutiveBitFields);
  ASSIGN_MAGIC_ENUM(AlignConsecutiveDeclarations);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_MAGIC_ENUM(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllArgumentsOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllConstructorInitializersOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortEnumsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortIfStatementsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortLambdasOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(AttributeMacros);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_MAGIC_ENUM(InsertTrailingCommas);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterCaseLabel);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_MAGIC_ENUM(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeLambdaBody);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeWhile);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_SAME_FIELD(BreakBeforeConceptDeclarations);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DeriveLineEnding);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  NEW_FIELD(EmptyLineAfterAccessModifier);
  ASSIGN_MAGIC_ENUM(EmptyLineBeforeAccessModifier);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  NEW_FIELD(IfMacros);
  ASSIGN_SAME_FIELD(TypenameMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  ASSIGN_SAME_FIELD(NamespaceMacros);
  ASSIGN_SAME_FIELD(WhitespaceSensitiveMacros);
  ASSIGN_MAGIC_ENUM(IncludeStyle.IncludeBlocks);
  ASSIGN_INCLUDE_CATEGORY3(IncludeStyle.IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainSourceRegex);
  NEW_FIELD(IndentAccessModifiers);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentCaseBlocks);
  ASSIGN_SAME_FIELD(IndentGotoLabels);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  ASSIGN_MAGIC_ENUM(IndentExternBlock);
  ASSIGN_SAME_FIELD(IndentRequires);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_SAME_FIELD(JavaImportGroups);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  NEW_FIELD(LambdaBodyIndentation);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCBreakBeforeNestedBlockParam);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_SAME_FIELD(PenaltyIndentedWhitespace);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  NEW_FIELD(PPIndentWidth);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 12, 13);
  NEW_FIELD(ReferenceAlignment);
  ASSIGN_SAME_FIELD(ReflowComments);
  NEW_FIELD(ShortNamespaceLines);
  SWITCH_TO_ENUM(SortIncludes, sort_includes_options);
  ASSIGN_MAGIC_ENUM(SortJavaStaticImport);
  ASSIGN_SAME_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterLogicalNot);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_MAGIC_ENUM(SpaceAroundPointerQualifiers);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(SpaceBeforeCaseColon);
  ASSIGN_SAME_FIELD(SpaceBeforeCpp11BracedList);
  ASSIGN_SAME_FIELD(SpaceBeforeCtorInitializerColon);
  ASSIGN_SAME_FIELD(SpaceBeforeInheritanceColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeRangeBasedForLoopColon);
  ASSIGN_SAME_FIELD(SpaceInEmptyBlock);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  SWITCH_TO_ENUM(SpacesInAngles, spaces_in_angles_style);
  ASSIGN_SAME_FIELD(SpacesInConditionalStatement);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  NEW_FIELD(SpacesInLineCommentPrefix.Minimum);
  NEW_FIELD(SpacesInLineCommentPrefix.Maximum);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_SAME_FIELD(SpaceBeforeSquareBrackets);
  ASSIGN_MAGIC_ENUM(BitFieldColonSpacing);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(StatementAttributeLikeMacros);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_SAME_FIELD(UseCRLF);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v12::FormatStyle &prev,
                                                clang_v13::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v12::FormatStyle &prev,
                                                  clang_v13::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v13

namespace clang_update_v14 {

template <clang_vx::Update Upgrade>
void update(clang_v13::FormatStyle &prev, clang_v14::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v14::getPredefinedStyle(
            style, clang_v14::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v13::getPredefinedStyle(
            style, clang_v13::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "13";
  std::string_view next_version = "14";

  ASSIGN_SAME_FIELD(InheritsParentConfig);
  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_MAGIC_ENUM(AlignArrayOfStructures);
  ASSIGN_MAGIC_ENUM(AlignConsecutiveMacros);
  ASSIGN_MAGIC_ENUM(AlignConsecutiveAssignments);
  ASSIGN_MAGIC_ENUM(AlignConsecutiveBitFields);
  ASSIGN_MAGIC_ENUM(AlignConsecutiveDeclarations);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_MAGIC_ENUM(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllArgumentsOnNextLine);
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    ASSIGN_SAME_FIELD(AllowAllConstructorInitializersOnNextLine);
  }
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortEnumsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortIfStatementsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortLambdasOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(AttributeMacros);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_MAGIC_ENUM(InsertTrailingCommas);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterCaseLabel);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_MAGIC_ENUM(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeLambdaBody);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeWhile);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_SAME_FIELD(BreakBeforeConceptDeclarations);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  NEW_FIELD(QualifierAlignment);
  NEW_FIELD(QualifierOrder);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    ASSIGN_SAME_FIELD(ConstructorInitializerAllOnOneLineOrOnePerLine);
  }
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DeriveLineEnding);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_MAGIC_ENUM(EmptyLineAfterAccessModifier);
  ASSIGN_MAGIC_ENUM(EmptyLineBeforeAccessModifier);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  NEW_FIELD(PackConstructorInitializers);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_SAME_FIELD(IfMacros);
  ASSIGN_SAME_FIELD(TypenameMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  ASSIGN_SAME_FIELD(NamespaceMacros);
  ASSIGN_SAME_FIELD(WhitespaceSensitiveMacros);
  ASSIGN_MAGIC_ENUM(IncludeStyle.IncludeBlocks);
  ASSIGN_INCLUDE_CATEGORY3(IncludeStyle.IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainSourceRegex);
  ASSIGN_SAME_FIELD(IndentAccessModifiers);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentCaseBlocks);
  ASSIGN_SAME_FIELD(IndentGotoLabels);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  ASSIGN_MAGIC_ENUM(IndentExternBlock);
  ASSIGN_SAME_FIELD(IndentRequires);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_SAME_FIELD(JavaImportGroups);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_MAGIC_ENUM(LambdaBodyIndentation);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCBreakBeforeNestedBlockParam);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  NEW_FIELD(PenaltyBreakOpenParenthesis);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_SAME_FIELD(PenaltyIndentedWhitespace);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(PPIndentWidth);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 13, 14);
  ASSIGN_MAGIC_ENUM(ReferenceAlignment);
  ASSIGN_SAME_FIELD(ReflowComments);
  NEW_FIELD(RemoveBracesLLVM);
  NEW_FIELD(SeparateDefinitionBlocks);
  ASSIGN_SAME_FIELD(ShortNamespaceLines);
  ASSIGN_MAGIC_ENUM(SortIncludes);
  ASSIGN_MAGIC_ENUM(SortJavaStaticImport);
  ASSIGN_SAME_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterLogicalNot);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_MAGIC_ENUM(SpaceAroundPointerQualifiers);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(SpaceBeforeCaseColon);
  ASSIGN_SAME_FIELD(SpaceBeforeCpp11BracedList);
  ASSIGN_SAME_FIELD(SpaceBeforeCtorInitializerColon);
  ASSIGN_SAME_FIELD(SpaceBeforeInheritanceColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  NEW_FIELD(SpaceBeforeParensOptions.AfterControlStatements);
  NEW_FIELD(SpaceBeforeParensOptions.AfterForeachMacros);
  NEW_FIELD(SpaceBeforeParensOptions.AfterFunctionDeclarationName);
  NEW_FIELD(SpaceBeforeParensOptions.AfterFunctionDefinitionName);
  NEW_FIELD(SpaceBeforeParensOptions.AfterIfMacros);
  NEW_FIELD(SpaceBeforeParensOptions.AfterOverloadedOperator);
  NEW_FIELD(SpaceBeforeParensOptions.BeforeNonEmptyParentheses);
  ASSIGN_SAME_FIELD(SpaceBeforeRangeBasedForLoopColon);
  ASSIGN_SAME_FIELD(SpaceInEmptyBlock);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_MAGIC_ENUM(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInConditionalStatement);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInLineCommentPrefix.Minimum);
  ASSIGN_SAME_FIELD(SpacesInLineCommentPrefix.Maximum);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_SAME_FIELD(SpaceBeforeSquareBrackets);
  ASSIGN_MAGIC_ENUM(BitFieldColonSpacing);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(StatementAttributeLikeMacros);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_SAME_FIELD(UseCRLF);
  ASSIGN_MAGIC_ENUM(UseTab);

  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    bool IsGoogleOrChromium = style == "google" || style == "chromium";
    bool OnCurrentLine = next.ConstructorInitializerAllOnOneLineOrOnePerLine;
    bool OnNextLine = next.AllowAllConstructorInitializersOnNextLine;
    if (!IsGoogleOrChromium) {
      if (next.PackConstructorInitializers ==
              clang_v14::FormatStyle::PackConstructorInitializersStyle::
                  PCIS_BinPack &&
          OnCurrentLine) {
        next.PackConstructorInitializers =
            OnNextLine ? clang_v14::FormatStyle::
                             PackConstructorInitializersStyle::PCIS_NextLine
                       : clang_v14::FormatStyle::
                             PackConstructorInitializersStyle::PCIS_CurrentLine;
      }
    } else if (next.PackConstructorInitializers ==
               clang_v14::FormatStyle::PackConstructorInitializersStyle::
                   PCIS_NextLine) {
      if (!OnCurrentLine)
        next.PackConstructorInitializers = clang_v14::FormatStyle::
            PackConstructorInitializersStyle::PCIS_BinPack;
      else if (!OnNextLine)
        next.PackConstructorInitializers = clang_v14::FormatStyle::
            PackConstructorInitializersStyle::PCIS_CurrentLine;
    }
  } else {
    prev.ConstructorInitializerAllOnOneLineOrOnePerLine =
        next.PackConstructorInitializers ==
            clang_v14::FormatStyle::PackConstructorInitializersStyle::
                PCIS_CurrentLine ||
        next.PackConstructorInitializers ==
            clang_v14::FormatStyle::PackConstructorInitializersStyle::
                PCIS_BinPack;

    prev.AllowAllConstructorInitializersOnNextLine =
        next.PackConstructorInitializers ==
            clang_v14::FormatStyle::PackConstructorInitializersStyle::
                PCIS_NextLine ||
        next.PackConstructorInitializers ==
            clang_v14::FormatStyle::PackConstructorInitializersStyle::
                PCIS_BinPack;

    std::cout << "Warning when downgrading from version 14. Field "
                 "ConstructorInitializerAllOnOneLineOrOnePerLine and "
                 "AllowAllConstructorInitializersOnNextLine is unsured.\n";
  }
}

template void update<clang_vx::Update::UPGRADE>(clang_v13::FormatStyle &prev,
                                                clang_v14::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v13::FormatStyle &prev,
                                                  clang_v14::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v14

namespace clang_update_v15 {

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
    bool, clang_v15::FormatStyle::BreakBeforeConceptDeclarationsStyle, 2>
    break_before_concept_declarations_style{
        {false, clang_v15::FormatStyle::BreakBeforeConceptDeclarationsStyle::
                    BBCDS_Allowed},
        {true, clang_v15::FormatStyle::BreakBeforeConceptDeclarationsStyle::
                   BBCDS_Always}};

template <clang_vx::Update Upgrade>
void update(clang_v14::FormatStyle &prev, clang_v15::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v15::getPredefinedStyle(
            style, clang_v15::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v14::getPredefinedStyle(
            style, clang_v14::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "14";
  std::string_view next_version = "15";

  ASSIGN_SAME_FIELD(InheritsParentConfig);
  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_MAGIC_ENUM(AlignArrayOfStructures);
  SWITCH_TO_ENUM(AlignConsecutiveMacros, align_consecutive_style);
  SWITCH_TO_ENUM(AlignConsecutiveAssignments, align_consecutive_style);
  SWITCH_TO_ENUM(AlignConsecutiveBitFields, align_consecutive_style);
  SWITCH_TO_ENUM(AlignConsecutiveDeclarations, align_consecutive_style);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_MAGIC_ENUM(AlignOperands);
  ASSIGN_SAME_FIELD(AlignTrailingComments);
  ASSIGN_SAME_FIELD(AllowAllArgumentsOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortEnumsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortIfStatementsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortLambdasOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(AttributeMacros);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_MAGIC_ENUM(InsertTrailingCommas);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterCaseLabel);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_MAGIC_ENUM(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeLambdaBody);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeWhile);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  SWITCH_TO_ENUM(BreakBeforeConceptDeclarations,
                 break_before_concept_declarations_style);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_MAGIC_ENUM(QualifierAlignment);
  ASSIGN_SAME_FIELD(QualifierOrder);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DeriveLineEnding);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_MAGIC_ENUM(EmptyLineAfterAccessModifier);
  ASSIGN_MAGIC_ENUM(EmptyLineBeforeAccessModifier);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_SAME_FIELD(IfMacros);
  ASSIGN_SAME_FIELD(TypenameMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  ASSIGN_SAME_FIELD(NamespaceMacros);
  ASSIGN_SAME_FIELD(WhitespaceSensitiveMacros);
  ASSIGN_MAGIC_ENUM(IncludeStyle.IncludeBlocks);
  ASSIGN_INCLUDE_CATEGORY3(IncludeStyle.IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainSourceRegex);
  ASSIGN_SAME_FIELD(IndentAccessModifiers);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentCaseBlocks);
  ASSIGN_SAME_FIELD(IndentGotoLabels);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  ASSIGN_MAGIC_ENUM(IndentExternBlock);
  RENAME_FIELD(IndentRequires, IndentRequiresClause);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  NEW_FIELD(InsertBraces);
  ASSIGN_SAME_FIELD(JavaImportGroups);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_MAGIC_ENUM(LambdaBodyIndentation);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCBreakBeforeNestedBlockParam);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakOpenParenthesis);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_SAME_FIELD(PenaltyIndentedWhitespace);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(PPIndentWidth);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 14, 15);
  ASSIGN_MAGIC_ENUM(ReferenceAlignment);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(RemoveBracesLLVM);
  NEW_FIELD(RequiresClausePosition);
  ASSIGN_MAGIC_ENUM(SeparateDefinitionBlocks);
  ASSIGN_SAME_FIELD(ShortNamespaceLines);
  ASSIGN_MAGIC_ENUM(SortIncludes);
  ASSIGN_MAGIC_ENUM(SortJavaStaticImport);
  ASSIGN_SAME_FIELD(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterLogicalNot);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_MAGIC_ENUM(SpaceAroundPointerQualifiers);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(SpaceBeforeCaseColon);
  ASSIGN_SAME_FIELD(SpaceBeforeCpp11BracedList);
  ASSIGN_SAME_FIELD(SpaceBeforeCtorInitializerColon);
  ASSIGN_SAME_FIELD(SpaceBeforeInheritanceColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterControlStatements);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterForeachMacros);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterFunctionDeclarationName);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterFunctionDefinitionName);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterIfMacros);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterOverloadedOperator);
  NEW_FIELD(SpaceBeforeParensOptions.AfterRequiresInClause);
  NEW_FIELD(SpaceBeforeParensOptions.AfterRequiresInExpression);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.BeforeNonEmptyParentheses);
  ASSIGN_SAME_FIELD(SpaceBeforeRangeBasedForLoopColon);
  ASSIGN_SAME_FIELD(SpaceInEmptyBlock);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_MAGIC_ENUM(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInConditionalStatement);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInLineCommentPrefix.Minimum);
  ASSIGN_SAME_FIELD(SpacesInLineCommentPrefix.Maximum);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_SAME_FIELD(SpaceBeforeSquareBrackets);
  ASSIGN_MAGIC_ENUM(BitFieldColonSpacing);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(StatementAttributeLikeMacros);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_SAME_FIELD(UseCRLF);
  ASSIGN_MAGIC_ENUM(UseTab);

  if constexpr (Upgrade == clang_vx::Update::DOWNGRADE) {
    prev.ConstructorInitializerAllOnOneLineOrOnePerLine =
        next.PackConstructorInitializers ==
            clang_v15::FormatStyle::PackConstructorInitializersStyle::
                PCIS_CurrentLine ||
        next.PackConstructorInitializers ==
            clang_v15::FormatStyle::PackConstructorInitializersStyle::
                PCIS_BinPack;

    prev.AllowAllConstructorInitializersOnNextLine =
        next.PackConstructorInitializers ==
            clang_v15::FormatStyle::PackConstructorInitializersStyle::
                PCIS_NextLine ||
        next.PackConstructorInitializers ==
            clang_v15::FormatStyle::PackConstructorInitializersStyle::
                PCIS_BinPack;

    std::cout << "Warning when downgrading from version 15. Field "
                 "ConstructorInitializerAllOnOneLineOrOnePerLine and "
                 "AllowAllConstructorInitializersOnNextLine is unsured.\n";
  }
}

template void update<clang_vx::Update::UPGRADE>(clang_v14::FormatStyle &prev,
                                                clang_v15::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v14::FormatStyle &prev,
                                                  clang_v15::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v15

namespace clang_update_v16 {

constexpr frozen::unordered_map<
    bool, clang_v16::FormatStyle::TrailingCommentsAlignmentStyle, 2>
    trailing_comments_alignment_style{
        {false,
         {clang_v16::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Never,
          0}},
        {true,
         {clang_v16::FormatStyle::TrailingCommentsAlignmentKinds::TCAS_Always,
          0}}};

constexpr frozen::unordered_map<
    bool, clang_v16::FormatStyle::SortUsingDeclarationsOptions, 2>
    sort_using_declarations_options{
        {false,
         clang_v16::FormatStyle::SortUsingDeclarationsOptions::SUD_Never},
        {true, clang_v16::FormatStyle::SortUsingDeclarationsOptions::
                   SUD_LexicographicNumeric}};

template <clang_vx::Update Upgrade>
void update(clang_v15::FormatStyle &prev, clang_v16::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v16::getPredefinedStyle(
            style, clang_v16::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v15::getPredefinedStyle(
            style, clang_v15::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "15";
  std::string_view next_version = "16";

  ASSIGN_SAME_FIELD(InheritsParentConfig);
  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_MAGIC_ENUM(AlignArrayOfStructures);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.PadOperators);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_MAGIC_ENUM(AlignOperands);
  SWITCH_TO_ENUM(AlignTrailingComments, trailing_comments_alignment_style);
  ASSIGN_SAME_FIELD(AllowAllArgumentsOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_SAME_FIELD(AllowShortEnumsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortIfStatementsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortLambdasOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(AttributeMacros);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_MAGIC_ENUM(InsertTrailingCommas);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterCaseLabel);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_MAGIC_ENUM(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeLambdaBody);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeWhile);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_MAGIC_ENUM(BreakBeforeConceptDeclarations);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  NEW_FIELD(BreakBeforeInlineASMColon);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_MAGIC_ENUM(QualifierAlignment);
  ASSIGN_SAME_FIELD(QualifierOrder);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    next.LineEnding =
        prev.DeriveLineEnding
            ? (prev.UseCRLF
                   ? clang_v16::FormatStyle::LineEndingStyle::LE_DeriveCRLF
                   : clang_v16::FormatStyle::LineEndingStyle::LE_DeriveLF)
            : (prev.UseCRLF ? clang_v16::FormatStyle::LineEndingStyle::LE_CRLF
                            : clang_v16::FormatStyle::LineEndingStyle::LE_LF);
  } else {
    prev.UseCRLF =
        next.LineEnding ==
            clang_v16::FormatStyle::LineEndingStyle::LE_DeriveCRLF ||
        next.LineEnding == clang_v16::FormatStyle::LineEndingStyle::LE_CRLF;
    prev.DeriveLineEnding =
        next.LineEnding ==
            clang_v16::FormatStyle::LineEndingStyle::LE_DeriveCRLF ||
        next.LineEnding == clang_v16::FormatStyle::LineEndingStyle::LE_DeriveLF;
  }
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_MAGIC_ENUM(EmptyLineAfterAccessModifier);
  ASSIGN_MAGIC_ENUM(EmptyLineBeforeAccessModifier);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_MAGIC_ENUM(PackConstructorInitializers);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_MAGIC_ENUM(IncludeStyle.IncludeBlocks);
  ASSIGN_INCLUDE_CATEGORY3(IncludeStyle.IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainSourceRegex);
  ASSIGN_SAME_FIELD(IfMacros);
  ASSIGN_SAME_FIELD(TypenameMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  ASSIGN_SAME_FIELD(NamespaceMacros);
  ASSIGN_SAME_FIELD(WhitespaceSensitiveMacros);
  ASSIGN_SAME_FIELD(IndentAccessModifiers);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentCaseBlocks);
  ASSIGN_SAME_FIELD(IndentGotoLabels);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  ASSIGN_MAGIC_ENUM(IndentExternBlock);
  ASSIGN_SAME_FIELD(IndentRequiresClause);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_SAME_FIELD(InsertBraces);
  ASSIGN_SAME_FIELD(JavaImportGroups);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_MAGIC_ENUM(LambdaBodyIndentation);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCBreakBeforeNestedBlockParam);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakOpenParenthesis);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_SAME_FIELD(PenaltyIndentedWhitespace);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(PPIndentWidth);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 15, 16);
  ASSIGN_MAGIC_ENUM(ReferenceAlignment);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(RemoveBracesLLVM);
  NEW_FIELD(RemoveSemicolon);
  NEW_FIELD(BreakAfterAttributes);
  NEW_FIELD(BreakArrays);
  NEW_FIELD(InsertNewlineAtEOF);
  NEW_FIELD(IntegerLiteralSeparator.Binary);
  NEW_FIELD(IntegerLiteralSeparator.BinaryMinDigits);
  NEW_FIELD(IntegerLiteralSeparator.Decimal);
  NEW_FIELD(IntegerLiteralSeparator.DecimalMinDigits);
  NEW_FIELD(IntegerLiteralSeparator.Hex);
  NEW_FIELD(IntegerLiteralSeparator.HexMinDigits);
  ASSIGN_MAGIC_ENUM(RequiresClausePosition);
  NEW_FIELD(RequiresExpressionIndentation);
  ASSIGN_MAGIC_ENUM(SeparateDefinitionBlocks);
  ASSIGN_SAME_FIELD(ShortNamespaceLines);
  ASSIGN_MAGIC_ENUM(SortIncludes);
  ASSIGN_MAGIC_ENUM(SortJavaStaticImport);
  SWITCH_TO_ENUM(SortUsingDeclarations, sort_using_declarations_options);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterLogicalNot);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_MAGIC_ENUM(SpaceAroundPointerQualifiers);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(SpaceBeforeCaseColon);
  ASSIGN_SAME_FIELD(SpaceBeforeCpp11BracedList);
  ASSIGN_SAME_FIELD(SpaceBeforeCtorInitializerColon);
  ASSIGN_SAME_FIELD(SpaceBeforeInheritanceColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterControlStatements);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterForeachMacros);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterFunctionDeclarationName);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterFunctionDefinitionName);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterIfMacros);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterOverloadedOperator);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterRequiresInClause);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterRequiresInExpression);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.BeforeNonEmptyParentheses);
  ASSIGN_SAME_FIELD(SpaceBeforeRangeBasedForLoopColon);
  ASSIGN_SAME_FIELD(SpaceInEmptyBlock);
  ASSIGN_SAME_FIELD(SpaceInEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_MAGIC_ENUM(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInConditionalStatement);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInCStyleCastParentheses);
  ASSIGN_SAME_FIELD(SpacesInLineCommentPrefix.Minimum);
  ASSIGN_SAME_FIELD(SpacesInLineCommentPrefix.Maximum);
  ASSIGN_SAME_FIELD(SpacesInParentheses);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_SAME_FIELD(SpaceBeforeSquareBrackets);
  ASSIGN_MAGIC_ENUM(BitFieldColonSpacing);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(StatementAttributeLikeMacros);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_MAGIC_ENUM(UseTab);
}

template void update<clang_vx::Update::UPGRADE>(clang_v15::FormatStyle &prev,
                                                clang_v16::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v15::FormatStyle &prev,
                                                  clang_v16::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v16

namespace clang_update_v17 {

constexpr frozen::unordered_map<
    bool, clang_v17::FormatStyle::SpacesInParensStyle, 2>
    spaces_in_parens_style{
        {false, clang_v17::FormatStyle::SpacesInParensStyle::SIPO_Never},
        {true, clang_v17::FormatStyle::SpacesInParensStyle::SIPO_Custom}};

template <clang_vx::Update Upgrade>
void update(clang_v16::FormatStyle &prev, clang_v17::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v17::getPredefinedStyle(
            style, clang_v17::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v16::getPredefinedStyle(
            style, clang_v16::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "16";
  std::string_view next_version = "17";

  ASSIGN_SAME_FIELD(InheritsParentConfig);
  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_MAGIC_ENUM(AlignArrayOfStructures);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.PadOperators);
  NEW_FIELD(AlignConsecutiveShortCaseStatements.Enabled);
  NEW_FIELD(AlignConsecutiveShortCaseStatements.AcrossEmptyLines);
  NEW_FIELD(AlignConsecutiveShortCaseStatements.AcrossComments);
  NEW_FIELD(AlignConsecutiveShortCaseStatements.AlignCaseColons);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_MAGIC_ENUM(AlignOperands);
  ASSIGN_MAGIC_ENUM(AlignTrailingComments.Kind);
  ASSIGN_SAME_FIELD(AlignTrailingComments.OverEmptyLines);
  ASSIGN_SAME_FIELD(AllowAllArgumentsOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_MAGIC_ENUM(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortEnumsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortIfStatementsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortLambdasOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_MAGIC_ENUM(AlwaysBreakTemplateDeclarations);
  ASSIGN_SAME_FIELD(AttributeMacros);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BitFieldColonSpacing);
  NEW_FIELD(BracedInitializerIndentWidth);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterCaseLabel);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterClass);
  ASSIGN_MAGIC_ENUM(BraceWrapping.AfterControlStatement);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterEnum);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterNamespace);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterObjCDeclaration);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterStruct);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterUnion);
  ASSIGN_SAME_FIELD(BraceWrapping.AfterExternBlock);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeCatch);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeElse);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeLambdaBody);
  ASSIGN_SAME_FIELD(BraceWrapping.BeforeWhile);
  ASSIGN_SAME_FIELD(BraceWrapping.IndentBraces);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyFunction);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyRecord);
  ASSIGN_SAME_FIELD(BraceWrapping.SplitEmptyNamespace);
  ASSIGN_MAGIC_ENUM(BreakAfterAttributes);
  ASSIGN_SAME_FIELD(BreakArrays);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_MAGIC_ENUM(BreakBeforeConceptDeclarations);
  ASSIGN_MAGIC_ENUM(BreakBeforeInlineASMColon);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  ASSIGN_SAME_FIELD(CompactNamespaces);
  ASSIGN_SAME_FIELD(ConstructorInitializerIndentWidth);
  ASSIGN_SAME_FIELD(ContinuationIndentWidth);
  ASSIGN_SAME_FIELD(Cpp11BracedListStyle);
  ASSIGN_SAME_FIELD(DerivePointerAlignment);
  ASSIGN_SAME_FIELD(DisableFormat);
  ASSIGN_MAGIC_ENUM(EmptyLineAfterAccessModifier);
  ASSIGN_MAGIC_ENUM(EmptyLineBeforeAccessModifier);
  ASSIGN_SAME_FIELD(ExperimentalAutoDetectBinPacking);
  ASSIGN_SAME_FIELD(FixNamespaceComments);
  ASSIGN_SAME_FIELD(ForEachMacros);
  ASSIGN_MAGIC_ENUM(IncludeStyle.IncludeBlocks);
  ASSIGN_INCLUDE_CATEGORY3(IncludeStyle.IncludeCategories);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainRegex);
  ASSIGN_SAME_FIELD(IncludeStyle.IncludeIsMainSourceRegex);
  ASSIGN_SAME_FIELD(IfMacros);
  ASSIGN_SAME_FIELD(IndentAccessModifiers);
  ASSIGN_SAME_FIELD(IndentCaseBlocks);
  ASSIGN_SAME_FIELD(IndentCaseLabels);
  ASSIGN_SAME_FIELD(IndentGotoLabels);
  ASSIGN_MAGIC_ENUM(IndentExternBlock);
  ASSIGN_MAGIC_ENUM(IndentPPDirectives);
  ASSIGN_SAME_FIELD(IndentRequiresClause);
  ASSIGN_SAME_FIELD(IndentWidth);
  ASSIGN_SAME_FIELD(IndentWrappedFunctionNames);
  ASSIGN_SAME_FIELD(InsertBraces);
  ASSIGN_SAME_FIELD(InsertNewlineAtEOF);
  ASSIGN_MAGIC_ENUM(InsertTrailingCommas);
  ASSIGN_SAME_FIELD(IntegerLiteralSeparator.Binary);
  ASSIGN_SAME_FIELD(IntegerLiteralSeparator.BinaryMinDigits);
  ASSIGN_SAME_FIELD(IntegerLiteralSeparator.Decimal);
  ASSIGN_SAME_FIELD(IntegerLiteralSeparator.DecimalMinDigits);
  ASSIGN_SAME_FIELD(IntegerLiteralSeparator.Hex);
  ASSIGN_SAME_FIELD(IntegerLiteralSeparator.HexMinDigits);
  ASSIGN_SAME_FIELD(JavaImportGroups);
  ASSIGN_MAGIC_ENUM(JavaScriptQuotes);
  ASSIGN_SAME_FIELD(JavaScriptWrapImports);
  NEW_FIELD(KeepEmptyLinesAtEOF);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(LambdaBodyIndentation);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_MAGIC_ENUM(LineEnding);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  NEW_FIELD(Macros);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(NamespaceMacros);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCBreakBeforeNestedBlockParam);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_MAGIC_ENUM(PackConstructorInitializers);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakOpenParenthesis);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyIndentedWhitespace);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(PPIndentWidth);
  ASSIGN_MAGIC_ENUM(QualifierAlignment);
  ASSIGN_SAME_FIELD(QualifierOrder);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 16, 17);
  ASSIGN_MAGIC_ENUM(ReferenceAlignment);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(RemoveBracesLLVM);
  NEW_FIELD(RemoveParentheses);
  ASSIGN_SAME_FIELD(RemoveSemicolon);
  ASSIGN_MAGIC_ENUM(RequiresClausePosition);
  ASSIGN_MAGIC_ENUM(RequiresExpressionIndentation);
  ASSIGN_MAGIC_ENUM(SeparateDefinitionBlocks);
  ASSIGN_SAME_FIELD(ShortNamespaceLines);
  ASSIGN_MAGIC_ENUM(SortIncludes);
  ASSIGN_MAGIC_ENUM(SortJavaStaticImport);
  ASSIGN_MAGIC_ENUM(SortUsingDeclarations);
  ASSIGN_SAME_FIELD(SpaceAfterCStyleCast);
  ASSIGN_SAME_FIELD(SpaceAfterLogicalNot);
  ASSIGN_SAME_FIELD(SpaceAfterTemplateKeyword);
  ASSIGN_MAGIC_ENUM(SpaceAroundPointerQualifiers);
  ASSIGN_SAME_FIELD(SpaceBeforeAssignmentOperators);
  ASSIGN_SAME_FIELD(SpaceBeforeCaseColon);
  ASSIGN_SAME_FIELD(SpaceBeforeCpp11BracedList);
  ASSIGN_SAME_FIELD(SpaceBeforeCtorInitializerColon);
  ASSIGN_SAME_FIELD(SpaceBeforeInheritanceColon);
  NEW_FIELD(SpaceBeforeJsonColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterControlStatements);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterForeachMacros);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterFunctionDeclarationName);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterFunctionDefinitionName);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterIfMacros);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterOverloadedOperator);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterRequiresInClause);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterRequiresInExpression);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.BeforeNonEmptyParentheses);
  ASSIGN_SAME_FIELD(SpaceBeforeSquareBrackets);
  ASSIGN_SAME_FIELD(SpaceBeforeRangeBasedForLoopColon);
  ASSIGN_SAME_FIELD(SpaceInEmptyBlock);
  ASSIGN_SAME_FIELD(SpacesBeforeTrailingComments);
  ASSIGN_MAGIC_ENUM(SpacesInAngles);
  ASSIGN_SAME_FIELD(SpacesInContainerLiterals);
  ASSIGN_SAME_FIELD(SpacesInLineCommentPrefix.Minimum);
  ASSIGN_SAME_FIELD(SpacesInLineCommentPrefix.Maximum);
  RENAME_AND_SWITCH_TO_ENUM(SpacesInParentheses, SpacesInParens,
                            spaces_in_parens_style);
  RENAME_FIELD(SpaceInEmptyParentheses,
               SpacesInParensOptions.InEmptyParentheses);
  RENAME_FIELD(SpacesInConditionalStatement,
               SpacesInParensOptions.InConditionalStatements);
  RENAME_FIELD(SpacesInCStyleCastParentheses,
               SpacesInParensOptions.InCStyleCasts);
  NEW_FIELD(SpacesInParensOptions.Other);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(StatementAttributeLikeMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  ASSIGN_SAME_FIELD(TabWidth);
  NEW_FIELD(TypeNames);
  ASSIGN_SAME_FIELD(TypenameMacros);
  ASSIGN_MAGIC_ENUM(UseTab);
  NEW_FIELD(VerilogBreakBetweenInstancePorts);
  ASSIGN_SAME_FIELD(WhitespaceSensitiveMacros);
}

template void update<clang_vx::Update::UPGRADE>(clang_v16::FormatStyle &prev,
                                                clang_v17::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v16::FormatStyle &prev,
                                                  clang_v17::FormatStyle &next,
                                                  const std::string &style);

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
        item.Regex, item.Priority, item.SortPriority,
        item.RegexIsCaseSensitive});
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
        item.Regex, item.Priority, item.SortPriority,
        item.RegexIsCaseSensitive});
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
