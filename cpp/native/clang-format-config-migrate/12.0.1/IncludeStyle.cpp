//===--- IncludeStyle.cpp - Style of C++ #include directives -----*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IncludeStyle.h"

using clang_v12::IncludeStyle;

namespace llvm {
namespace yaml {

void MappingTraits<IncludeStyle::IncludeCategory>::mapping(
    IO &IO, IncludeStyle::IncludeCategory &Category) {
  IO.mapOptional("Regex", Category.Regex);
  IO.mapOptional("Priority", Category.Priority);
  IO.mapOptional("SortPriority", Category.SortPriority);
  IO.mapOptional("CaseSensitive", Category.RegexIsCaseSensitive);
}

void ScalarEnumerationTraits<IncludeStyle::IncludeBlocksStyle>::enumeration(
    IO &IO, IncludeStyle::IncludeBlocksStyle &Value) {
  IO.enumCase(Value, "Preserve", IncludeStyle::IBS_Preserve);
  IO.enumCase(Value, "Merge", IncludeStyle::IBS_Merge);
  IO.enumCase(Value, "Regroup", IncludeStyle::IBS_Regroup);
}

} // namespace yaml
} // namespace llvm
