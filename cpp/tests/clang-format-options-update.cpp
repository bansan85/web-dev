#include "../native/clang-format-options-update/Format.h"
#include "../native/clang-format-options-update/update.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <frozen/bits/hash_string.h>
#include <frozen/unordered_map.h>
#include <frozen/unordered_set.h>
#include <fstream>
#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <sstream>
#include <string>

namespace frozen {
template <> struct elsa<std::string_view> {
  constexpr std::size_t operator()(std::string_view value) const {
    return hash_string(value);
  }
  constexpr std::size_t operator()(std::string_view value,
                                   std::size_t seed) const {
    return hash_string(value, seed);
  }
};
} // namespace frozen

namespace {

std::ostream &operator<<(std::ostream &os, clang_vx::Version v) {
  if (auto name = magic_enum::enum_name(v); !name.empty()) {
    os << name;
  } else {
    os << "Unknown";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const std::vector<clang_vx::Version> &versions) {
  os << "[";
  bool first = true;
  for (const auto &v : versions) {
    if (!first) {
      os << ", ";
    }
    os << v;
    first = false;
  }
  os << "]";
  return os;
}

constexpr frozen::unordered_set<std::string_view, 9> styles{
    "chromium",  "clang-format", "gnu",  "google", "llvm",
    "microsoft", "mozilla",      "none", "webkit"};

std::map<std::string_view, std::vector<clang_vx::Version>> compatibilities{
    {"chromium-3_4.cfg",
     {clang_vx::Version::V3_4, clang_vx::Version::V3_5, clang_vx::Version::V3_6,
      clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"chromium-3_5.cfg", {clang_vx::Version::V3_5, clang_vx::Version::V3_6,
                          clang_vx::Version::V3_7, clang_vx::Version::V3_8,
                          clang_vx::Version::V3_9, clang_vx::Version::V4,
                          clang_vx::Version::V5,   clang_vx::Version::V6,
                          clang_vx::Version::V7,   clang_vx::Version::V8,
                          clang_vx::Version::V9,   clang_vx::Version::V10,
                          clang_vx::Version::V11,  clang_vx::Version::V12,
                          clang_vx::Version::V13,  clang_vx::Version::V14,
                          clang_vx::Version::V15,  clang_vx::Version::V16,
                          clang_vx::Version::V17,  clang_vx::Version::V18,
                          clang_vx::Version::V19}},
    {"chromium-3_6.cfg",
     {clang_vx::Version::V3_6, clang_vx::Version::V3_7, clang_vx::Version::V3_8,
      clang_vx::Version::V3_9, clang_vx::Version::V4,   clang_vx::Version::V5,
      clang_vx::Version::V6,   clang_vx::Version::V7,   clang_vx::Version::V8,
      clang_vx::Version::V9,   clang_vx::Version::V10,  clang_vx::Version::V11,
      clang_vx::Version::V12,  clang_vx::Version::V13,  clang_vx::Version::V14,
      clang_vx::Version::V15,  clang_vx::Version::V16,  clang_vx::Version::V17,
      clang_vx::Version::V18,  clang_vx::Version::V19}},
    {"chromium-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"chromium-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"chromium-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-6.cfg", {clang_vx::Version::V6}},
    {"chromium-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"chromium-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"chromium-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"chromium-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"chromium-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-18.cfg", {clang_vx::Version::V18, clang_vx::Version::V19}},
    {"chromium-19.cfg", {clang_vx::Version::V19}},
    {"clang-format-18.cfg", {clang_vx::Version::V18, clang_vx::Version::V19}},
    {"clang-format-19.cfg", {clang_vx::Version::V19}},
    {"gnu-3_5.cfg", {clang_vx::Version::V3_5, clang_vx::Version::V3_6,
                     clang_vx::Version::V3_7, clang_vx::Version::V3_8,
                     clang_vx::Version::V3_9, clang_vx::Version::V4,
                     clang_vx::Version::V5,   clang_vx::Version::V6,
                     clang_vx::Version::V7,   clang_vx::Version::V8,
                     clang_vx::Version::V9,   clang_vx::Version::V10,
                     clang_vx::Version::V11,  clang_vx::Version::V12,
                     clang_vx::Version::V13,  clang_vx::Version::V14,
                     clang_vx::Version::V15,  clang_vx::Version::V16,
                     clang_vx::Version::V17,  clang_vx::Version::V18,
                     clang_vx::Version::V19}},
    {"gnu-3_6.cfg",
     {clang_vx::Version::V3_6, clang_vx::Version::V3_7, clang_vx::Version::V3_8,
      clang_vx::Version::V3_9, clang_vx::Version::V4,   clang_vx::Version::V5,
      clang_vx::Version::V6,   clang_vx::Version::V7,   clang_vx::Version::V8,
      clang_vx::Version::V9,   clang_vx::Version::V10,  clang_vx::Version::V11,
      clang_vx::Version::V12,  clang_vx::Version::V13,  clang_vx::Version::V14,
      clang_vx::Version::V15,  clang_vx::Version::V16,  clang_vx::Version::V17,
      clang_vx::Version::V18,  clang_vx::Version::V19}},
    {"gnu-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"gnu-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"gnu-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-6.cfg", {clang_vx::Version::V6}},
    {"gnu-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"gnu-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"gnu-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"gnu-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"gnu-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-18.cfg", {clang_vx::Version::V18, clang_vx::Version::V19}},
    {"gnu-19.cfg", {clang_vx::Version::V19}},
    {"google-3_4.cfg",
     {clang_vx::Version::V3_4, clang_vx::Version::V3_5, clang_vx::Version::V3_6,
      clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"google-3_5.cfg", {clang_vx::Version::V3_5, clang_vx::Version::V3_6,
                        clang_vx::Version::V3_7, clang_vx::Version::V3_8,
                        clang_vx::Version::V3_9, clang_vx::Version::V4,
                        clang_vx::Version::V5,   clang_vx::Version::V6,
                        clang_vx::Version::V7,   clang_vx::Version::V8,
                        clang_vx::Version::V9,   clang_vx::Version::V10,
                        clang_vx::Version::V11,  clang_vx::Version::V12,
                        clang_vx::Version::V13,  clang_vx::Version::V14,
                        clang_vx::Version::V15,  clang_vx::Version::V16,
                        clang_vx::Version::V17,  clang_vx::Version::V18,
                        clang_vx::Version::V19}},
    {"google-3_6.cfg",
     {clang_vx::Version::V3_6, clang_vx::Version::V3_7, clang_vx::Version::V3_8,
      clang_vx::Version::V3_9, clang_vx::Version::V4,   clang_vx::Version::V5,
      clang_vx::Version::V6,   clang_vx::Version::V7,   clang_vx::Version::V8,
      clang_vx::Version::V9,   clang_vx::Version::V10,  clang_vx::Version::V11,
      clang_vx::Version::V12,  clang_vx::Version::V13,  clang_vx::Version::V14,
      clang_vx::Version::V15,  clang_vx::Version::V16,  clang_vx::Version::V17,
      clang_vx::Version::V18,  clang_vx::Version::V19}},
    {"google-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"google-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"google-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-6.cfg", {clang_vx::Version::V6}},
    {"google-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"google-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"google-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"google-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"google-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-18.cfg", {clang_vx::Version::V18, clang_vx::Version::V19}},
    {"google-19.cfg", {clang_vx::Version::V19}},
    {"llvm-3_4.cfg",
     {clang_vx::Version::V3_4, clang_vx::Version::V3_5, clang_vx::Version::V3_6,
      clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"llvm-3_5.cfg", {clang_vx::Version::V3_5, clang_vx::Version::V3_6,
                      clang_vx::Version::V3_7, clang_vx::Version::V3_8,
                      clang_vx::Version::V3_9, clang_vx::Version::V4,
                      clang_vx::Version::V5,   clang_vx::Version::V6,
                      clang_vx::Version::V7,   clang_vx::Version::V8,
                      clang_vx::Version::V9,   clang_vx::Version::V10,
                      clang_vx::Version::V11,  clang_vx::Version::V12,
                      clang_vx::Version::V13,  clang_vx::Version::V14,
                      clang_vx::Version::V15,  clang_vx::Version::V16,
                      clang_vx::Version::V17,  clang_vx::Version::V18,
                      clang_vx::Version::V19}},
    {"llvm-3_6.cfg",
     {clang_vx::Version::V3_6, clang_vx::Version::V3_7, clang_vx::Version::V3_8,
      clang_vx::Version::V3_9, clang_vx::Version::V4,   clang_vx::Version::V5,
      clang_vx::Version::V6,   clang_vx::Version::V7,   clang_vx::Version::V8,
      clang_vx::Version::V9,   clang_vx::Version::V10,  clang_vx::Version::V11,
      clang_vx::Version::V12,  clang_vx::Version::V13,  clang_vx::Version::V14,
      clang_vx::Version::V15,  clang_vx::Version::V16,  clang_vx::Version::V17,
      clang_vx::Version::V18,  clang_vx::Version::V19}},
    {"llvm-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"llvm-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"llvm-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-6.cfg", {clang_vx::Version::V6}},
    {"llvm-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"llvm-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"llvm-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"llvm-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"llvm-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-18.cfg", {clang_vx::Version::V18, clang_vx::Version::V19}},
    {"llvm-19.cfg", {clang_vx::Version::V19}},
    {"microsoft-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"microsoft-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"microsoft-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"microsoft-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"microsoft-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"microsoft-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"microsoft-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"microsoft-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"microsoft-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"microsoft-18.cfg", {clang_vx::Version::V18, clang_vx::Version::V19}},
    {"microsoft-19.cfg", {clang_vx::Version::V19}},
    {"mozilla-3_4.cfg",
     {clang_vx::Version::V3_4, clang_vx::Version::V3_5, clang_vx::Version::V3_6,
      clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"mozilla-3_5.cfg", {clang_vx::Version::V3_5, clang_vx::Version::V3_6,
                         clang_vx::Version::V3_7, clang_vx::Version::V3_8,
                         clang_vx::Version::V3_9, clang_vx::Version::V4,
                         clang_vx::Version::V5,   clang_vx::Version::V6,
                         clang_vx::Version::V7,   clang_vx::Version::V8,
                         clang_vx::Version::V9,   clang_vx::Version::V10,
                         clang_vx::Version::V11,  clang_vx::Version::V12,
                         clang_vx::Version::V13,  clang_vx::Version::V14,
                         clang_vx::Version::V15,  clang_vx::Version::V16,
                         clang_vx::Version::V17,  clang_vx::Version::V18,
                         clang_vx::Version::V19}},
    {"mozilla-3_6.cfg",
     {clang_vx::Version::V3_6, clang_vx::Version::V3_7, clang_vx::Version::V3_8,
      clang_vx::Version::V3_9, clang_vx::Version::V4,   clang_vx::Version::V5,
      clang_vx::Version::V6,   clang_vx::Version::V7,   clang_vx::Version::V8,
      clang_vx::Version::V9,   clang_vx::Version::V10,  clang_vx::Version::V11,
      clang_vx::Version::V12,  clang_vx::Version::V13,  clang_vx::Version::V14,
      clang_vx::Version::V15,  clang_vx::Version::V16,  clang_vx::Version::V17,
      clang_vx::Version::V18,  clang_vx::Version::V19}},
    {"mozilla-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"mozilla-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"mozilla-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-6.cfg", {clang_vx::Version::V6}},
    {"mozilla-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"mozilla-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"mozilla-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"mozilla-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"mozilla-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-18.cfg", {clang_vx::Version::V18, clang_vx::Version::V19}},
    {"mozilla-19.cfg", {clang_vx::Version::V19}},
    {"none-3_5.cfg", {clang_vx::Version::V3_5, clang_vx::Version::V3_6,
                      clang_vx::Version::V3_7, clang_vx::Version::V3_8,
                      clang_vx::Version::V3_9, clang_vx::Version::V4,
                      clang_vx::Version::V5,   clang_vx::Version::V6,
                      clang_vx::Version::V7,   clang_vx::Version::V8,
                      clang_vx::Version::V9,   clang_vx::Version::V10,
                      clang_vx::Version::V11,  clang_vx::Version::V12,
                      clang_vx::Version::V13,  clang_vx::Version::V14,
                      clang_vx::Version::V15,  clang_vx::Version::V16,
                      clang_vx::Version::V17,  clang_vx::Version::V18,
                      clang_vx::Version::V19}},
    {"none-3_6.cfg",
     {clang_vx::Version::V3_6, clang_vx::Version::V3_7, clang_vx::Version::V3_8,
      clang_vx::Version::V3_9, clang_vx::Version::V4,   clang_vx::Version::V5,
      clang_vx::Version::V6,   clang_vx::Version::V7,   clang_vx::Version::V8,
      clang_vx::Version::V9,   clang_vx::Version::V10,  clang_vx::Version::V11,
      clang_vx::Version::V12,  clang_vx::Version::V13,  clang_vx::Version::V14,
      clang_vx::Version::V15,  clang_vx::Version::V16,  clang_vx::Version::V17,
      clang_vx::Version::V18,  clang_vx::Version::V19}},
    {"none-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"none-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"none-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-6.cfg", {clang_vx::Version::V6}},
    {"none-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"none-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"none-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"none-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"none-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-18.cfg", {clang_vx::Version::V18, clang_vx::Version::V19}},
    {"none-19.cfg", {clang_vx::Version::V19}},
    {"webkit-3_4.cfg",
     {clang_vx::Version::V3_4, clang_vx::Version::V3_5, clang_vx::Version::V3_6,
      clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"webkit-3_5.cfg", {clang_vx::Version::V3_5, clang_vx::Version::V3_6,
                        clang_vx::Version::V3_7, clang_vx::Version::V3_8,
                        clang_vx::Version::V3_9, clang_vx::Version::V4,
                        clang_vx::Version::V5,   clang_vx::Version::V6,
                        clang_vx::Version::V7,   clang_vx::Version::V8,
                        clang_vx::Version::V9,   clang_vx::Version::V10,
                        clang_vx::Version::V11,  clang_vx::Version::V12,
                        clang_vx::Version::V13,  clang_vx::Version::V14,
                        clang_vx::Version::V15,  clang_vx::Version::V16,
                        clang_vx::Version::V17,  clang_vx::Version::V18,
                        clang_vx::Version::V19}},
    {"webkit-3_6.cfg",
     {clang_vx::Version::V3_6, clang_vx::Version::V3_7, clang_vx::Version::V3_8,
      clang_vx::Version::V3_9, clang_vx::Version::V4,   clang_vx::Version::V5,
      clang_vx::Version::V6,   clang_vx::Version::V7,   clang_vx::Version::V8,
      clang_vx::Version::V9,   clang_vx::Version::V10,  clang_vx::Version::V11,
      clang_vx::Version::V12,  clang_vx::Version::V13,  clang_vx::Version::V14,
      clang_vx::Version::V15,  clang_vx::Version::V16,  clang_vx::Version::V17,
      clang_vx::Version::V18,  clang_vx::Version::V19}},
    {"webkit-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"webkit-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"webkit-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-6.cfg", {clang_vx::Version::V6}},
    {"webkit-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"webkit-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"webkit-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"webkit-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19}},
    {"webkit-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-18.cfg", {clang_vx::Version::V18, clang_vx::Version::V19}},
    {"webkit-19.cfg", {clang_vx::Version::V19}}};

} // namespace

TEST_CASE("getCompatibleVersion", "[clang-format-options-update]") {
  for (std::string_view style : styles) {
    for (clang_vx::Version version :
         magic_enum::enum_values<clang_vx::Version>()) {
      std::string filename =
          std::string{style} + "-" +
          std::string{magic_enum::enum_name(version).substr(1)} + ".cfg";
      if (std::filesystem::exists(filename)) {
        std::ifstream myfile;
        myfile.open(filename);
        std::string myline;
        std::stringstream ss;
        REQUIRE(myfile.is_open());
        while (myfile) {
          std::getline(myfile, myline);
          ss << myline << '\n';
        }
        std::vector<clang_vx::Version> compatible_versions =
            clang_vx::getCompatibleVersion(ss.str());
        const std::vector<clang_vx::Version> &result_expected =
            compatibilities.at(filename);
        REQUIRE(result_expected == compatible_versions);
      }
    }
  }
}

TEST_CASE("updateEnum", "[clang-format-options-update]") {
  for (const std::string_view &style_sv : styles) {
    std::string style{style_sv};
    if (clang_v3_3::FormatStyle style3_3_old;
        clang_v3_3::getPredefinedStyle(style, &style3_3_old)) {
      clang_v3_4::FormatStyle style3_4_new;
      clang_update_v3_4::update<clang_vx::Update::UPGRADE>(style3_3_old,
                                                           style3_4_new, style);

      clang_v3_4::FormatStyle style3_4_old;
      clang_v3_4::getPredefinedStyle(style, &style3_4_old);
      clang_v3_3::FormatStyle style3_3_new;
      clang_update_v3_4::update<clang_vx::Update::DOWNGRADE>(
          style3_3_new, style3_4_old, style);

      if (style == "llvm") {
        style3_3_old.PenaltyReturnTypeOnItsOwnLine = 10;
        style3_3_new.PenaltyReturnTypeOnItsOwnLine = 10;
        style3_4_old.PenaltyReturnTypeOnItsOwnLine = 10;
        style3_4_new.PenaltyReturnTypeOnItsOwnLine = 10;
      }

      REQUIRE(style3_3_old == style3_3_new);
      REQUIRE(style3_4_old == style3_4_new);
    }

    if (clang_v3_4::FormatStyle style3_4_old;
        clang_v3_4::getPredefinedStyle(style, &style3_4_old)) {
      clang_v3_5::FormatStyle style3_5_new;
      clang_update_v3_5::update<clang_vx::Update::UPGRADE>(style3_4_old,
                                                           style3_5_new, style);

      clang_v3_5::FormatStyle style3_5_old;
      clang_v3_5::getPredefinedStyle(
          style, clang_v3_5::FormatStyle::LanguageKind::LK_Cpp, &style3_5_old);
      clang_v3_4::FormatStyle style3_4_new;
      clang_update_v3_5::update<clang_vx::Update::DOWNGRADE>(
          style3_4_new, style3_5_old, style);

      // Default value changed for PenaltyBreakComment.
      style3_4_old.PenaltyBreakComment = 150;
      style3_4_new.PenaltyBreakComment = 150;
      style3_5_old.PenaltyBreakComment = 150;
      style3_5_new.PenaltyBreakComment = 150;
      // and for IndentFunctionDeclarationAfterType
      style3_4_old.IndentFunctionDeclarationAfterType = true;
      style3_4_new.IndentFunctionDeclarationAfterType = true;
      style3_5_old.IndentWrappedFunctionNames = true;
      style3_5_new.IndentWrappedFunctionNames = true;

      if (style == "llvm") {
        style3_4_old.Standard =
            clang_v3_4::FormatStyle::LanguageStandard::LS_Auto;
        style3_4_new.Standard =
            clang_v3_4::FormatStyle::LanguageStandard::LS_Auto;
        style3_5_old.Standard =
            clang_v3_5::FormatStyle::LanguageStandard::LS_Auto;
        style3_5_new.Standard =
            clang_v3_5::FormatStyle::LanguageStandard::LS_Auto;
        style3_4_old.Cpp11BracedListStyle = true;
        style3_4_new.Cpp11BracedListStyle = true;
        style3_5_old.Cpp11BracedListStyle = true;
        style3_5_new.Cpp11BracedListStyle = true;
      }

      REQUIRE(style3_4_old == style3_4_new);
      REQUIRE(style3_5_old == style3_5_new);
    }

    if (clang_v3_5::FormatStyle style3_5_old; clang_v3_5::getPredefinedStyle(
            style, clang_v3_5::FormatStyle::LanguageKind::LK_Cpp,
            &style3_5_old)) {
      clang_v3_6::FormatStyle style3_6_new;
      clang_update_v3_6::update<clang_vx::Update::UPGRADE>(style3_5_old,
                                                           style3_6_new, style);

      clang_v3_6::FormatStyle style3_6_old;
      clang_v3_6::getPredefinedStyle(
          style, clang_v3_6::FormatStyle::LanguageKind::LK_Cpp, &style3_6_old);
      clang_v3_5::FormatStyle style3_5_new;
      clang_update_v3_6::update<clang_vx::Update::DOWNGRADE>(
          style3_5_new, style3_6_old, style);

      if (style == "chromium") {
        style3_5_old.Standard =
            clang_v3_5::FormatStyle::LanguageStandard::LS_Auto;
        style3_5_new.Standard =
            clang_v3_5::FormatStyle::LanguageStandard::LS_Auto;
        style3_6_old.Standard =
            clang_v3_6::FormatStyle::LanguageStandard::LS_Auto;
        style3_6_new.Standard =
            clang_v3_6::FormatStyle::LanguageStandard::LS_Auto;
      }

      REQUIRE(style3_5_old == style3_5_new);
      REQUIRE(style3_6_old == style3_6_new);
    }

    if (clang_v3_6::FormatStyle style3_6_old; clang_v3_6::getPredefinedStyle(
            style, clang_v3_6::FormatStyle::LanguageKind::LK_Cpp,
            &style3_6_old)) {
      clang_v3_7::FormatStyle style3_7_new;
      clang_update_v3_7::update<clang_vx::Update::UPGRADE>(style3_6_old,
                                                           style3_7_new, style);

      clang_v3_7::FormatStyle style3_7_old;
      clang_v3_7::getPredefinedStyle(
          style, clang_v3_7::FormatStyle::LanguageKind::LK_Cpp, &style3_7_old);
      clang_v3_6::FormatStyle style3_6_new;
      clang_update_v3_7::update<clang_vx::Update::DOWNGRADE>(
          style3_6_new, style3_7_old, style);

      if (style == "mozilla") {
        style3_6_old.DerivePointerAlignment = true;
        style3_6_new.DerivePointerAlignment = true;
        style3_7_old.DerivePointerAlignment = true;
        style3_7_new.DerivePointerAlignment = true;
        style3_6_old.Standard =
            clang_v3_6::FormatStyle::LanguageStandard::LS_Auto;
        style3_6_new.Standard =
            clang_v3_6::FormatStyle::LanguageStandard::LS_Auto;
        style3_7_old.Standard =
            clang_v3_7::FormatStyle::LanguageStandard::LS_Auto;
        style3_7_new.Standard =
            clang_v3_7::FormatStyle::LanguageStandard::LS_Auto;
        style3_6_old.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
        style3_6_new.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
        style3_7_old.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
        style3_7_new.ConstructorInitializerAllOnOneLineOrOnePerLine = true;
        style3_6_old.BreakConstructorInitializersBeforeComma = true;
        style3_6_new.BreakConstructorInitializersBeforeComma = true;
        style3_7_old.BreakConstructorInitializersBeforeComma = true;
        style3_7_new.BreakConstructorInitializersBeforeComma = true;
        style3_6_old.AllowShortFunctionsOnASingleLine =
            clang_v3_6::FormatStyle::ShortFunctionStyle::SFS_Empty;
        style3_6_new.AllowShortFunctionsOnASingleLine =
            clang_v3_6::FormatStyle::ShortFunctionStyle::SFS_Empty;
        style3_7_old.AllowShortFunctionsOnASingleLine =
            clang_v3_7::FormatStyle::ShortFunctionStyle::SFS_Empty;
        style3_7_new.AllowShortFunctionsOnASingleLine =
            clang_v3_7::FormatStyle::ShortFunctionStyle::SFS_Empty;
        style3_6_old.ConstructorInitializerIndentWidth = 3;
        style3_6_new.ConstructorInitializerIndentWidth = 3;
        style3_7_old.ConstructorInitializerIndentWidth = 3;
        style3_7_new.ConstructorInitializerIndentWidth = 3;
        style3_6_old.AlwaysBreakTemplateDeclarations = true;
        style3_6_new.AlwaysBreakTemplateDeclarations = true;
        style3_7_old.AlwaysBreakTemplateDeclarations = true;
        style3_7_new.AlwaysBreakTemplateDeclarations = true;
        style3_6_old.ContinuationIndentWidth = 3;
        style3_6_new.ContinuationIndentWidth = 3;
        style3_7_old.ContinuationIndentWidth = 3;
        style3_7_new.ContinuationIndentWidth = 3;
        style3_6_old.AlwaysBreakAfterDefinitionReturnType = false;
        style3_6_new.AlwaysBreakAfterDefinitionReturnType = false;
        style3_7_old.AlwaysBreakAfterDefinitionReturnType = clang_v3_7::
            FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None;
        style3_7_new.AlwaysBreakAfterDefinitionReturnType = clang_v3_7::
            FormatStyle::DefinitionReturnTypeBreakingStyle::DRTBS_None;
        style3_6_old.BreakBeforeBraces =
            clang_v3_6::FormatStyle::BraceBreakingStyle::BS_Attach;
        style3_6_new.BreakBeforeBraces =
            clang_v3_6::FormatStyle::BraceBreakingStyle::BS_Attach;
        style3_7_old.BreakBeforeBraces =
            clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Attach;
        style3_7_new.BreakBeforeBraces =
            clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Attach;
      }

      REQUIRE(style3_6_old == style3_6_new);
      REQUIRE(style3_7_old == style3_7_new);
    }

    if (clang_v3_7::FormatStyle style3_7_old; clang_v3_7::getPredefinedStyle(
            style, clang_v3_7::FormatStyle::LanguageKind::LK_Cpp,
            &style3_7_old)) {
      clang_v3_8::FormatStyle style3_8_new;
      clang_update_v3_8::update<clang_vx::Update::UPGRADE>(style3_7_old,
                                                           style3_8_new, style);

      clang_v3_8::FormatStyle style3_8_old;
      clang_v3_8::getPredefinedStyle(
          style, clang_v3_8::FormatStyle::LanguageKind::LK_Cpp, &style3_8_old);
      clang_v3_7::FormatStyle style3_7_new;
      clang_update_v3_8::update<clang_vx::Update::DOWNGRADE>(
          style3_7_new, style3_8_old, style);

      if (style == "chromium") {
        style3_7_old.MacroBlockBegin = "";
        style3_7_new.MacroBlockBegin = "";
        style3_8_old.MacroBlockBegin = "";
        style3_8_new.MacroBlockBegin = "";
        style3_7_old.MacroBlockEnd = "";
        style3_7_new.MacroBlockEnd = "";
        style3_8_old.MacroBlockEnd = "";
        style3_8_new.MacroBlockEnd = "";
      }
      if (style == "webkit") {
        style3_7_old.BreakBeforeBraces =
            clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Attach;
        style3_7_new.BreakBeforeBraces =
            clang_v3_7::FormatStyle::BraceBreakingStyle::BS_Attach;
        style3_8_old.BreakBeforeBraces =
            clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Attach;
        style3_8_new.BreakBeforeBraces =
            clang_v3_8::FormatStyle::BraceBreakingStyle::BS_Attach;
      }

      REQUIRE(style3_7_old == style3_7_new);
      REQUIRE(style3_8_old == style3_8_new);
    }

    if (clang_v3_8::FormatStyle style3_8_old; clang_v3_8::getPredefinedStyle(
            style, clang_v3_8::FormatStyle::LanguageKind::LK_Cpp,
            &style3_8_old)) {
      clang_v3_9::FormatStyle style3_9_new;
      clang_update_v3_9::update<clang_vx::Update::UPGRADE>(style3_8_old,
                                                           style3_9_new, style);

      clang_v3_9::FormatStyle style3_9_old;
      clang_v3_9::getPredefinedStyle(
          style, clang_v3_9::FormatStyle::LanguageKind::LK_Cpp, &style3_9_old);
      clang_v3_8::FormatStyle style3_8_new;
      clang_update_v3_9::update<clang_vx::Update::DOWNGRADE>(
          style3_8_new, style3_9_old, style);

      REQUIRE(style3_8_old == style3_8_new);
      REQUIRE(style3_9_old == style3_9_new);
    }

    if (clang_v3_9::FormatStyle style3_9_old; clang_v3_9::getPredefinedStyle(
            style, clang_v3_9::FormatStyle::LanguageKind::LK_Cpp,
            &style3_9_old)) {
      clang_v4::FormatStyle style4_new;
      clang_update_v4::update<clang_vx::Update::UPGRADE>(style3_9_old,
                                                         style4_new, style);

      clang_v4::FormatStyle style4_old;
      clang_v4::getPredefinedStyle(
          style, clang_v4::FormatStyle::LanguageKind::LK_Cpp, &style4_old);
      clang_v3_9::FormatStyle style3_9_new;
      clang_update_v4::update<clang_vx::Update::DOWNGRADE>(style3_9_new,
                                                           style4_old, style);

      if (style == "mozilla") {
        style3_9_old.AlwaysBreakAfterReturnType = clang_v3_9::FormatStyle::
            ReturnTypeBreakingStyle::RTBS_TopLevelDefinitions;
        style3_9_new.AlwaysBreakAfterReturnType = clang_v3_9::FormatStyle::
            ReturnTypeBreakingStyle::RTBS_TopLevelDefinitions;
        style4_old.AlwaysBreakAfterReturnType = clang_v4::FormatStyle::
            ReturnTypeBreakingStyle::RTBS_TopLevelDefinitions;
        style4_new.AlwaysBreakAfterReturnType = clang_v4::FormatStyle::
            ReturnTypeBreakingStyle::RTBS_TopLevelDefinitions;
        style3_9_old.BinPackParameters = true;
        style3_9_new.BinPackParameters = true;
        style4_old.BinPackParameters = true;
        style4_new.BinPackParameters = true;
        style3_9_old.BinPackArguments = true;
        style3_9_new.BinPackArguments = true;
        style4_old.BinPackArguments = true;
        style4_new.BinPackArguments = true;
      }

      if (style == "webkit") {
        style3_9_old.Standard =
            clang_v3_9::FormatStyle::LanguageStandard::LS_Auto;
        style3_9_new.Standard =
            clang_v3_9::FormatStyle::LanguageStandard::LS_Auto;
        style4_old.Standard = clang_v4::FormatStyle::LanguageStandard::LS_Auto;
        style4_new.Standard = clang_v4::FormatStyle::LanguageStandard::LS_Auto;
      }

      REQUIRE(style3_9_old == style3_9_new);
      REQUIRE(style4_old == style4_new);
    }

    if (clang_v4::FormatStyle style4_old; clang_v4::getPredefinedStyle(
            style, clang_v4::FormatStyle::LanguageKind::LK_Cpp, &style4_old)) {
      clang_v5::FormatStyle style5_new;
      clang_update_v5::update<clang_vx::Update::UPGRADE>(style4_old, style5_new,
                                                         style);

      clang_v5::FormatStyle style5_old;
      clang_v5::getPredefinedStyle(
          style, clang_v5::FormatStyle::LanguageKind::LK_Cpp, &style5_old);
      clang_v4::FormatStyle style4_new;
      clang_update_v5::update<clang_vx::Update::DOWNGRADE>(style4_new,
                                                           style5_old, style);

      style4_old.BreakConstructorInitializersBeforeComma = true;
      style4_new.BreakConstructorInitializersBeforeComma = true;
      style5_old.BreakConstructorInitializers = clang_v5::FormatStyle::
          BreakConstructorInitializersStyle::BCIS_BeforeComma;
      style5_new.BreakConstructorInitializers = clang_v5::FormatStyle::
          BreakConstructorInitializersStyle::BCIS_BeforeComma;
      if (style == "chromium") {
        style4_old.SortIncludes = true;
        style4_new.SortIncludes = true;
        style5_old.SortIncludes = true;
        style5_new.SortIncludes = true;
      }
      if (style == "gnu") {
        style4_old.IncludeIsMainRegex = "$";
        style4_new.IncludeIsMainRegex = "$";
        style5_old.IncludeIsMainRegex = "$";
        style5_new.IncludeIsMainRegex = "$";
      }

      REQUIRE(style4_old == style4_new);
      REQUIRE(style5_old == style5_new);
    }

    if (clang_v5::FormatStyle style5_old; clang_v5::getPredefinedStyle(
            style, clang_v5::FormatStyle::LanguageKind::LK_Cpp, &style5_old)) {
      clang_v6::FormatStyle style6_new;
      clang_update_v6::update<clang_vx::Update::UPGRADE>(style5_old, style6_new,
                                                         style);

      clang_v6::FormatStyle style6_old;
      clang_v6::getPredefinedStyle(
          style, clang_v6::FormatStyle::LanguageKind::LK_Cpp, &style6_old);
      clang_v5::FormatStyle style5_new;
      clang_update_v6::update<clang_vx::Update::DOWNGRADE>(style5_new,
                                                           style6_old, style);

      REQUIRE(style5_old == style5_new);
      REQUIRE(style6_old == style6_new);
    }

    if (clang_v6::FormatStyle style6_old; clang_v6::getPredefinedStyle(
            style, clang_v6::FormatStyle::LanguageKind::LK_Cpp, &style6_old)) {
      clang_v7::FormatStyle style7_new;
      clang_update_v7::update<clang_vx::Update::UPGRADE>(style6_old, style7_new,
                                                         style);

      clang_v7::FormatStyle style7_old;
      clang_v7::getPredefinedStyle(
          style, clang_v7::FormatStyle::LanguageKind::LK_Cpp, &style7_old);
      clang_v6::FormatStyle style6_new;
      clang_update_v7::update<clang_vx::Update::DOWNGRADE>(style6_new,
                                                           style7_old, style);

      if (style == "chromium" || style == "google") {
        style6_old.ObjCSpaceBeforeProtocolList = true;
        style6_new.ObjCSpaceBeforeProtocolList = true;
        style7_old.ObjCSpaceBeforeProtocolList = true;
        style7_new.ObjCSpaceBeforeProtocolList = true;
      }

      REQUIRE(style6_old == style6_new);
      REQUIRE(style7_old == style7_new);
    }

    if (clang_v7::FormatStyle style7_old; clang_v7::getPredefinedStyle(
            style, clang_v7::FormatStyle::LanguageKind::LK_Cpp, &style7_old)) {
      clang_v8::FormatStyle style8_new;
      clang_update_v8::update<clang_vx::Update::UPGRADE>(style7_old, style8_new,
                                                         style);

      clang_v8::FormatStyle style8_old;
      clang_v8::getPredefinedStyle(
          style, clang_v8::FormatStyle::LanguageKind::LK_Cpp, &style8_old);
      clang_v7::FormatStyle style7_new;
      clang_update_v8::update<clang_vx::Update::DOWNGRADE>(style7_new,
                                                           style8_old, style);

      REQUIRE(style7_old == style7_new);
      REQUIRE(style8_old == style8_new);
    }

    if (clang_v8::FormatStyle style8_old; clang_v8::getPredefinedStyle(
            style, clang_v8::FormatStyle::LanguageKind::LK_Cpp, &style8_old)) {
      clang_v9::FormatStyle style9_new;
      clang_update_v9::update<clang_vx::Update::UPGRADE>(style8_old, style9_new,
                                                         style);

      clang_v9::FormatStyle style9_old;
      clang_v9::getPredefinedStyle(
          style, clang_v9::FormatStyle::LanguageKind::LK_Cpp, &style9_old);
      clang_v8::FormatStyle style8_new;
      clang_update_v9::update<clang_vx::Update::DOWNGRADE>(style8_new,
                                                           style9_old, style);

      if (style == "chromium" || style == "google") {
        style8_old.IncludeStyle.IncludeBlocks =
            clang_v8::IncludeStyle::IBS_Preserve;
        style8_new.IncludeStyle.IncludeBlocks =
            clang_v8::IncludeStyle::IBS_Preserve;
        style9_old.IncludeStyle.IncludeBlocks =
            clang_v9::IncludeStyle::IBS_Preserve;
        style9_new.IncludeStyle.IncludeBlocks =
            clang_v9::IncludeStyle::IBS_Preserve;
      }

      REQUIRE(style8_old == style8_new);
      REQUIRE(style9_old == style9_new);
    }
  }
}
