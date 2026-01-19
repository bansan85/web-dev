#pragma once

#include "boost/pfr/core.hpp"
#include "clang/Basic/OperatorPrecedence.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/YAMLTraits.h"
#include <set>
#include <string>
#include <vector>

namespace clang_vx {

namespace tok = clang::tok;

tok::TokenKind getTokenFromQualifier(const std::string &Qualifier);

enum class Version {
  V3_3,
  V3_4,
  V3_5,
  V3_6,
  V3_7,
  V3_8,
  V3_9,
  V4,
  V5,
  V6,
  V7,
  V8,
  V9,
  V10,
  V11,
  V12,
  V13,
  V14,
  V15,
  V16,
  V17,
  V18,
  V19,
  V20,
  V21
};

std::vector<Version> getCompatibleVersion(const std::string &config);

std::string versionEnumToString(Version version);

Version versionStringToEnum(const std::string &version);

std::vector<std::string> getStyleNames(Version version);

std::vector<std::string> getStyleNamesRange(Version vstart, Version vend);

template <typename T> class OutputDiffOnly {
public:
  OutputDiffOnly(const T *defaultStyle, const T &style, bool skip_same)
      : _default_style(defaultStyle), _style(style), _skip_same(skip_same) {};

  const T *getDefaultStyle() const { return _default_style; }
  bool getSkipSame() const { return _skip_same; }

  template <typename U> bool same(U *num) {
    ptrdiff_t offset = (char *)num - (char *)&_style;
    U *equivalent = (U *)((char *)_default_style + offset);
    if constexpr (std::is_aggregate_v<U>) {
      bool result = true;
      boost::pfr::for_each_field(
          *equivalent, [&](const auto &field, std::size_t index) {
            boost::pfr::for_each_field(
                *num, [&](const auto &other_field, std::size_t other_index) {
                  if (index == other_index && field != other_field) {
                    result = false;
                  }
                });
          });
      return result;
    } else {
      return *equivalent == *num;
    }
  }

  template <typename U, typename V> bool same(U *num, V *ref_pos_val) {
    ptrdiff_t offset = (char *)ref_pos_val - (char *)&_style;
    V *equivalent = (V *)((char *)_default_style + offset);
    return static_cast<U>(*equivalent) == *num;
  }

private:
  const T *_default_style;
  const T &_style;
  bool _skip_same;
};

template <typename U, typename T>
void IoMapOptional(llvm::yaml::IO &io, const char *Key, T &Val) {
  if (io.outputting()) {
    OutputDiffOnly<U> &out = *static_cast<OutputDiffOnly<U> *>(io.getContext());
    if (!out.getSkipSame() || !out.same(&Val)) {
      io.mapOptional(Key, Val);
    }
  } else {
    io.mapOptional(Key, Val);
  }
}

template <typename U, typename T, typename V>
void IoMapOptional(llvm::yaml::IO &io, const char *Key, T &Val, V &RefPosVal) {
  if (io.outputting()) {
    if constexpr (std::is_aggregate_v<T>) {
      io.mapOptional(Key, Val);
    } else {
      OutputDiffOnly<U> &out =
          *static_cast<OutputDiffOnly<U> *>(io.getContext());
      if (!out.getSkipSame() || !out.same(&Val, &RefPosVal)) {
        io.mapOptional(Key, Val);
      }
    }
  } else {
    io.mapOptional(Key, Val);
  }
}

template <typename U, typename T>
void IoMapOptionalHardcodedValue(llvm::yaml::IO &io, const char *Key, T &Val) {
  if (io.outputting()) {
    OutputDiffOnly<U> &out = *static_cast<OutputDiffOnly<U> *>(io.getContext());
    if (!out.getSkipSame()) {
      io.mapOptional(Key, Val);
    }
  } else {
    io.mapOptional(Key, Val);
  }
}

} // namespace clang_vx