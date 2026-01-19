# Add new version

## Get source

Get the version of the new LLVM version with 3 digits (i.e. 20.1.6) and create a folder with this name in current path (`cpp/native/clang-format-config-migrate`).

In this new folder, copy the following files from LLVM project (`cpp/third_party/llvm`):

  - clang/include/clang/Format/Format.h
  - clang/include/clang/Tooling/Inclusions/IncludeStyle.h
  - clang/lib/Format/Format.cpp
  - clang/lib/Tooling/Inclusions/IncludeStyle.cpp

## Adjust code

Keep only the main struct of each header and helpers that are needed to serialize to Yaml format.

When editing, it's strongly recommanded to edit the file with a GUI editor (i.e. WinMerge) that allow real time comparision with its latest version that has been integrated in `clang-format-config-migrate` project.

Keep up to date the next paragraph to be sure to not forget something.

### IncludeStyle.h

Replace main `#ifdef` by `#pragma once`.

Replace the namespace around `struct IncludeStyle` from `clang::tooling` to `clang_vXX` with XX the number of the LLVM version (i.e. 20). You don't need to write the full version number if you don't intend to add another version with the same major version.

Replace all `clang::tooling::IncludeStyle` to `clang_vXX::IncludeStyle`.

Apply `clang-format`.

### IncludeStyle.cpp

Replace `#include "clang/Tooling/Inclusions/IncludeStyle.h"` by `#include "IncludeStyle.h"`.

Replace `using clang::tooling::IncludeStyle;` by `using clang_vXX::IncludeStyle;`

Apply `clang-format`.

### Format.h

Replace main `#ifdef` by `#pragma once`.

Remove all headers from LLVM and use only local header. So keep only `#include "IncludeStyle.h"` and remove:

```cpp
#include "clang/Basic/LangOptions.h"
#include "clang/Tooling/Core/Replacement.h"
#include "clang/Tooling/Inclusions/IncludeStyle.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/SourceMgr.h"

namespace llvm {
namespace vfs {
class FileSystem;
}
} // namespace llvm
```

Add missing header `#include <vector>`

Replace the namespace around `struct FormatStyle` from `clang::tooling` to `clang_vXX` with XX the number of the LLVM version (i.e. 20). You don't need to write the full version number if you don't intend to add another version with the same major version.

In `struct FormatStyle`, replace `tooling::IncludeStyle IncludeStyle;` by `clang_vXX::IncludeStyle IncludeStyle;`

In prototype `getPredefinedStyle`, explicity set `llvm` namespace to `llvm::StringRef`.

After `getPredefinedStyle`, add a new prototype `std::vector<std::string> getStyleNames();`. This function is needed to know which style is supported by this version of Format.

You need to remove other parameters that is not needed to serialize to Yaml. Replace prototypes `std::error_code parseConfiguration(llvm::MemoryBufferRef Config, FormatStyle *Style, bool AllowUnknownOptions = false, llvm::SourceMgr::DiagHandlerTy DiagHandler = nullptr, void *DiagHandlerCtx = nullptr)` to `std::error_code parseConfiguration(llvm::MemoryBufferRef Config, FormatStyle *Style)`

And replace

```cpp
inline std::error_code parseConfiguration(StringRef Config, FormatStyle *Style,
                                          bool AllowUnknownOptions = false) {
  return parseConfiguration(llvm::MemoryBufferRef(Config, "YAML"), Style,
                            AllowUnknownOptions);
}
```

to

```cpp
inline std::error_code parseConfiguration(const std::string &Config,
                                          FormatStyle *Style) {
  return parseConfiguration(llvm::MemoryBufferRef(Config, "YAML"), Style);
}
```

and

```cpp
  friend std::error_code
  parseConfiguration(llvm::MemoryBufferRef Config, FormatStyle *Style,
                     bool AllowUnknownOptions,
                     llvm::SourceMgr::DiagHandlerTy DiagHandler,
                     void *DiagHandlerCtxt, bool IsDotHFile);
```

to

```
  friend std::error_code parseConfiguration(llvm::MemoryBufferRef Config,
                                            FormatStyle *Style);
```

Replace prototype `std::string configurationAsText(const FormatStyle &Style);` to `std::string configurationAsText(const FormatStyle &Style, const std::string &DefaultStyleName, bool SkipSameValue);`. More parameters are needed for migration.

Remove all prototype after `configurationAsText`.

Replace `clang::format::ParseError` by `clang_vXX::ParseError`.

Apply `clang-format`.

### Format.cpp

This file needs lots of changes. You need to add code to support optional output if value is the same than default value.

Cleanup headers. Replace

```cpp
#include "clang/Format/Format.h"
#include "DefinitionBlockSeparator.h"
#include "IntegerLiteralSeparatorFixer.h"
#include "NamespaceEndCommentsFixer.h"
#include "ObjCPropertyAttributeOrderFixer.h"
#include "QualifierAlignmentFixer.h"
#include "SortJavaScriptImports.h"
#include "UnwrappedLineFormatter.h"
#include "UsingDeclarationsSorter.h"
#include "clang/Tooling/Inclusions/HeaderIncludes.h"
#include "llvm/ADT/Sequence.h"
```

to

```cpp
#include "Format.h"
#include "../Format.h"
#include "clang/Basic/OperatorPrecedence.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Debug.h"
#include <set>
```

Replace `using clang::format::FormatStyle;` by `using clang_vXX::FormatStyle;`.

Add

```cpp
namespace prec = clang::prec;
namespace tok = clang::tok;
```

Replace all `IO.mapOptional(` by `clang_vx::IoMapOptional<FormatStyle>(IO, ` (`clang_vx` must not be replaced by the version of clang) except:

  - if `template <> struct MappingTraits` is used in a `std::vector` (i.e. `RawStringFormat`),
  - if you are in condition `!IO.outputting()` because you will only read, not write.

In `template <> struct MappingTraits<FormatStyle>`, replace the `BasedOnStyle` field from:

In `template <> struct MappingTraits<FormatStyle::SpacesInLineComment>`, replace

```cpp
    clang_vx::IoMapOptional<FormatStyle>(IO, "Maximum", signedMaximum);
```

by

```cpp
    clang_vx::IoMapOptional<FormatStyle>(IO, "Maximum", signedMaximum,
                                         Space.Maximum);
```


```cpp
    StringRef BasedOnStyle;
    if (IO.outputting()) {
      StringRef Styles[] = {"LLVM",   "Google", "Chromium",  "Mozilla",
                            "WebKit", "GNU",    "Microsoft", "clang-format"};
      for (StringRef StyleName : Styles) {
        FormatStyle PredefinedStyle;
        if (getPredefinedStyle(StyleName, Style.Language, &PredefinedStyle) &&
            Style == PredefinedStyle) {
          BasedOnStyle = StyleName;
          break;
        }
      }
    } else {
      IO.mapOptional("BasedOnStyle", BasedOnStyle);
      if (!BasedOnStyle.empty()) {
        FormatStyle::LanguageKind OldLanguage = Style.Language;
        FormatStyle::LanguageKind Language =
            ((FormatStyle *)IO.getContext())->Language;
        if (!getPredefinedStyle(BasedOnStyle, Language, &Style)) {
          IO.setError(Twine("Unknown value for BasedOnStyle: ", BasedOnStyle));
          return;
        }
        Style.Language = OldLanguage;
      }
    }
```

by

```cpp
    std::string BasedOnStyle;
    if (IO.outputting()) {
      clang_vx::OutputDiffOnly<FormatStyle> &out =
          *static_cast<clang_vx::OutputDiffOnly<FormatStyle> *>(
              IO.getContext());
      if (out.getDefaultStyle()) {
        for (const std::string &StyleName : clang_vXX::getStyleNames()) {
          FormatStyle PredefinedStyle;
          if (clang_vXX::getPredefinedStyle(StyleName, Style.Language,
                                            &PredefinedStyle) &&
              *out.getDefaultStyle() == PredefinedStyle) {
            BasedOnStyle = StyleName;
            break;
          }
        }
        IO.mapOptional("BasedOnStyle", BasedOnStyle);
      }
    } else {
      IO.mapOptional("BasedOnStyle", BasedOnStyle);
      for (const std::string &StyleName : clang_vXX::getStyleNames()) {
        FormatStyle PredefinedStyle;
        if (clang_vXX::getPredefinedStyle(StyleName, Style.Language,
                                          &PredefinedStyle) &&
            Style == PredefinedStyle) {
          BasedOnStyle = StyleName;
          break;
        }
      }
    }
```

Replace

```cpp
    const bool IsGoogleOrChromium = BasedOnStyle.equals_insensitive("google") ||
                                    BasedOnStyle.equals_insensitive("chromium");
```

by

```cpp
    const bool IsGoogleOrChromium =
        BasedOnStyle == "google" || BasedOnStyle == "chromium";
```

Remove the function `make_string_error`. It will be unused after removing uneeded classes.

Add function `getStyleNames`. i.e.

```cpp
std::vector<std::string> getStyleNames() {
  return {"chromium",  "clang-format", "gnu",  "google", "llvm",
          "microsoft", "mozilla",      "none", "webkit"};
}
```

Replace `LeftRightQualifierAlignmentFixer::getTokenFromQualifier` by `clang_vx::getTokenFromQualifier`.

Adjust parseConfiguration by removing parameters and not allowing unknown keys.

Replace configurationAsText by

```cpp
std::string configurationAsText(const FormatStyle &Style,
                                const std::string &DefaultStyleName,
                                bool SkipSameValue) {
  std::string Text;
  llvm::raw_string_ostream Stream(Text);
  FormatStyle DefaultStyle;
  // We use the same mapping method for input and output, so we need a
  // non-const reference here.
  FormatStyle NonConstStyle = Style;
  expandPresetsBraceWrapping(NonConstStyle);
  expandPresetsSpaceBeforeParens(NonConstStyle);
  expandPresetsSpacesInParens(NonConstStyle);
  std::optional<clang_vx::OutputDiffOnly<FormatStyle>> ctxt;
  if (!getPredefinedStyle(DefaultStyleName, Style.Language, &DefaultStyle)) {
    ctxt.emplace(nullptr, NonConstStyle, false);
  } else {
    ctxt.emplace(&DefaultStyle, NonConstStyle, SkipSameValue);
  }
  llvm::yaml::Output Output(Stream, &*ctxt);
  Output << NonConstStyle;

  return Stream.str();
}
```

Remove the huge anonymous namespace at the end of the file and functions after. It contains classes that is non needed.

Replace `tooling::IncludeStyle` by `clang_vXX::IncludeStyle`.

Replace namespace `format::clang` by `clang_vXX`.

In prototype `getPredefinedStyle`, explicity set `llvm` namespace to `llvm::StringRef`.

Apply `clang-format`.

## Build new files

### `cpp/native/CMakeLists.txt`

In `POST_BUILD` of `clang_format_parser`, add 

```cmake
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/XX.YY.ZZ/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/XX.YY.ZZ"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-vXX.YY.ZZ-binding.cpp.inc"
```

### `cpp/webassembly/CMakeLists.txt`

Add the following source files to `web_clang_format_config_migrate` target.

```cmake
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/XX.YY.ZZ/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/XX.YY.ZZ/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/XX.YY.ZZ/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/XX.YY.ZZ/IncludeStyle.h"
```

## Implement migration

### `cpp/native/clang-format-config-migrate/Format.h`

Add the new version `VXX` in class `Version`.

### `cpp/native/clang-format-config-migrate/Format.cpp`

Add:

```cpp
#include "XX.YY.ZZ/Format.h"
```

Compare function `getTokenFromQualifier` with the one from `clang/lib/Format/QualifierAlignmentFixer.cpp` in llvm project.

In `getCompatibleVersion` function, add `PARSE_CONFIG(XX);`.

In `getStyleNames` function, add:

```cpp
  case Version::VXX: {
    return clang_vXX::getStyleNames();
  }
```

### `cpp/native/clang-format-config-migrate/update.h`

Just add

```cpp
#include "XX.YY.ZZ/Format.h"

namespace clang_update_vXX {

template <clang_vx::Update Upgrade>
void update(clang_vPP::FormatStyle &prev, clang_vXX::FormatStyle &next,
            const std::string &style);

} // namespace clang_update_vXX
```

The first `FormatStyle &prev` is the previous version and `FormatStyle &next` is the next version. i.e.:

```cpp
clang_v19::FormatStyle &prev, clang_v20::FormatStyle &next
```

### `cpp/native/clang-format-config-migrate/update.cpp`

In this section, `XX` is the new version (i.e. 20) and `PP` is the previous version (i.e. 19).

This is the important file where you do the migration.

Add the new version of FormatStyle (`clang_vXX::FormatStyle`) in the typedef variant `AllFormatStyle`.

In `versionToFormatStyle` function, add:

```cpp
  case clang_vx::Version::VXX: {
    clang_vXX::FormatStyle fsXX;
    fsXX.Language = clang_vXX::FormatStyle::LanguageKind::LK_Cpp;
    if (!clang_vXX::getPredefinedStyle(
            default_style, clang_vXX::FormatStyle::LanguageKind::LK_Cpp,
            &fsXX)) {
      throw std::runtime_error("Unknown style " + default_style + ";");
    }
    std::error_code ec = clang_vXX::parseConfiguration(data, &fsXX);
    if (ec) {
      throw std::runtime_error("Failed to parse yaml config file vXX.\n" +
                               ec.message());
    }
    return fsXX;
  }
```

In `formatStyleToVersion` function, add:

```cpp
          [&default_style, skip_same_value](const clang_vXX::FormatStyle &fs) {
            return clang_vXX::configurationAsText(fs, default_style,
                                                  skip_same_value);
          }
```

In `updateTo` function, add:

```cpp
    case Version::VPP: {
      after = clang_vXX::FormatStyle{};
      clang_update_vXX::update<Update::UPGRADE>(
          std::get<clang_vPP::FormatStyle>(before),
          std::get<clang_vXX::FormatStyle>(after), style_i);
      break;
    }
```

In `downgradeTo` function, add:

```cpp
    case Version::VXX: {
      after = clang_vPP::FormatStyle{};
      clang_update_vXX::update<Update::DOWNGRADE>(
          std::get<clang_vPP::FormatStyle>(after),
          std::get<clang_vXX::FormatStyle>(before), style_i);
      break;
    }
```

Finally, duplicate the whole namespace `clang_update_vPP` to `clang_update_vXX`. And replace all `PP` by `XX` and all "`PP`-1" by `PP`.

Then you need to open `cpp/native/clang-format-config-migrate/XX.YY.ZZ/Format.h` and compare with `cpp/native/clang-format-config-migrate/PP.QQ.RR/Format.h`.

At first, all `frozen::unordered_map` in `clang_update_vXX` before update `function`.


It's important to compare for each field, in both old and new version :
  - the field exist only in new version. Use `NEW_FIELD`.
  - the field name and type are the same. Use `ASSIGN_MAGIC_ENUM` for enum type, `ASSIGN_SAME_FIELD` either.
  - the field is renamed and the type are the same. Use `RENAME_MAGIC_ENUM` for enum type, `RENAME_FIELD` either.
  - the type changes from number to enum. Use `SWITCH_TO_ENUM` if the name is the same, `RENAME_AND_SWITCH_TO_ENUM` either.
  - the type is a vector of struct, you need to implement the convertion. You have `ASSIGN_RAW_STRING_FORMAT` for `RawStringFormats` and `ASSIGN_INCLUDE_CATEGORY3` for `IncludeStyle.IncludeCategories`.

But sure to replace all `NEW_FIELD` of previous namespace by `ASSIGN_MAGIC_ENUM` or `ASSIGN_SAME_FIELD`.

At the current time, an old field has never disappeared in a new version.

`ASSIGN_MAGIC_ENUM` and `RENAME_MAGIC_ENUM` handle the case where an enum class is not the same. If a value doesn't exist while migrating, a warning is printed.

### `cpp/webassembly/web-clang-format-config-migrate.cpp`

Add the new version number to the binding of the `enum class Version`. Add `.value("VXX", clang_vx::Version::VXX)` at the end of the declaration of `emscripten::enum_<clang_vx::Version>("Version")`.

## Tests

You need to generate dataset with default config for each style. Launch command with clang-format-XX (here for webkit style):

`clang-format-XX -dump-config -style=webkit > cpp/tests/data/config-file-XX.cfg`

In `cpp/tests/CMakeLists.txt`, add in source file of `test_clang_format_config_migrate`:

```cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/XX.YY.ZZ/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/XX.YY.ZZ/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/XX.YY.ZZ/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/XX.YY.ZZ/IncludeStyle.h"
```

In `cpp/tests/clang-format-config-migrate.cpp`, append `clang_vx::Version::VXX` to all fields in `compatibilities` variable. For example, replace `clang_vx::Version::VPP` by `clang_vx::Version::VPP, clang_vx::Version::VXX` (EXCEPT for v6 than is not compatible with another version) and apply `clang-format`.

In `cpp/tests/clang-format-config-migrate.cpp`, add all files generated by previous command `clang-format -dump-config -style=...` in `compatibilities`: `{"STYLE-XX.cfg", {clang_vx::Version::VXX}}`.

At the end of the `updateEnum` test, add:

```cpp
    if (clang_vPP::FormatStyle stylePP_old; clang_vPP::getPredefinedStyle(
            style, clang_vPP::FormatStyle::LanguageKind::LK_Cpp,
            &stylePP_old)) {
      clang_vXX::FormatStyle styleXX_new;
      clang_update_vXX::update<clang_vx::Update::UPGRADE>(stylePP_old,
                                                          styleXX_new, style);

      clang_vXX::FormatStyle styleXX_old;
      clang_vXX::getPredefinedStyle(
          style, clang_vXX::FormatStyle::LanguageKind::LK_Cpp, &styleXX_old);
      clang_vPP::FormatStyle stylePP_new;
      clang_update_vXX::update<clang_vx::Update::DOWNGRADE>(stylePP_new,
                                                            styleXX_old, style);

      // Uncomment it only if you are sure (check Format.cpp) that the migration
      // of a default style will not work.
      // In this case, force old and new value to a random but same value.
      /*
      if (style == "clang-format") {
        stylePP_old.IntegerLiteralSeparator.Decimal = 0;
        stylePP_new.IntegerLiteralSeparator.Decimal = 0;
        styleXX_old.IntegerLiteralSeparator.Decimal = 0;
        styleXX_new.IntegerLiteralSeparator.Decimal = 0;
        stylePP_old.IntegerLiteralSeparator.DecimalMinDigits = 0;
        stylePP_new.IntegerLiteralSeparator.DecimalMinDigits = 0;
        styleXX_old.IntegerLiteralSeparator.DecimalMinDigits = 0;
        styleXX_new.IntegerLiteralSeparator.DecimalMinDigits = 0;
        stylePP_old.RemoveSemicolon = true;
        stylePP_new.RemoveSemicolon = true;
        styleXX_old.RemoveSemicolon = true;
        styleXX_new.RemoveSemicolon = true;
      }
      */

      REQUIRE(stylePP_old == stylePP_new);
      REQUIRE(styleXX_old == styleXX_new);
    }
```

Then run tests. If a test fails while `REQUIRE(stylePP_old == stylePP_new)` or while `REQUIRE(styleXX_old == styleXX_new)`, launch `gdb ./test_clang_format_config_migrate` from `build_tests_debug` folder, add a breakpoint `b clang-format-config-migrate.cpp:1700`, execute the loop to be sure to be on the right style. Then `print stylePP_old` and `print stylePP_new` and compare result.

## Angular project

In `angular/src/app/apps/clang-format-config-migrate.component.html`, add the new version `<option value="VXX">VXX</option>` to the list of new version.
