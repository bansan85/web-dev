#include "../native/clang-format-config-migrate/Format.h"
#include "../native/clang-format-config-migrate/update.h"
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
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"chromium-3_5.cfg",
     {clang_vx::Version::V3_5, clang_vx::Version::V3_6, clang_vx::Version::V3_7,
      clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5,   clang_vx::Version::V6,   clang_vx::Version::V7,
      clang_vx::Version::V8,   clang_vx::Version::V9,   clang_vx::Version::V10,
      clang_vx::Version::V11,  clang_vx::Version::V12,  clang_vx::Version::V13,
      clang_vx::Version::V14,  clang_vx::Version::V15,  clang_vx::Version::V16,
      clang_vx::Version::V17,  clang_vx::Version::V18,  clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"chromium-3_6.cfg", {clang_vx::Version::V3_6, clang_vx::Version::V3_7,
                          clang_vx::Version::V3_8, clang_vx::Version::V3_9,
                          clang_vx::Version::V4,   clang_vx::Version::V5,
                          clang_vx::Version::V6,   clang_vx::Version::V7,
                          clang_vx::Version::V8,   clang_vx::Version::V9,
                          clang_vx::Version::V10,  clang_vx::Version::V11,
                          clang_vx::Version::V12,  clang_vx::Version::V13,
                          clang_vx::Version::V14,  clang_vx::Version::V15,
                          clang_vx::Version::V16,  clang_vx::Version::V17,
                          clang_vx::Version::V18,  clang_vx::Version::V19,
                          clang_vx::Version::V20}},
    {"chromium-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"chromium-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"chromium-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"chromium-6.cfg", {clang_vx::Version::V6}},
    {"chromium-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"chromium-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"chromium-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"chromium-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"chromium-18.cfg",
     {clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-19.cfg", {clang_vx::Version::V19, clang_vx::Version::V20}},
    {"chromium-20.cfg", {clang_vx::Version::V20}},
    {"clang-format-18.cfg",
     {clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"clang-format-19.cfg", {clang_vx::Version::V19, clang_vx::Version::V20}},
    {"clang-format-20.cfg", {clang_vx::Version::V20}},
    {"gnu-3_5.cfg",
     {clang_vx::Version::V3_5, clang_vx::Version::V3_6, clang_vx::Version::V3_7,
      clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5,   clang_vx::Version::V6,   clang_vx::Version::V7,
      clang_vx::Version::V8,   clang_vx::Version::V9,   clang_vx::Version::V10,
      clang_vx::Version::V11,  clang_vx::Version::V12,  clang_vx::Version::V13,
      clang_vx::Version::V14,  clang_vx::Version::V15,  clang_vx::Version::V16,
      clang_vx::Version::V17,  clang_vx::Version::V18,  clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"gnu-3_6.cfg", {clang_vx::Version::V3_6, clang_vx::Version::V3_7,
                     clang_vx::Version::V3_8, clang_vx::Version::V3_9,
                     clang_vx::Version::V4,   clang_vx::Version::V5,
                     clang_vx::Version::V6,   clang_vx::Version::V7,
                     clang_vx::Version::V8,   clang_vx::Version::V9,
                     clang_vx::Version::V10,  clang_vx::Version::V11,
                     clang_vx::Version::V12,  clang_vx::Version::V13,
                     clang_vx::Version::V14,  clang_vx::Version::V15,
                     clang_vx::Version::V16,  clang_vx::Version::V17,
                     clang_vx::Version::V18,  clang_vx::Version::V19,
                     clang_vx::Version::V20}},
    {"gnu-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"gnu-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"gnu-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"gnu-6.cfg", {clang_vx::Version::V6}},
    {"gnu-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"gnu-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"gnu-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"gnu-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"gnu-18.cfg",
     {clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-19.cfg", {clang_vx::Version::V19, clang_vx::Version::V20}},
    {"gnu-20.cfg", {clang_vx::Version::V20}},
    {"google-3_4.cfg",
     {clang_vx::Version::V3_4, clang_vx::Version::V3_5, clang_vx::Version::V3_6,
      clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"google-3_5.cfg",
     {clang_vx::Version::V3_5, clang_vx::Version::V3_6, clang_vx::Version::V3_7,
      clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5,   clang_vx::Version::V6,   clang_vx::Version::V7,
      clang_vx::Version::V8,   clang_vx::Version::V9,   clang_vx::Version::V10,
      clang_vx::Version::V11,  clang_vx::Version::V12,  clang_vx::Version::V13,
      clang_vx::Version::V14,  clang_vx::Version::V15,  clang_vx::Version::V16,
      clang_vx::Version::V17,  clang_vx::Version::V18,  clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"google-3_6.cfg", {clang_vx::Version::V3_6, clang_vx::Version::V3_7,
                        clang_vx::Version::V3_8, clang_vx::Version::V3_9,
                        clang_vx::Version::V4,   clang_vx::Version::V5,
                        clang_vx::Version::V6,   clang_vx::Version::V7,
                        clang_vx::Version::V8,   clang_vx::Version::V9,
                        clang_vx::Version::V10,  clang_vx::Version::V11,
                        clang_vx::Version::V12,  clang_vx::Version::V13,
                        clang_vx::Version::V14,  clang_vx::Version::V15,
                        clang_vx::Version::V16,  clang_vx::Version::V17,
                        clang_vx::Version::V18,  clang_vx::Version::V19,
                        clang_vx::Version::V20}},
    {"google-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"google-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"google-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"google-6.cfg", {clang_vx::Version::V6}},
    {"google-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"google-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"google-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"google-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"google-18.cfg",
     {clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-19.cfg", {clang_vx::Version::V19, clang_vx::Version::V20}},
    {"google-20.cfg", {clang_vx::Version::V20}},
    {"llvm-3_4.cfg",
     {clang_vx::Version::V3_4, clang_vx::Version::V3_5, clang_vx::Version::V3_6,
      clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"llvm-3_5.cfg",
     {clang_vx::Version::V3_5, clang_vx::Version::V3_6, clang_vx::Version::V3_7,
      clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5,   clang_vx::Version::V6,   clang_vx::Version::V7,
      clang_vx::Version::V8,   clang_vx::Version::V9,   clang_vx::Version::V10,
      clang_vx::Version::V11,  clang_vx::Version::V12,  clang_vx::Version::V13,
      clang_vx::Version::V14,  clang_vx::Version::V15,  clang_vx::Version::V16,
      clang_vx::Version::V17,  clang_vx::Version::V18,  clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"llvm-3_6.cfg", {clang_vx::Version::V3_6, clang_vx::Version::V3_7,
                      clang_vx::Version::V3_8, clang_vx::Version::V3_9,
                      clang_vx::Version::V4,   clang_vx::Version::V5,
                      clang_vx::Version::V6,   clang_vx::Version::V7,
                      clang_vx::Version::V8,   clang_vx::Version::V9,
                      clang_vx::Version::V10,  clang_vx::Version::V11,
                      clang_vx::Version::V12,  clang_vx::Version::V13,
                      clang_vx::Version::V14,  clang_vx::Version::V15,
                      clang_vx::Version::V16,  clang_vx::Version::V17,
                      clang_vx::Version::V18,  clang_vx::Version::V19,
                      clang_vx::Version::V20}},
    {"llvm-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"llvm-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"llvm-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"llvm-6.cfg", {clang_vx::Version::V6}},
    {"llvm-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"llvm-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"llvm-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"llvm-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"llvm-18.cfg",
     {clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-19.cfg", {clang_vx::Version::V19, clang_vx::Version::V20}},
    {"llvm-20.cfg", {clang_vx::Version::V20}},
    {"microsoft-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"microsoft-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"microsoft-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"microsoft-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"microsoft-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"microsoft-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"microsoft-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"microsoft-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"microsoft-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"microsoft-18.cfg",
     {clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"microsoft-19.cfg", {clang_vx::Version::V19, clang_vx::Version::V20}},
    {"microsoft-20.cfg", {clang_vx::Version::V20}},
    {"mozilla-3_4.cfg",
     {clang_vx::Version::V3_4, clang_vx::Version::V3_5, clang_vx::Version::V3_6,
      clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"mozilla-3_5.cfg",
     {clang_vx::Version::V3_5, clang_vx::Version::V3_6, clang_vx::Version::V3_7,
      clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5,   clang_vx::Version::V6,   clang_vx::Version::V7,
      clang_vx::Version::V8,   clang_vx::Version::V9,   clang_vx::Version::V10,
      clang_vx::Version::V11,  clang_vx::Version::V12,  clang_vx::Version::V13,
      clang_vx::Version::V14,  clang_vx::Version::V15,  clang_vx::Version::V16,
      clang_vx::Version::V17,  clang_vx::Version::V18,  clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"mozilla-3_6.cfg", {clang_vx::Version::V3_6, clang_vx::Version::V3_7,
                         clang_vx::Version::V3_8, clang_vx::Version::V3_9,
                         clang_vx::Version::V4,   clang_vx::Version::V5,
                         clang_vx::Version::V6,   clang_vx::Version::V7,
                         clang_vx::Version::V8,   clang_vx::Version::V9,
                         clang_vx::Version::V10,  clang_vx::Version::V11,
                         clang_vx::Version::V12,  clang_vx::Version::V13,
                         clang_vx::Version::V14,  clang_vx::Version::V15,
                         clang_vx::Version::V16,  clang_vx::Version::V17,
                         clang_vx::Version::V18,  clang_vx::Version::V19,
                         clang_vx::Version::V20}},
    {"mozilla-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"mozilla-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"mozilla-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"mozilla-6.cfg", {clang_vx::Version::V6}},
    {"mozilla-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"mozilla-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"mozilla-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"mozilla-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"mozilla-18.cfg",
     {clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-19.cfg", {clang_vx::Version::V19, clang_vx::Version::V20}},
    {"mozilla-20.cfg", {clang_vx::Version::V20}},
    {"none-3_5.cfg",
     {clang_vx::Version::V3_5, clang_vx::Version::V3_6, clang_vx::Version::V3_7,
      clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5,   clang_vx::Version::V6,   clang_vx::Version::V7,
      clang_vx::Version::V8,   clang_vx::Version::V9,   clang_vx::Version::V10,
      clang_vx::Version::V11,  clang_vx::Version::V12,  clang_vx::Version::V13,
      clang_vx::Version::V14,  clang_vx::Version::V15,  clang_vx::Version::V16,
      clang_vx::Version::V17,  clang_vx::Version::V18,  clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"none-3_6.cfg", {clang_vx::Version::V3_6, clang_vx::Version::V3_7,
                      clang_vx::Version::V3_8, clang_vx::Version::V3_9,
                      clang_vx::Version::V4,   clang_vx::Version::V5,
                      clang_vx::Version::V6,   clang_vx::Version::V7,
                      clang_vx::Version::V8,   clang_vx::Version::V9,
                      clang_vx::Version::V10,  clang_vx::Version::V11,
                      clang_vx::Version::V12,  clang_vx::Version::V13,
                      clang_vx::Version::V14,  clang_vx::Version::V15,
                      clang_vx::Version::V16,  clang_vx::Version::V17,
                      clang_vx::Version::V18,  clang_vx::Version::V19,
                      clang_vx::Version::V20}},
    {"none-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"none-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"none-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"none-6.cfg", {clang_vx::Version::V6}},
    {"none-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"none-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"none-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"none-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"none-18.cfg",
     {clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-19.cfg", {clang_vx::Version::V19, clang_vx::Version::V20}},
    {"none-20.cfg", {clang_vx::Version::V20}},
    {"webkit-3_4.cfg",
     {clang_vx::Version::V3_4, clang_vx::Version::V3_5, clang_vx::Version::V3_6,
      clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"webkit-3_5.cfg",
     {clang_vx::Version::V3_5, clang_vx::Version::V3_6, clang_vx::Version::V3_7,
      clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5,   clang_vx::Version::V6,   clang_vx::Version::V7,
      clang_vx::Version::V8,   clang_vx::Version::V9,   clang_vx::Version::V10,
      clang_vx::Version::V11,  clang_vx::Version::V12,  clang_vx::Version::V13,
      clang_vx::Version::V14,  clang_vx::Version::V15,  clang_vx::Version::V16,
      clang_vx::Version::V17,  clang_vx::Version::V18,  clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"webkit-3_6.cfg", {clang_vx::Version::V3_6, clang_vx::Version::V3_7,
                        clang_vx::Version::V3_8, clang_vx::Version::V3_9,
                        clang_vx::Version::V4,   clang_vx::Version::V5,
                        clang_vx::Version::V6,   clang_vx::Version::V7,
                        clang_vx::Version::V8,   clang_vx::Version::V9,
                        clang_vx::Version::V10,  clang_vx::Version::V11,
                        clang_vx::Version::V12,  clang_vx::Version::V13,
                        clang_vx::Version::V14,  clang_vx::Version::V15,
                        clang_vx::Version::V16,  clang_vx::Version::V17,
                        clang_vx::Version::V18,  clang_vx::Version::V19,
                        clang_vx::Version::V20}},
    {"webkit-3_7.cfg",
     {clang_vx::Version::V3_7, clang_vx::Version::V3_8, clang_vx::Version::V3_9,
      clang_vx::Version::V4,   clang_vx::Version::V5,   clang_vx::Version::V6,
      clang_vx::Version::V7,   clang_vx::Version::V8,   clang_vx::Version::V9,
      clang_vx::Version::V10,  clang_vx::Version::V11,  clang_vx::Version::V12,
      clang_vx::Version::V13,  clang_vx::Version::V14,  clang_vx::Version::V15,
      clang_vx::Version::V16,  clang_vx::Version::V17,  clang_vx::Version::V18,
      clang_vx::Version::V19,  clang_vx::Version::V20}},
    {"webkit-3_8.cfg",
     {clang_vx::Version::V3_8, clang_vx::Version::V3_9, clang_vx::Version::V4,
      clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"webkit-3_9.cfg",
     {clang_vx::Version::V3_9, clang_vx::Version::V4, clang_vx::Version::V5,
      clang_vx::Version::V6, clang_vx::Version::V7, clang_vx::Version::V8,
      clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-4.cfg",
     {clang_vx::Version::V4, clang_vx::Version::V5, clang_vx::Version::V6,
      clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-5.cfg",
     {clang_vx::Version::V5, clang_vx::Version::V6, clang_vx::Version::V7,
      clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"webkit-6.cfg", {clang_vx::Version::V6}},
    {"webkit-7.cfg",
     {clang_vx::Version::V7, clang_vx::Version::V8, clang_vx::Version::V9,
      clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-8.cfg",
     {clang_vx::Version::V8, clang_vx::Version::V9, clang_vx::Version::V10,
      clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"webkit-9.cfg",
     {clang_vx::Version::V9, clang_vx::Version::V10, clang_vx::Version::V11,
      clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-10.cfg",
     {clang_vx::Version::V10, clang_vx::Version::V11, clang_vx::Version::V12,
      clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-11.cfg",
     {clang_vx::Version::V11, clang_vx::Version::V12, clang_vx::Version::V13,
      clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"webkit-12.cfg",
     {clang_vx::Version::V12, clang_vx::Version::V13, clang_vx::Version::V14,
      clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-13.cfg",
     {clang_vx::Version::V13, clang_vx::Version::V14, clang_vx::Version::V15,
      clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-14.cfg",
     {clang_vx::Version::V14, clang_vx::Version::V15, clang_vx::Version::V16,
      clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"webkit-15.cfg",
     {clang_vx::Version::V15, clang_vx::Version::V16, clang_vx::Version::V17,
      clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-16.cfg",
     {clang_vx::Version::V16, clang_vx::Version::V17, clang_vx::Version::V18,
      clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-17.cfg",
     {clang_vx::Version::V17, clang_vx::Version::V18, clang_vx::Version::V19,
      clang_vx::Version::V20}},
    {"webkit-18.cfg",
     {clang_vx::Version::V18, clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-19.cfg", {clang_vx::Version::V19, clang_vx::Version::V20}},
    {"webkit-20.cfg", {clang_vx::Version::V20}}};

} // namespace

TEST_CASE("getCompatibleVersion", "[clang-format-config-migrate]") {
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

TEST_CASE("updateEnum", "[clang-format-config-migrate]") {
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

      style4_old.IncludeCategories.clear();
      style4_new.IncludeCategories.clear();
      style5_old.IncludeCategories.clear();
      style5_new.IncludeCategories.clear();

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

      if (style == "chromium" || style == "google") {
        style5_old.IncludeCategories.clear();
        style5_new.IncludeCategories.clear();
        style6_old.IncludeCategories.clear();
        style6_new.IncludeCategories.clear();
      }

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

      style6_old.RawStringFormats.clear();
      style6_new.RawStringFormats.clear();
      style7_old.RawStringFormats.clear();
      style7_new.RawStringFormats.clear();

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

    if (clang_v9::FormatStyle style9_old; clang_v9::getPredefinedStyle(
            style, clang_v9::FormatStyle::LanguageKind::LK_Cpp, &style9_old)) {
      clang_v10::FormatStyle style10_new;
      clang_update_v10::update<clang_vx::Update::UPGRADE>(style9_old,
                                                          style10_new, style);

      clang_v10::FormatStyle style10_old;
      clang_v10::getPredefinedStyle(
          style, clang_v10::FormatStyle::LanguageKind::LK_Cpp, &style10_old);
      clang_v9::FormatStyle style9_new;
      clang_update_v10::update<clang_vx::Update::DOWNGRADE>(style9_new,
                                                            style10_old, style);

      style9_old.IncludeStyle.IncludeBlocks =
          clang_v9::IncludeStyle::IBS_Preserve;
      style9_new.IncludeStyle.IncludeBlocks =
          clang_v9::IncludeStyle::IBS_Preserve;
      style10_old.IncludeStyle.IncludeBlocks =
          clang_v10::IncludeStyle::IBS_Preserve;
      style10_new.IncludeStyle.IncludeBlocks =
          clang_v10::IncludeStyle::IBS_Preserve;
      style9_old.Standard = clang_v9::FormatStyle::LanguageStandard::LS_Auto;
      style9_new.Standard = clang_v9::FormatStyle::LanguageStandard::LS_Auto;
      style10_old.Standard = clang_v10::FormatStyle::LanguageStandard::LS_Auto;
      style10_new.Standard = clang_v10::FormatStyle::LanguageStandard::LS_Auto;

      if (style == "webkit") {
        style9_old.AllowShortBlocksOnASingleLine = false;
        style9_new.AllowShortBlocksOnASingleLine = false;
        style10_old.AllowShortBlocksOnASingleLine =
            clang_v10::FormatStyle::ShortBlockStyle::SBS_Never;
        style10_new.AllowShortBlocksOnASingleLine =
            clang_v10::FormatStyle::ShortBlockStyle::SBS_Never;
      }

      REQUIRE(style9_old == style9_new);
      REQUIRE(style10_old == style10_new);
    }

    if (clang_v10::FormatStyle style10_old; clang_v10::getPredefinedStyle(
            style, clang_v10::FormatStyle::LanguageKind::LK_Cpp,
            &style10_old)) {
      clang_v11::FormatStyle style11_new;
      clang_update_v11::update<clang_vx::Update::UPGRADE>(style10_old,
                                                          style11_new, style);

      clang_v11::FormatStyle style11_old;
      clang_v11::getPredefinedStyle(
          style, clang_v11::FormatStyle::LanguageKind::LK_Cpp, &style11_old);
      clang_v10::FormatStyle style10_new;
      clang_update_v11::update<clang_vx::Update::DOWNGRADE>(style10_new,
                                                            style11_old, style);

      if (style == "chromium" || style == "google") {
        style10_old.RawStringFormats.clear();
        style10_new.RawStringFormats.clear();
        style11_old.RawStringFormats.clear();
        style11_new.RawStringFormats.clear();
      }

      REQUIRE(style10_old == style10_new);
      REQUIRE(style11_old == style11_new);
    }

    if (clang_v11::FormatStyle style11_old; clang_v11::getPredefinedStyle(
            style, clang_v11::FormatStyle::LanguageKind::LK_Cpp,
            &style11_old)) {
      clang_v12::FormatStyle style12_new;
      clang_update_v12::update<clang_vx::Update::UPGRADE>(style11_old,
                                                          style12_new, style);

      clang_v12::FormatStyle style12_old;
      clang_v12::getPredefinedStyle(
          style, clang_v12::FormatStyle::LanguageKind::LK_Cpp, &style12_old);
      clang_v11::FormatStyle style11_new;
      clang_update_v12::update<clang_vx::Update::DOWNGRADE>(style11_new,
                                                            style12_old, style);

      style11_old.WhitespaceSensitiveMacros.clear();
      style11_new.WhitespaceSensitiveMacros.clear();
      style12_old.WhitespaceSensitiveMacros.clear();
      style12_new.WhitespaceSensitiveMacros.clear();

      REQUIRE(style11_old == style11_new);
      REQUIRE(style12_old == style12_new);
    }

    if (clang_v12::FormatStyle style12_old; clang_v12::getPredefinedStyle(
            style, clang_v12::FormatStyle::LanguageKind::LK_Cpp,
            &style12_old)) {
      clang_v13::FormatStyle style13_new;
      clang_update_v13::update<clang_vx::Update::UPGRADE>(style12_old,
                                                          style13_new, style);

      clang_v13::FormatStyle style13_old;
      clang_v13::getPredefinedStyle(
          style, clang_v13::FormatStyle::LanguageKind::LK_Cpp, &style13_old);
      clang_v12::FormatStyle style12_new;
      clang_update_v13::update<clang_vx::Update::DOWNGRADE>(style12_new,
                                                            style13_old, style);

      if (style == "chromium" || style == "google") {
        style12_old.RawStringFormats.clear();
        style12_new.RawStringFormats.clear();
        style13_old.RawStringFormats.clear();
        style13_new.RawStringFormats.clear();
      }
      style12_old.SortIncludes = false;
      style12_new.SortIncludes = false;
      style13_old.SortIncludes =
          clang_v13::FormatStyle::SortIncludesOptions::SI_Never;
      style13_new.SortIncludes =
          clang_v13::FormatStyle::SortIncludesOptions::SI_Never;

      REQUIRE(style12_old == style12_new);
      REQUIRE(style13_old == style13_new);
    }

    if (clang_v13::FormatStyle style13_old; clang_v13::getPredefinedStyle(
            style, clang_v13::FormatStyle::LanguageKind::LK_Cpp,
            &style13_old)) {
      clang_v14::FormatStyle style14_new;
      clang_update_v14::update<clang_vx::Update::UPGRADE>(style13_old,
                                                          style14_new, style);

      clang_v14::FormatStyle style14_old;
      clang_v14::getPredefinedStyle(
          style, clang_v14::FormatStyle::LanguageKind::LK_Cpp, &style14_old);
      clang_v13::FormatStyle style13_new;
      clang_update_v14::update<clang_vx::Update::DOWNGRADE>(style13_new,
                                                            style14_old, style);

      style13_old.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
      style13_new.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
      style14_old.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
      style14_new.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
      style13_old.AllowAllConstructorInitializersOnNextLine = false;
      style13_new.AllowAllConstructorInitializersOnNextLine = false;
      style14_old.AllowAllConstructorInitializersOnNextLine = false;
      style14_new.AllowAllConstructorInitializersOnNextLine = false;

      REQUIRE(style13_old == style13_new);
      REQUIRE(style14_old == style14_new);
    }

    if (clang_v14::FormatStyle style14_old; clang_v14::getPredefinedStyle(
            style, clang_v14::FormatStyle::LanguageKind::LK_Cpp,
            &style14_old)) {
      clang_v15::FormatStyle style15_new;
      clang_update_v15::update<clang_vx::Update::UPGRADE>(style14_old,
                                                          style15_new, style);

      clang_v15::FormatStyle style15_old;
      clang_v15::getPredefinedStyle(
          style, clang_v15::FormatStyle::LanguageKind::LK_Cpp, &style15_old);
      clang_v14::FormatStyle style14_new;
      clang_update_v15::update<clang_vx::Update::DOWNGRADE>(style14_new,
                                                            style15_old, style);

      style14_old.IndentRequires = false;
      style14_new.IndentRequires = false;
      style15_old.IndentRequiresClause = false;
      style15_new.IndentRequiresClause = false;

      style14_old.AlignConsecutiveMacros =
          clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_None;
      style14_old.AlignConsecutiveAssignments =
          clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_None;
      style14_old.AlignConsecutiveBitFields =
          clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_None;
      style14_old.AlignConsecutiveDeclarations =
          clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_None;
      style14_new.AlignConsecutiveMacros =
          clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_None;
      style14_new.AlignConsecutiveAssignments =
          clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_None;
      style14_new.AlignConsecutiveBitFields =
          clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_None;
      style14_new.AlignConsecutiveDeclarations =
          clang_v14::FormatStyle::AlignConsecutiveStyle::ACS_None;
      style15_old.AlignConsecutiveMacros = {};
      style15_old.AlignConsecutiveAssignments = {};
      style15_old.AlignConsecutiveBitFields = {};
      style15_old.AlignConsecutiveDeclarations = {};
      style15_new.AlignConsecutiveMacros = {};
      style15_new.AlignConsecutiveAssignments = {};
      style15_new.AlignConsecutiveBitFields = {};
      style15_new.AlignConsecutiveDeclarations = {};

      REQUIRE(style14_old == style14_new);
      REQUIRE(style15_old == style15_new);
    }

    if (clang_v15::FormatStyle style15_old; clang_v15::getPredefinedStyle(
            style, clang_v15::FormatStyle::LanguageKind::LK_Cpp,
            &style15_old)) {
      clang_v16::FormatStyle style16_new;
      clang_update_v16::update<clang_vx::Update::UPGRADE>(style15_old,
                                                          style16_new, style);

      clang_v16::FormatStyle style16_old;
      clang_v16::getPredefinedStyle(
          style, clang_v16::FormatStyle::LanguageKind::LK_Cpp, &style16_old);
      clang_v15::FormatStyle style15_new;
      clang_update_v16::update<clang_vx::Update::DOWNGRADE>(style15_new,
                                                            style16_old, style);

      style15_old.WhitespaceSensitiveMacros.clear();
      style15_new.WhitespaceSensitiveMacros.clear();
      style16_old.WhitespaceSensitiveMacros.clear();
      style16_new.WhitespaceSensitiveMacros.clear();

      REQUIRE(style15_old == style15_new);
      REQUIRE(style16_old == style16_new);
    }

    if (clang_v16::FormatStyle style16_old; clang_v16::getPredefinedStyle(
            style, clang_v16::FormatStyle::LanguageKind::LK_Cpp,
            &style16_old)) {
      clang_v17::FormatStyle style17_new;
      clang_update_v17::update<clang_vx::Update::UPGRADE>(style16_old,
                                                          style17_new, style);

      clang_v17::FormatStyle style17_old;
      clang_v17::getPredefinedStyle(
          style, clang_v17::FormatStyle::LanguageKind::LK_Cpp, &style17_old);
      clang_v16::FormatStyle style16_new;
      clang_update_v17::update<clang_vx::Update::DOWNGRADE>(style16_new,
                                                            style17_old, style);

      REQUIRE(style16_old == style16_new);
      REQUIRE(style17_old == style17_new);
    }

    if (clang_v17::FormatStyle style17_old; clang_v17::getPredefinedStyle(
            style, clang_v17::FormatStyle::LanguageKind::LK_Cpp,
            &style17_old)) {
      clang_v18::FormatStyle style18_new;
      clang_update_v18::update<clang_vx::Update::UPGRADE>(style17_old,
                                                          style18_new, style);

      clang_v18::FormatStyle style18_old;
      clang_v18::getPredefinedStyle(
          style, clang_v18::FormatStyle::LanguageKind::LK_Cpp, &style18_old);
      clang_v17::FormatStyle style17_new;
      clang_update_v18::update<clang_vx::Update::DOWNGRADE>(style17_new,
                                                            style18_old, style);

      style17_old.BreakAfterAttributes =
          clang_v17::FormatStyle::AttributeBreakingStyle::ABS_Leave;
      style17_new.BreakAfterAttributes =
          clang_v17::FormatStyle::AttributeBreakingStyle::ABS_Leave;
      style18_old.BreakAfterAttributes =
          clang_v18::FormatStyle::AttributeBreakingStyle::ABS_Leave;
      style18_new.BreakAfterAttributes =
          clang_v18::FormatStyle::AttributeBreakingStyle::ABS_Leave;

      REQUIRE(style17_old == style17_new);
      REQUIRE(style18_old == style18_new);
    }

    if (clang_v18::FormatStyle style18_old; clang_v18::getPredefinedStyle(
            style, clang_v18::FormatStyle::LanguageKind::LK_Cpp,
            &style18_old)) {
      clang_v19::FormatStyle style19_new;
      clang_update_v19::update<clang_vx::Update::UPGRADE>(style18_old,
                                                          style19_new, style);

      clang_v19::FormatStyle style19_old;
      clang_v19::getPredefinedStyle(
          style, clang_v19::FormatStyle::LanguageKind::LK_Cpp, &style19_old);
      clang_v18::FormatStyle style18_new;
      clang_update_v19::update<clang_vx::Update::DOWNGRADE>(style18_new,
                                                            style19_old, style);

      if (style == "clang-format") {
        style18_old.IntegerLiteralSeparator.Decimal = 0;
        style18_new.IntegerLiteralSeparator.Decimal = 0;
        style19_old.IntegerLiteralSeparator.Decimal = 0;
        style19_new.IntegerLiteralSeparator.Decimal = 0;
        style18_old.IntegerLiteralSeparator.DecimalMinDigits = 0;
        style18_new.IntegerLiteralSeparator.DecimalMinDigits = 0;
        style19_old.IntegerLiteralSeparator.DecimalMinDigits = 0;
        style19_new.IntegerLiteralSeparator.DecimalMinDigits = 0;
        style18_old.RemoveSemicolon = true;
        style18_new.RemoveSemicolon = true;
        style19_old.RemoveSemicolon = true;
        style19_new.RemoveSemicolon = true;
      }

      REQUIRE(style18_old == style18_new);
      REQUIRE(style19_old == style19_new);
    }

    if (clang_v19::FormatStyle style19_old; clang_v19::getPredefinedStyle(
            style, clang_v19::FormatStyle::LanguageKind::LK_Cpp,
            &style19_old)) {
      clang_v20::FormatStyle style20_new;
      clang_update_v20::update<clang_vx::Update::UPGRADE>(style19_old,
                                                          style20_new, style);

      clang_v20::FormatStyle style20_old;
      clang_v20::getPredefinedStyle(
          style, clang_v20::FormatStyle::LanguageKind::LK_Cpp, &style20_old);
      clang_v19::FormatStyle style19_new;
      clang_update_v20::update<clang_vx::Update::DOWNGRADE>(style19_new,
                                                            style20_old, style);

      if (style == "gnu") {
        style19_old.Standard =
            clang_v19::FormatStyle::LanguageStandard::LS_Auto;
        style19_new.Standard =
            clang_v19::FormatStyle::LanguageStandard::LS_Auto;
        style20_old.Standard =
            clang_v20::FormatStyle::LanguageStandard::LS_Auto;
        style20_new.Standard =
            clang_v20::FormatStyle::LanguageStandard::LS_Auto;
      }

      REQUIRE(style19_old == style19_new);
      REQUIRE(style20_old == style20_new);
    }
  }
}

TEST_CASE("showMigration", "[clang-format-config-migrate]") {
  auto max_value =
      *std::max_element(magic_enum::enum_values<clang_vx::Version>().begin(),
                        magic_enum::enum_values<clang_vx::Version>().end());
  for (std::string_view style : styles) {
    bool skip_first = true;
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

        for (bool skip : {false, true}) {
          if (version != max_value) {
            std::cout << "Update: " << filename << ", skip: " << std::boolalpha
                      << skip << std::noboolalpha << "\n"
                      << updateTo(version,
                                  static_cast<clang_vx::Version>(
                                      static_cast<size_t>(version) + 1),
                                  ss.str(), std::string{style}, skip)
                      << "\n";
          }
          if (!skip_first) {
            std::cout << "Downgrade: " << filename
                      << ", skip: " << std::boolalpha << skip
                      << std::noboolalpha << "\n"
                      << downgradeTo(version,
                                     static_cast<clang_vx::Version>(
                                         static_cast<size_t>(version) - 1),
                                     ss.str(), std::string{style}, skip)
                      << "\n";
          }
        }
        skip_first = false;
      }
    }
  }
}
