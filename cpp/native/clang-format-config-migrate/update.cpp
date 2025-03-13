#include "update.h"
#include <frozen/unordered_map.h>
#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <variant>

#define XSTR(S) STR(S)
#define STR(S) #S

template <typename... Func> struct overload : Func... {
  using Func::operator()...;
};

template <typename... Func> overload(Func...) -> overload<Func...>;

using AllFormatStyle = std::variant<
    clang_v3_3::FormatStyle, clang_v3_4::FormatStyle, clang_v3_5::FormatStyle,
    clang_v3_6::FormatStyle, clang_v3_7::FormatStyle, clang_v3_8::FormatStyle,
    clang_v3_9::FormatStyle, clang_v4::FormatStyle, clang_v5::FormatStyle,
    clang_v6::FormatStyle, clang_v7::FormatStyle, clang_v8::FormatStyle,
    clang_v9::FormatStyle, clang_v10::FormatStyle, clang_v11::FormatStyle,
    clang_v12::FormatStyle, clang_v13::FormatStyle, clang_v14::FormatStyle,
    clang_v15::FormatStyle, clang_v16::FormatStyle, clang_v17::FormatStyle,
    clang_v18::FormatStyle, clang_v19::FormatStyle>;

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

bool containsIgnoreCase(const std::vector<std::string> &vec,
                        const std::string &str) {
  return std::any_of(vec.begin(), vec.end(), [&str](const std::string &s) {
    return std::equal(
        s.begin(), s.end(), str.begin(), str.end(),
        [](char a, char b) { return std::tolower(a) == std::tolower(b); });
  });
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
  next.Language = clang_v3_5::FormatStyle::LanguageKind::LK_Cpp;
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

constexpr frozen::unordered_map<bool,
                                clang_v17::FormatStyle::SpacesInParensStyle, 2>
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

template <clang_vx::Update Upgrade>
void update(clang_v17::FormatStyle &prev, clang_v18::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v18::getPredefinedStyle(
            style, clang_v18::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v17::getPredefinedStyle(
            style, clang_v17::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "17";
  std::string_view next_version = "18";

  ASSIGN_SAME_FIELD(InheritsParentConfig);
  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_MAGIC_ENUM(AlignArrayOfStructures);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AlignCompound);
  NEW_FIELD(AlignConsecutiveMacros.AlignFunctionPointers);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AlignCompound);
  NEW_FIELD(AlignConsecutiveAssignments.AlignFunctionPointers);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AlignCompound);
  NEW_FIELD(AlignConsecutiveBitFields.AlignFunctionPointers);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AlignCompound);
  NEW_FIELD(AlignConsecutiveDeclarations.AlignFunctionPointers);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveShortCaseStatements.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveShortCaseStatements.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveShortCaseStatements.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveShortCaseStatements.AlignCaseColons);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_MAGIC_ENUM(AlignOperands);
  ASSIGN_MAGIC_ENUM(AlignTrailingComments.Kind);
  ASSIGN_SAME_FIELD(AlignTrailingComments.OverEmptyLines);
  ASSIGN_SAME_FIELD(AllowAllArgumentsOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  NEW_FIELD(AllowBreakBeforeNoexceptSpecifier);
  ASSIGN_MAGIC_ENUM(AllowShortBlocksOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  NEW_FIELD(AllowShortCompoundRequirementOnASingleLine);
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
  ASSIGN_SAME_FIELD(BracedInitializerIndentWidth);
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
  NEW_FIELD(BreakAdjacentStringLiterals);
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
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtEOF);
  ASSIGN_SAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks);
  ASSIGN_MAGIC_ENUM(LambdaBodyIndentation);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_MAGIC_ENUM(LineEnding);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(Macros);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(NamespaceMacros);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCBreakBeforeNestedBlockParam);
  NEW_FIELD(ObjCPropertyAttributeOrder);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_MAGIC_ENUM(PackConstructorInitializers);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakOpenParenthesis);
  NEW_FIELD(PenaltyBreakScopeResolution);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyIndentedWhitespace);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(PPIndentWidth);
  ASSIGN_MAGIC_ENUM(QualifierAlignment);
  ASSIGN_SAME_FIELD(QualifierOrder);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 17, 18);
  ASSIGN_MAGIC_ENUM(ReferenceAlignment);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(RemoveBracesLLVM);
  ASSIGN_MAGIC_ENUM(RemoveParentheses);
  ASSIGN_SAME_FIELD(RemoveSemicolon);
  ASSIGN_MAGIC_ENUM(RequiresClausePosition);
  ASSIGN_MAGIC_ENUM(RequiresExpressionIndentation);
  ASSIGN_MAGIC_ENUM(SeparateDefinitionBlocks);
  ASSIGN_SAME_FIELD(ShortNamespaceLines);
  NEW_FIELD(SkipMacroDefinitionBody);
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
  ASSIGN_SAME_FIELD(SpaceBeforeJsonColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterControlStatements);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterForeachMacros);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterFunctionDeclarationName);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterFunctionDefinitionName);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterIfMacros);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterOverloadedOperator);
  NEW_FIELD(SpaceBeforeParensOptions.AfterPlacementOperator);
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
  ASSIGN_MAGIC_ENUM(SpacesInParens);
  ASSIGN_SAME_FIELD(SpacesInParensOptions.InConditionalStatements);
  ASSIGN_SAME_FIELD(SpacesInParensOptions.InCStyleCasts);
  ASSIGN_SAME_FIELD(SpacesInParensOptions.InEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesInParensOptions.Other);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(StatementAttributeLikeMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_SAME_FIELD(TypeNames);
  ASSIGN_SAME_FIELD(TypenameMacros);
  ASSIGN_MAGIC_ENUM(UseTab);
  ASSIGN_SAME_FIELD(VerilogBreakBetweenInstancePorts);
  ASSIGN_SAME_FIELD(WhitespaceSensitiveMacros);
}

template void update<clang_vx::Update::UPGRADE>(clang_v17::FormatStyle &prev,
                                                clang_v18::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v17::FormatStyle &prev,
                                                  clang_v18::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v18

namespace clang_update_v19 {

template <clang_vx::Update Upgrade>
void update(clang_v18::FormatStyle &prev, clang_v19::FormatStyle &next,
            const std::string &style) {
  if constexpr (Upgrade == clang_vx::Update::UPGRADE) {
    if (!clang_v19::getPredefinedStyle(
            style, clang_v19::FormatStyle::LanguageKind::LK_Cpp, &next)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  } else {
    if (!clang_v18::getPredefinedStyle(
            style, clang_v18::FormatStyle::LanguageKind::LK_Cpp, &prev)) {
      throw std::runtime_error("Failed to load " + style + " style.");
    }
  }

  std::string_view prev_version = "18";
  std::string_view next_version = "19";

  ASSIGN_SAME_FIELD(InheritsParentConfig);
  ASSIGN_SAME_FIELD(AccessModifierOffset);
  ASSIGN_MAGIC_ENUM(AlignAfterOpenBracket);
  ASSIGN_MAGIC_ENUM(AlignArrayOfStructures);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.AlignFunctionPointers);
  ASSIGN_SAME_FIELD(AlignConsecutiveMacros.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.AlignFunctionPointers);
  ASSIGN_SAME_FIELD(AlignConsecutiveAssignments.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.AlignFunctionPointers);
  ASSIGN_SAME_FIELD(AlignConsecutiveBitFields.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AcrossComments);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AlignCompound);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.AlignFunctionPointers);
  ASSIGN_SAME_FIELD(AlignConsecutiveDeclarations.PadOperators);
  ASSIGN_SAME_FIELD(AlignConsecutiveShortCaseStatements.Enabled);
  ASSIGN_SAME_FIELD(AlignConsecutiveShortCaseStatements.AcrossEmptyLines);
  ASSIGN_SAME_FIELD(AlignConsecutiveShortCaseStatements.AcrossComments);
  NEW_FIELD(AlignConsecutiveShortCaseStatements.AlignCaseArrows);
  ASSIGN_SAME_FIELD(AlignConsecutiveShortCaseStatements.AlignCaseColons);
  NEW_FIELD(AlignConsecutiveTableGenBreakingDAGArgColons.Enabled);
  NEW_FIELD(AlignConsecutiveTableGenBreakingDAGArgColons.AcrossEmptyLines);
  NEW_FIELD(AlignConsecutiveTableGenBreakingDAGArgColons.AcrossComments);
  NEW_FIELD(AlignConsecutiveTableGenBreakingDAGArgColons.AlignCompound);
  NEW_FIELD(AlignConsecutiveTableGenBreakingDAGArgColons.AlignFunctionPointers);
  NEW_FIELD(AlignConsecutiveTableGenBreakingDAGArgColons.PadOperators);
  NEW_FIELD(AlignConsecutiveTableGenCondOperatorColons.Enabled);
  NEW_FIELD(AlignConsecutiveTableGenCondOperatorColons.AcrossEmptyLines);
  NEW_FIELD(AlignConsecutiveTableGenCondOperatorColons.AcrossComments);
  NEW_FIELD(AlignConsecutiveTableGenCondOperatorColons.AlignCompound);
  NEW_FIELD(AlignConsecutiveTableGenCondOperatorColons.AlignFunctionPointers);
  NEW_FIELD(AlignConsecutiveTableGenCondOperatorColons.PadOperators);
  NEW_FIELD(AlignConsecutiveTableGenDefinitionColons.Enabled);
  NEW_FIELD(AlignConsecutiveTableGenDefinitionColons.AcrossEmptyLines);
  NEW_FIELD(AlignConsecutiveTableGenDefinitionColons.AcrossComments);
  NEW_FIELD(AlignConsecutiveTableGenDefinitionColons.AlignCompound);
  NEW_FIELD(AlignConsecutiveTableGenDefinitionColons.AlignFunctionPointers);
  NEW_FIELD(AlignConsecutiveTableGenDefinitionColons.PadOperators);
  ASSIGN_MAGIC_ENUM(AlignEscapedNewlines);
  ASSIGN_MAGIC_ENUM(AlignOperands);
  ASSIGN_MAGIC_ENUM(AlignTrailingComments.Kind);
  ASSIGN_SAME_FIELD(AlignTrailingComments.OverEmptyLines);
  ASSIGN_SAME_FIELD(AllowAllArgumentsOnNextLine);
  ASSIGN_SAME_FIELD(AllowAllParametersOfDeclarationOnNextLine);
  ASSIGN_MAGIC_ENUM(AllowBreakBeforeNoexceptSpecifier);
  ASSIGN_MAGIC_ENUM(AllowShortBlocksOnASingleLine);
  NEW_FIELD(AllowShortCaseExpressionOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCaseLabelsOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortCompoundRequirementOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortEnumsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortFunctionsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortIfStatementsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AllowShortLambdasOnASingleLine);
  ASSIGN_SAME_FIELD(AllowShortLoopsOnASingleLine);
  ASSIGN_MAGIC_ENUM(AlwaysBreakAfterDefinitionReturnType);
  ASSIGN_SAME_FIELD(AlwaysBreakBeforeMultilineStrings);
  ASSIGN_SAME_FIELD(AttributeMacros);
  ASSIGN_SAME_FIELD(BinPackArguments);
  ASSIGN_SAME_FIELD(BinPackParameters);
  ASSIGN_MAGIC_ENUM(BitFieldColonSpacing);
  ASSIGN_SAME_FIELD(BracedInitializerIndentWidth);
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
  ASSIGN_SAME_FIELD(BreakAdjacentStringLiterals);
  ASSIGN_MAGIC_ENUM(BreakAfterAttributes);
  RENAME_MAGIC_ENUM(AlwaysBreakAfterReturnType, BreakAfterReturnType);
  ASSIGN_SAME_FIELD(BreakArrays);
  ASSIGN_MAGIC_ENUM(BreakBeforeBinaryOperators);
  ASSIGN_MAGIC_ENUM(BreakBeforeBraces);
  ASSIGN_MAGIC_ENUM(BreakBeforeConceptDeclarations);
  ASSIGN_MAGIC_ENUM(BreakBeforeInlineASMColon);
  ASSIGN_SAME_FIELD(BreakBeforeTernaryOperators);
  ASSIGN_MAGIC_ENUM(BreakConstructorInitializers);
  NEW_FIELD(BreakFunctionDefinitionParameters);
  ASSIGN_SAME_FIELD(BreakAfterJavaFieldAnnotations);
  ASSIGN_SAME_FIELD(BreakStringLiterals);
  ASSIGN_SAME_FIELD(ColumnLimit);
  ASSIGN_SAME_FIELD(CommentPragmas);
  ASSIGN_MAGIC_ENUM(BreakInheritanceList);
  RENAME_MAGIC_ENUM(AlwaysBreakTemplateDeclarations, BreakTemplateDeclarations);
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
  NEW_FIELD(IncludeStyle.MainIncludeChar);
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
  RENAME_FIELD(KeepEmptyLinesAtEOF, KeepEmptyLines.AtEndOfFile);
  RENAME_FIELD(KeepEmptyLinesAtTheStartOfBlocks, KeepEmptyLines.AtStartOfBlock);
  NEW_FIELD(KeepEmptyLines.AtStartOfFile);
  ASSIGN_MAGIC_ENUM(LambdaBodyIndentation);
  ASSIGN_MAGIC_ENUM(Language);
  ASSIGN_MAGIC_ENUM(LineEnding);
  ASSIGN_SAME_FIELD(MacroBlockBegin);
  ASSIGN_SAME_FIELD(MacroBlockEnd);
  ASSIGN_SAME_FIELD(Macros);
  ASSIGN_SAME_FIELD(MaxEmptyLinesToKeep);
  ASSIGN_MAGIC_ENUM(NamespaceIndentation);
  ASSIGN_SAME_FIELD(NamespaceMacros);
  ASSIGN_MAGIC_ENUM(ObjCBinPackProtocolList);
  ASSIGN_SAME_FIELD(ObjCBlockIndentWidth);
  ASSIGN_SAME_FIELD(ObjCBreakBeforeNestedBlockParam);
  ASSIGN_SAME_FIELD(ObjCPropertyAttributeOrder);
  ASSIGN_SAME_FIELD(ObjCSpaceAfterProperty);
  ASSIGN_SAME_FIELD(ObjCSpaceBeforeProtocolList);
  ASSIGN_MAGIC_ENUM(PackConstructorInitializers);
  ASSIGN_SAME_FIELD(PenaltyBreakAssignment);
  ASSIGN_SAME_FIELD(PenaltyBreakBeforeFirstCallParameter);
  ASSIGN_SAME_FIELD(PenaltyBreakComment);
  ASSIGN_SAME_FIELD(PenaltyBreakFirstLessLess);
  ASSIGN_SAME_FIELD(PenaltyBreakOpenParenthesis);
  ASSIGN_SAME_FIELD(PenaltyBreakScopeResolution);
  ASSIGN_SAME_FIELD(PenaltyBreakString);
  ASSIGN_SAME_FIELD(PenaltyBreakTemplateDeclaration);
  ASSIGN_SAME_FIELD(PenaltyExcessCharacter);
  ASSIGN_SAME_FIELD(PenaltyIndentedWhitespace);
  ASSIGN_SAME_FIELD(PenaltyReturnTypeOnItsOwnLine);
  ASSIGN_MAGIC_ENUM(PointerAlignment);
  ASSIGN_SAME_FIELD(PPIndentWidth);
  ASSIGN_MAGIC_ENUM(QualifierAlignment);
  ASSIGN_SAME_FIELD(QualifierOrder);
  ASSIGN_RAW_STRING_FORMAT(RawStringFormats, 18, 19);
  ASSIGN_MAGIC_ENUM(ReferenceAlignment);
  ASSIGN_SAME_FIELD(ReflowComments);
  ASSIGN_SAME_FIELD(RemoveBracesLLVM);
  ASSIGN_MAGIC_ENUM(RemoveParentheses);
  ASSIGN_SAME_FIELD(RemoveSemicolon);
  ASSIGN_MAGIC_ENUM(RequiresClausePosition);
  ASSIGN_MAGIC_ENUM(RequiresExpressionIndentation);
  ASSIGN_MAGIC_ENUM(SeparateDefinitionBlocks);
  ASSIGN_SAME_FIELD(ShortNamespaceLines);
  ASSIGN_SAME_FIELD(SkipMacroDefinitionBody);
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
  ASSIGN_SAME_FIELD(SpaceBeforeJsonColon);
  ASSIGN_MAGIC_ENUM(SpaceBeforeParens);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterControlStatements);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterForeachMacros);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterFunctionDeclarationName);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterFunctionDefinitionName);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterIfMacros);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterOverloadedOperator);
  ASSIGN_SAME_FIELD(SpaceBeforeParensOptions.AfterPlacementOperator);
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
  ASSIGN_MAGIC_ENUM(SpacesInParens);
  NEW_FIELD(SpacesInParensOptions.ExceptDoubleParentheses);
  ASSIGN_SAME_FIELD(SpacesInParensOptions.InConditionalStatements);
  ASSIGN_SAME_FIELD(SpacesInParensOptions.InCStyleCasts);
  ASSIGN_SAME_FIELD(SpacesInParensOptions.InEmptyParentheses);
  ASSIGN_SAME_FIELD(SpacesInParensOptions.Other);
  ASSIGN_SAME_FIELD(SpacesInSquareBrackets);
  ASSIGN_MAGIC_ENUM(Standard);
  ASSIGN_SAME_FIELD(StatementAttributeLikeMacros);
  ASSIGN_SAME_FIELD(StatementMacros);
  NEW_FIELD(TableGenBreakingDAGArgOperators);
  NEW_FIELD(TableGenBreakInsideDAGArg);
  ASSIGN_SAME_FIELD(TabWidth);
  ASSIGN_SAME_FIELD(TypeNames);
  ASSIGN_SAME_FIELD(TypenameMacros);
  ASSIGN_MAGIC_ENUM(UseTab);
  ASSIGN_SAME_FIELD(VerilogBreakBetweenInstancePorts);
  ASSIGN_SAME_FIELD(WhitespaceSensitiveMacros);
}

template void update<clang_vx::Update::UPGRADE>(clang_v18::FormatStyle &prev,
                                                clang_v19::FormatStyle &next,
                                                const std::string &style);
template void update<clang_vx::Update::DOWNGRADE>(clang_v18::FormatStyle &prev,
                                                  clang_v19::FormatStyle &next,
                                                  const std::string &style);

} // namespace clang_update_v19

namespace {

AllFormatStyle versionToFormatStyle(clang_vx::Version version,
                                    const std::string &data,
                                    const std::string &default_style) {
  switch (version) {
  case clang_vx::Version::V3_3: {
    throw std::runtime_error("V3.3 doesn't support serialization.");
  }
  case clang_vx::Version::V3_4: {
    clang_v3_4::FormatStyle fs3_4;
    if (!clang_v3_4::getPredefinedStyle(default_style, &fs3_4)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v3_4::parseConfiguration(data, &fs3_4);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v3_4.\n" +
                               ec.message());
    }
    return fs3_4;
  }
  case clang_vx::Version::V3_5: {
    clang_v3_5::FormatStyle fs3_5;
    fs3_5.Language = clang_v3_5::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v3_5::getPredefinedStyle(
            default_style, clang_v3_5::FormatStyle::LanguageKind::LK_Cpp,
            &fs3_5)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v3_5::parseConfiguration(data, &fs3_5);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v3_5.\n" +
                               ec.message());
    }
    return fs3_5;
  }
  case clang_vx::Version::V3_6: {
    clang_v3_6::FormatStyle fs3_6;
    fs3_6.Language = clang_v3_6::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v3_6::getPredefinedStyle(
            default_style, clang_v3_6::FormatStyle::LanguageKind::LK_Cpp,
            &fs3_6)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v3_6::parseConfiguration(data, &fs3_6);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v3_6.\n" +
                               ec.message());
    }
    return fs3_6;
  }
  case clang_vx::Version::V3_7: {
    clang_v3_7::FormatStyle fs3_7;
    fs3_7.Language = clang_v3_7::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v3_7::getPredefinedStyle(
            default_style, clang_v3_7::FormatStyle::LanguageKind::LK_Cpp,
            &fs3_7)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v3_7::parseConfiguration(data, &fs3_7);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v3_7.\n" +
                               ec.message());
    }
    return fs3_7;
  }
  case clang_vx::Version::V3_8: {
    clang_v3_8::FormatStyle fs3_8;
    fs3_8.Language = clang_v3_8::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v3_8::getPredefinedStyle(
            default_style, clang_v3_8::FormatStyle::LanguageKind::LK_Cpp,
            &fs3_8)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v3_8::parseConfiguration(data, &fs3_8);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v3_8.\n" +
                               ec.message());
    }
    return fs3_8;
  }
  case clang_vx::Version::V3_9: {
    clang_v3_9::FormatStyle fs3_9;
    fs3_9.Language = clang_v3_9::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v3_9::getPredefinedStyle(
            default_style, clang_v3_9::FormatStyle::LanguageKind::LK_Cpp,
            &fs3_9)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v3_9::parseConfiguration(data, &fs3_9);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v3_9.\n" +
                               ec.message());
    }
    return fs3_9;
  }
  case clang_vx::Version::V4: {
    clang_v4::FormatStyle fs4;
    fs4.Language = clang_v4::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v4::getPredefinedStyle(
            default_style, clang_v4::FormatStyle::LanguageKind::LK_Cpp, &fs4)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v4::parseConfiguration(data, &fs4);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v4.\n" +
                               ec.message());
    }
    return fs4;
  }
  case clang_vx::Version::V5: {
    clang_v5::FormatStyle fs5;
    if (!clang_v5::getPredefinedStyle(
            default_style, clang_v5::FormatStyle::LanguageKind::LK_Cpp, &fs5)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    fs5.Language = clang_v5::FormatStyle::LanguageKind::LK_Cpp;
    std::error_code ec = clang_v5::parseConfiguration(data, &fs5);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v5.\n" +
                               ec.message());
    }
    return fs5;
  }
  case clang_vx::Version::V6: {
    clang_v6::FormatStyle fs6;
    fs6.Language = clang_v6::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v6::getPredefinedStyle(
            default_style, clang_v6::FormatStyle::LanguageKind::LK_Cpp, &fs6)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v6::parseConfiguration(data, &fs6);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v6.\n" +
                               ec.message());
    }
    return fs6;
  }
  case clang_vx::Version::V7: {
    clang_v7::FormatStyle fs7;
    fs7.Language = clang_v7::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v7::getPredefinedStyle(
            default_style, clang_v7::FormatStyle::LanguageKind::LK_Cpp, &fs7)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v7::parseConfiguration(data, &fs7);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v7.\n" +
                               ec.message());
    }
    return fs7;
  }
  case clang_vx::Version::V8: {
    clang_v8::FormatStyle fs8;
    fs8.Language = clang_v8::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v8::getPredefinedStyle(
            default_style, clang_v8::FormatStyle::LanguageKind::LK_Cpp, &fs8)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v8::parseConfiguration(data, &fs8);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v8.\n" +
                               ec.message());
    }
    return fs8;
  }
  case clang_vx::Version::V9: {
    clang_v9::FormatStyle fs9;
    fs9.Language = clang_v9::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v9::getPredefinedStyle(
            default_style, clang_v9::FormatStyle::LanguageKind::LK_Cpp, &fs9)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v9::parseConfiguration(data, &fs9);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v9.\n" +
                               ec.message());
    }
    return fs9;
  }
  case clang_vx::Version::V10: {
    clang_v10::FormatStyle fs10;
    fs10.Language = clang_v10::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v10::getPredefinedStyle(
            default_style, clang_v10::FormatStyle::LanguageKind::LK_Cpp,
            &fs10)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v10::parseConfiguration(data, &fs10);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v10.\n" +
                               ec.message());
    }
    return fs10;
  }
  case clang_vx::Version::V11: {
    clang_v11::FormatStyle fs11;
    fs11.Language = clang_v11::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v11::getPredefinedStyle(
            default_style, clang_v11::FormatStyle::LanguageKind::LK_Cpp,
            &fs11)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v11::parseConfiguration(data, &fs11);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v11.\n" +
                               ec.message());
    }
    return fs11;
  }
  case clang_vx::Version::V12: {
    clang_v12::FormatStyle fs12;
    fs12.Language = clang_v12::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v12::getPredefinedStyle(
            default_style, clang_v12::FormatStyle::LanguageKind::LK_Cpp,
            &fs12)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v12::parseConfiguration(data, &fs12);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v12.\n" +
                               ec.message());
    }
    return fs12;
  }
  case clang_vx::Version::V13: {
    clang_v13::FormatStyle fs13;
    fs13.Language = clang_v13::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v13::getPredefinedStyle(
            default_style, clang_v13::FormatStyle::LanguageKind::LK_Cpp,
            &fs13)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v13::parseConfiguration(data, &fs13);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v13.\n" +
                               ec.message());
    }
    return fs13;
  }
  case clang_vx::Version::V14: {
    clang_v14::FormatStyle fs14;
    fs14.Language = clang_v14::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v14::getPredefinedStyle(
            default_style, clang_v14::FormatStyle::LanguageKind::LK_Cpp,
            &fs14)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v14::parseConfiguration(data, &fs14);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v14.\n" +
                               ec.message());
    }
    return fs14;
  }
  case clang_vx::Version::V15: {
    clang_v15::FormatStyle fs15;
    fs15.Language = clang_v15::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v15::getPredefinedStyle(
            default_style, clang_v15::FormatStyle::LanguageKind::LK_Cpp,
            &fs15)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v15::parseConfiguration(data, &fs15);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v15.\n" +
                               ec.message());
    }
    return fs15;
  }
  case clang_vx::Version::V16: {
    clang_v16::FormatStyle fs16;
    fs16.Language = clang_v16::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v16::getPredefinedStyle(
            default_style, clang_v16::FormatStyle::LanguageKind::LK_Cpp,
            &fs16)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v16::parseConfiguration(data, &fs16);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v16.\n" +
                               ec.message());
    }
    return fs16;
  }
  case clang_vx::Version::V17: {
    clang_v17::FormatStyle fs17;
    fs17.Language = clang_v17::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v17::getPredefinedStyle(
            default_style, clang_v17::FormatStyle::LanguageKind::LK_Cpp,
            &fs17)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v17::parseConfiguration(data, &fs17);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v17.\n" +
                               ec.message());
    }
    return fs17;
  }
  case clang_vx::Version::V18: {
    clang_v18::FormatStyle fs18;
    fs18.Language = clang_v18::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v18::getPredefinedStyle(
            default_style, clang_v18::FormatStyle::LanguageKind::LK_Cpp,
            &fs18)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v18::parseConfiguration(data, &fs18);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v18.\n" +
                               ec.message());
    }
    return fs18;
  }
  case clang_vx::Version::V19: {
    clang_v19::FormatStyle fs19;
    fs19.Language = clang_v19::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_v19::getPredefinedStyle(
            default_style, clang_v19::FormatStyle::LanguageKind::LK_Cpp,
            &fs19)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_v19::parseConfiguration(data, &fs19);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file v19.\n" +
                               ec.message());
    }
    return fs19;
  }
  default: {
    throw std::runtime_error("Unknown version.");
  }
  }
}

std::string formatStyleToVersion(const AllFormatStyle &data,
                                 const std::string &default_style,
                                 bool skip_same_value) {
  return std::visit(
      overload{
          [](const clang_v3_3::FormatStyle &fs) {
            throw std::runtime_error("V3.3 doesn't support serialization.");
            return std::string{};
          },
          [&default_style, skip_same_value](const clang_v3_4::FormatStyle &fs) {
            return clang_v3_4::configurationAsText(fs, default_style,
                                                   skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v3_5::FormatStyle &fs) {
            return clang_v3_5::configurationAsText(fs, default_style,
                                                   skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v3_6::FormatStyle &fs) {
            return clang_v3_6::configurationAsText(fs, default_style,
                                                   skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v3_7::FormatStyle &fs) {
            return clang_v3_7::configurationAsText(fs, default_style,
                                                   skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v3_8::FormatStyle &fs) {
            return clang_v3_8::configurationAsText(fs, default_style,
                                                   skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v3_9::FormatStyle &fs) {
            return clang_v3_9::configurationAsText(fs, default_style,
                                                   skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v4::FormatStyle &fs) {
            return clang_v4::configurationAsText(fs, default_style,
                                                 skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v5::FormatStyle &fs) {
            return clang_v5::configurationAsText(fs, default_style,
                                                 skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v6::FormatStyle &fs) {
            return clang_v6::configurationAsText(fs, default_style,
                                                 skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v7::FormatStyle &fs) {
            return clang_v7::configurationAsText(fs, default_style,
                                                 skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v8::FormatStyle &fs) {
            return clang_v8::configurationAsText(fs, default_style,
                                                 skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v9::FormatStyle &fs) {
            return clang_v9::configurationAsText(fs, default_style,
                                                 skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v10::FormatStyle &fs) {
            return clang_v10::configurationAsText(fs, default_style,
                                                  skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v11::FormatStyle &fs) {
            return clang_v11::configurationAsText(fs, default_style,
                                                  skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v12::FormatStyle &fs) {
            return clang_v12::configurationAsText(fs, default_style,
                                                  skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v13::FormatStyle &fs) {
            return clang_v13::configurationAsText(fs, default_style,
                                                  skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v14::FormatStyle &fs) {
            return clang_v14::configurationAsText(fs, default_style,
                                                  skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v15::FormatStyle &fs) {
            return clang_v15::configurationAsText(fs, default_style,
                                                  skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v16::FormatStyle &fs) {
            return clang_v16::configurationAsText(fs, default_style,
                                                  skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v17::FormatStyle &fs) {
            return clang_v17::configurationAsText(fs, default_style,
                                                  skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v18::FormatStyle &fs) {
            return clang_v18::configurationAsText(fs, default_style,
                                                  skip_same_value);
          },
          [&default_style, skip_same_value](const clang_v19::FormatStyle &fs) {
            return clang_v19::configurationAsText(fs, default_style,
                                                  skip_same_value);
          }},
      data);
}
} // namespace

namespace clang_vx {
std::string updateTo(Version vstart, Version vend, const std::string &data,
                     const std::string &default_style, bool skip_same_value) {
  AllFormatStyle before = versionToFormatStyle(vstart, data, default_style);
  if (vstart == vend) {
    return formatStyleToVersion(before, default_style, skip_same_value);
  }
  AllFormatStyle after;

  std::string based_on_style;
  size_t pos = data.find("BasedOnStyle:");
  if (pos != std::string::npos) {
    pos += 13;
    while (pos < data.size() && data[pos] == ' ')
      ++pos; // Ignorer les espaces
    size_t end = data.find('\n', pos);
    based_on_style = data.substr(pos, end - pos);
    std::cout << "BasedOnStyle: " << based_on_style << std::endl;
  }

  for (size_t vi = static_cast<size_t>(vstart); vi < static_cast<size_t>(vend);
       vi++) {
    Version v_i = static_cast<Version>(vi);
    Version vnext_i = static_cast<Version>(vi + 1);

    std::vector<std::string> compatible_styles = getStyleNames(vnext_i);

    std::string style_i;

    if (!based_on_style.empty() &&
        containsIgnoreCase(compatible_styles, based_on_style)) {
      style_i = based_on_style;
    } else if (!default_style.empty() &&
               containsIgnoreCase(compatible_styles, default_style)) {
      style_i = default_style;
    } else {
      std::ostringstream ss;
      if (based_on_style.empty()) {
        ss << "No BasedOnStyle found in yaml config.\n";
      } else {
        ss << based_on_style << " is not compatible with version "
           << magic_enum::enum_name(vnext_i) << " (" << compatible_styles
           << ").\n";
      }
      if (default_style.empty()) {
        ss << "No fallback style is set.\n";
      } else {
        ss << default_style << " is not compatible with version "
           << magic_enum::enum_name(vnext_i) << " (" << compatible_styles
           << ").\n";
      }
      throw std::runtime_error(ss.str());
    }

    switch (v_i) {
    case Version::V3_3: {
      after = clang_v3_4::FormatStyle{};
      clang_update_v3_4::update<Update::UPGRADE>(
          std::get<clang_v3_3::FormatStyle>(before),
          std::get<clang_v3_4::FormatStyle>(after), style_i);
      break;
    }
    case Version::V3_4: {
      after = clang_v3_5::FormatStyle{};
      clang_update_v3_5::update<Update::UPGRADE>(
          std::get<clang_v3_4::FormatStyle>(before),
          std::get<clang_v3_5::FormatStyle>(after), style_i);
      break;
    }
    case Version::V3_5: {
      after = clang_v3_6::FormatStyle{};
      clang_update_v3_6::update<Update::UPGRADE>(
          std::get<clang_v3_5::FormatStyle>(before),
          std::get<clang_v3_6::FormatStyle>(after), style_i);
      break;
    }
    case Version::V3_6: {
      after = clang_v3_7::FormatStyle{};
      clang_update_v3_7::update<Update::UPGRADE>(
          std::get<clang_v3_6::FormatStyle>(before),
          std::get<clang_v3_7::FormatStyle>(after), style_i);
      break;
    }
    case Version::V3_7: {
      after = clang_v3_8::FormatStyle{};
      clang_update_v3_8::update<Update::UPGRADE>(
          std::get<clang_v3_7::FormatStyle>(before),
          std::get<clang_v3_8::FormatStyle>(after), style_i);
      break;
    }
    case Version::V3_8: {
      after = clang_v3_9::FormatStyle{};
      clang_update_v3_9::update<Update::UPGRADE>(
          std::get<clang_v3_8::FormatStyle>(before),
          std::get<clang_v3_9::FormatStyle>(after), style_i);
      break;
    }
    case Version::V3_9: {
      after = clang_v4::FormatStyle{};
      clang_update_v4::update<Update::UPGRADE>(
          std::get<clang_v3_9::FormatStyle>(before),
          std::get<clang_v4::FormatStyle>(after), style_i);
      break;
    }
    case Version::V4: {
      after = clang_v5::FormatStyle{};
      clang_update_v5::update<Update::UPGRADE>(
          std::get<clang_v4::FormatStyle>(before),
          std::get<clang_v5::FormatStyle>(after), style_i);
      break;
    }
    case Version::V5: {
      after = clang_v6::FormatStyle{};
      clang_update_v6::update<Update::UPGRADE>(
          std::get<clang_v5::FormatStyle>(before),
          std::get<clang_v6::FormatStyle>(after), style_i);
      break;
    }
    case Version::V6: {
      after = clang_v7::FormatStyle{};
      clang_update_v7::update<Update::UPGRADE>(
          std::get<clang_v6::FormatStyle>(before),
          std::get<clang_v7::FormatStyle>(after), style_i);
      break;
    }
    case Version::V7: {
      after = clang_v8::FormatStyle{};
      clang_update_v8::update<Update::UPGRADE>(
          std::get<clang_v7::FormatStyle>(before),
          std::get<clang_v8::FormatStyle>(after), style_i);
      break;
    }
    case Version::V8: {
      after = clang_v9::FormatStyle{};
      clang_update_v9::update<Update::UPGRADE>(
          std::get<clang_v8::FormatStyle>(before),
          std::get<clang_v9::FormatStyle>(after), style_i);
      break;
    }
    case Version::V9: {
      after = clang_v10::FormatStyle{};
      clang_update_v10::update<Update::UPGRADE>(
          std::get<clang_v9::FormatStyle>(before),
          std::get<clang_v10::FormatStyle>(after), style_i);
      break;
    }
    case Version::V10: {
      after = clang_v11::FormatStyle{};
      clang_update_v11::update<Update::UPGRADE>(
          std::get<clang_v10::FormatStyle>(before),
          std::get<clang_v11::FormatStyle>(after), style_i);
      break;
    }
    case Version::V11: {
      after = clang_v12::FormatStyle{};
      clang_update_v12::update<Update::UPGRADE>(
          std::get<clang_v11::FormatStyle>(before),
          std::get<clang_v12::FormatStyle>(after), style_i);
      break;
    }
    case Version::V12: {
      after = clang_v13::FormatStyle{};
      clang_update_v13::update<Update::UPGRADE>(
          std::get<clang_v12::FormatStyle>(before),
          std::get<clang_v13::FormatStyle>(after), style_i);
      break;
    }
    case Version::V13: {
      after = clang_v14::FormatStyle{};
      clang_update_v14::update<Update::UPGRADE>(
          std::get<clang_v13::FormatStyle>(before),
          std::get<clang_v14::FormatStyle>(after), style_i);
      break;
    }
    case Version::V14: {
      after = clang_v15::FormatStyle{};
      clang_update_v15::update<Update::UPGRADE>(
          std::get<clang_v14::FormatStyle>(before),
          std::get<clang_v15::FormatStyle>(after), style_i);
      break;
    }
    case Version::V15: {
      after = clang_v16::FormatStyle{};
      clang_update_v16::update<Update::UPGRADE>(
          std::get<clang_v15::FormatStyle>(before),
          std::get<clang_v16::FormatStyle>(after), style_i);
      break;
    }
    case Version::V16: {
      after = clang_v17::FormatStyle{};
      clang_update_v17::update<Update::UPGRADE>(
          std::get<clang_v16::FormatStyle>(before),
          std::get<clang_v17::FormatStyle>(after), style_i);
      break;
    }
    case Version::V17: {
      after = clang_v18::FormatStyle{};
      clang_update_v18::update<Update::UPGRADE>(
          std::get<clang_v17::FormatStyle>(before),
          std::get<clang_v18::FormatStyle>(after), style_i);
      break;
    }
    case Version::V18: {
      after = clang_v19::FormatStyle{};
      clang_update_v19::update<Update::UPGRADE>(
          std::get<clang_v18::FormatStyle>(before),
          std::get<clang_v19::FormatStyle>(after), style_i);
      break;
    }
    default: {
      throw std::runtime_error("Unsupported version while upgrading.");
      break;
    }
    }
    std::swap(before, after);
  }

  return formatStyleToVersion(before, default_style, skip_same_value);
}

std::string downgradeTo(Version vstart, Version vend, const std::string &data,
                        const std::string &default_style,
                        bool skip_same_value) {
  AllFormatStyle before = versionToFormatStyle(vstart, data, default_style);
  if (vstart == vend) {
    return formatStyleToVersion(before, default_style, skip_same_value);
  }
  AllFormatStyle after;

  std::string based_on_style;
  size_t pos = data.find("BasedOnStyle:");
  if (pos != std::string::npos) {
    pos += 13;
    while (pos < data.size() && data[pos] == ' ')
      ++pos; // Ignorer les espaces
    size_t end = data.find('\n', pos);
    based_on_style = data.substr(pos, end - pos);
    std::cout << "BasedOnStyle: " << based_on_style << std::endl;
  }

  for (size_t vi = static_cast<size_t>(vstart); vi > static_cast<size_t>(vend);
       vi--) {
    Version v_i = static_cast<Version>(vi);
    Version vnext_i = static_cast<Version>(vi - 1);

    std::vector<std::string> compatible_styles = getStyleNames(vnext_i);

    std::string style_i;

    if (!based_on_style.empty() &&
        containsIgnoreCase(compatible_styles, based_on_style)) {
      style_i = based_on_style;
    } else if (!default_style.empty() &&
               containsIgnoreCase(compatible_styles, default_style)) {
      style_i = default_style;
    } else {
      std::ostringstream ss;
      if (based_on_style.empty()) {
        ss << "No BasedOnStyle found in yaml config.\n";
      } else {
        ss << based_on_style << " is not compatible with version "
           << magic_enum::enum_name(vnext_i) << " (" << compatible_styles
           << ").\n";
      }
      if (default_style.empty()) {
        ss << "No fallback style is set.\n";
      } else {
        ss << default_style << " is not compatible with version "
           << magic_enum::enum_name(vnext_i) << " (" << compatible_styles
           << ").\n";
      }
      throw std::runtime_error(ss.str());
    }

    switch (v_i) {
    case Version::V3_4: {
      after = clang_v3_3::FormatStyle{};
      clang_update_v3_4::update<Update::DOWNGRADE>(
          std::get<clang_v3_3::FormatStyle>(after),
          std::get<clang_v3_4::FormatStyle>(before), style_i);
      break;
    }
    case Version::V3_5: {
      after = clang_v3_4::FormatStyle{};
      clang_update_v3_5::update<Update::DOWNGRADE>(
          std::get<clang_v3_4::FormatStyle>(after),
          std::get<clang_v3_5::FormatStyle>(before), style_i);
      break;
    }
    case Version::V3_6: {
      after = clang_v3_5::FormatStyle{};
      clang_update_v3_6::update<Update::DOWNGRADE>(
          std::get<clang_v3_5::FormatStyle>(after),
          std::get<clang_v3_6::FormatStyle>(before), style_i);
      break;
    }
    case Version::V3_7: {
      after = clang_v3_6::FormatStyle{};
      clang_update_v3_7::update<Update::DOWNGRADE>(
          std::get<clang_v3_6::FormatStyle>(after),
          std::get<clang_v3_7::FormatStyle>(before), style_i);
      break;
    }
    case Version::V3_8: {
      after = clang_v3_7::FormatStyle{};
      clang_update_v3_8::update<Update::DOWNGRADE>(
          std::get<clang_v3_7::FormatStyle>(after),
          std::get<clang_v3_8::FormatStyle>(before), style_i);
      break;
    }
    case Version::V3_9: {
      after = clang_v3_8::FormatStyle{};
      clang_update_v3_9::update<Update::DOWNGRADE>(
          std::get<clang_v3_8::FormatStyle>(after),
          std::get<clang_v3_9::FormatStyle>(before), style_i);
      break;
    }
    case Version::V4: {
      after = clang_v3_9::FormatStyle{};
      clang_update_v4::update<Update::DOWNGRADE>(
          std::get<clang_v3_9::FormatStyle>(after),
          std::get<clang_v4::FormatStyle>(before), style_i);
      break;
    }
    case Version::V5: {
      after = clang_v4::FormatStyle{};
      clang_update_v5::update<Update::DOWNGRADE>(
          std::get<clang_v4::FormatStyle>(after),
          std::get<clang_v5::FormatStyle>(before), style_i);
      break;
    }
    case Version::V6: {
      after = clang_v5::FormatStyle{};
      clang_update_v6::update<Update::DOWNGRADE>(
          std::get<clang_v5::FormatStyle>(after),
          std::get<clang_v6::FormatStyle>(before), style_i);
      break;
    }
    case Version::V7: {
      after = clang_v6::FormatStyle{};
      clang_update_v7::update<Update::DOWNGRADE>(
          std::get<clang_v6::FormatStyle>(after),
          std::get<clang_v7::FormatStyle>(before), style_i);
      break;
    }
    case Version::V8: {
      after = clang_v7::FormatStyle{};
      clang_update_v8::update<Update::DOWNGRADE>(
          std::get<clang_v7::FormatStyle>(after),
          std::get<clang_v8::FormatStyle>(before), style_i);
      break;
    }
    case Version::V9: {
      after = clang_v8::FormatStyle{};
      clang_update_v9::update<Update::DOWNGRADE>(
          std::get<clang_v8::FormatStyle>(after),
          std::get<clang_v9::FormatStyle>(before), style_i);
      break;
    }
    case Version::V10: {
      after = clang_v9::FormatStyle{};
      clang_update_v10::update<Update::DOWNGRADE>(
          std::get<clang_v9::FormatStyle>(after),
          std::get<clang_v10::FormatStyle>(before), style_i);
      break;
    }
    case Version::V11: {
      after = clang_v10::FormatStyle{};
      clang_update_v11::update<Update::DOWNGRADE>(
          std::get<clang_v10::FormatStyle>(after),
          std::get<clang_v11::FormatStyle>(before), style_i);
      break;
    }
    case Version::V12: {
      after = clang_v11::FormatStyle{};
      clang_update_v12::update<Update::DOWNGRADE>(
          std::get<clang_v11::FormatStyle>(after),
          std::get<clang_v12::FormatStyle>(before), style_i);
      break;
    }
    case Version::V13: {
      after = clang_v12::FormatStyle{};
      clang_update_v13::update<Update::DOWNGRADE>(
          std::get<clang_v12::FormatStyle>(after),
          std::get<clang_v13::FormatStyle>(before), style_i);
      break;
    }
    case Version::V14: {
      after = clang_v13::FormatStyle{};
      clang_update_v14::update<Update::DOWNGRADE>(
          std::get<clang_v13::FormatStyle>(after),
          std::get<clang_v14::FormatStyle>(before), style_i);
      break;
    }
    case Version::V15: {
      after = clang_v14::FormatStyle{};
      clang_update_v15::update<Update::DOWNGRADE>(
          std::get<clang_v14::FormatStyle>(after),
          std::get<clang_v15::FormatStyle>(before), style_i);
      break;
    }
    case Version::V16: {
      after = clang_v15::FormatStyle{};
      clang_update_v16::update<Update::DOWNGRADE>(
          std::get<clang_v15::FormatStyle>(after),
          std::get<clang_v16::FormatStyle>(before), style_i);
      break;
    }
    case Version::V17: {
      after = clang_v16::FormatStyle{};
      clang_update_v17::update<Update::DOWNGRADE>(
          std::get<clang_v16::FormatStyle>(after),
          std::get<clang_v17::FormatStyle>(before), style_i);
      break;
    }
    case Version::V18: {
      after = clang_v17::FormatStyle{};
      clang_update_v18::update<Update::DOWNGRADE>(
          std::get<clang_v17::FormatStyle>(after),
          std::get<clang_v18::FormatStyle>(before), style_i);
      break;
    }
    case Version::V19: {
      after = clang_v18::FormatStyle{};
      clang_update_v19::update<Update::DOWNGRADE>(
          std::get<clang_v18::FormatStyle>(after),
          std::get<clang_v19::FormatStyle>(before), style_i);
      break;
    }
    default: {
      throw std::runtime_error("Unsupported version while upgrading.");
      break;
    }
    }
    std::swap(before, after);
  }

  return formatStyleToVersion(before, default_style, skip_same_value);
}

std::string migrateTo(Version vstart, Version vend, const std::string &data,
                      const std::string &default_style, bool skip_same_value) {
  if (static_cast<size_t>(vstart) < static_cast<size_t>(vend)) {
    return updateTo(vstart, vend, data, default_style, skip_same_value);
  }

  return downgradeTo(vstart, vend, data, default_style, skip_same_value);
}

} // namespace clang_vx