#include <clang/Format/Format.h>
#include <clang/Tooling/Tooling.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <string>

namespace web_demangler {

std::string Format(const std::string &code) {
  clang::format::FormatStyle style = clang::format::getGoogleStyle(
      clang::format::FormatStyle::LanguageKind::LK_Cpp);

  clang::tooling::Range range(0, code.size());

  clang::format::FormattingAttemptStatus status;

  clang::tooling::Replacements replacements =
      clang::format::reformat(style, code, {range}, "<stdin>", &status);

  if (status.FormatComplete) {
    llvm::Expected<std::string> formattedCode =
        clang::tooling::applyAllReplacements(code, replacements);

    if (formattedCode) {
      return formattedCode.get();
    }
  }
  return code;
}

void RegisterFormatStyle() {
  emscripten::enum_<clang::format::FormatStyle::BracketAlignmentStyle>(
      "BracketAlignmentStyle")
      .value("Align",
             clang::format::FormatStyle::BracketAlignmentStyle::BAS_Align)
      .value("DontAlign",
             clang::format::FormatStyle::BracketAlignmentStyle::BAS_DontAlign)
      .value("AlwaysBreak",
             clang::format::FormatStyle::BracketAlignmentStyle::BAS_AlwaysBreak)
      .value(
          "BlockIndent",
          clang::format::FormatStyle::BracketAlignmentStyle::BAS_BlockIndent);

  emscripten::enum_<clang::format::FormatStyle::ArrayInitializerAlignmentStyle>(
      "ArrayInitializerAlignmentStyle")
      .value(
          "Left",
          clang::format::FormatStyle::ArrayInitializerAlignmentStyle::AIAS_Left)
      .value("Right", clang::format::FormatStyle::
                          ArrayInitializerAlignmentStyle::AIAS_Right)
      .value("None", clang::format::FormatStyle::
                         ArrayInitializerAlignmentStyle::AIAS_None);

  emscripten::class_<clang::format::FormatStyle::AlignConsecutiveStyle>(
      "AlignConsecutiveStyle")
      .property("Enabled",
                &clang::format::FormatStyle::AlignConsecutiveStyle::Enabled)
      .property(
          "AcrossEmptyLines",
          &clang::format::FormatStyle::AlignConsecutiveStyle::AcrossEmptyLines)
      .property(
          "AcrossComments",
          &clang::format::FormatStyle::AlignConsecutiveStyle::AcrossComments)
      .property(
          "AlignCompound",
          &clang::format::FormatStyle::AlignConsecutiveStyle::AlignCompound)
      .property("AlignFunctionPointers",
                &clang::format::FormatStyle::AlignConsecutiveStyle::
                    AlignFunctionPointers)
      .property(
          "PadOperators",
          &clang::format::FormatStyle::AlignConsecutiveStyle::PadOperators);

  emscripten::class_<
      clang::format::FormatStyle::ShortCaseStatementsAlignmentStyle>(
      "ShortCaseStatementsAlignmentStyle")
      .property("Enabled", &clang::format::FormatStyle::
                               ShortCaseStatementsAlignmentStyle::Enabled)
      .property("AcrossEmptyLines",
                &clang::format::FormatStyle::ShortCaseStatementsAlignmentStyle::
                    AcrossEmptyLines)
      .property("AcrossComments",
                &clang::format::FormatStyle::ShortCaseStatementsAlignmentStyle::
                    AcrossComments)
      .property("AlignCaseArrows",
                &clang::format::FormatStyle::ShortCaseStatementsAlignmentStyle::
                    AlignCaseArrows)
      .property("AlignCaseColons",
                &clang::format::FormatStyle::ShortCaseStatementsAlignmentStyle::
                    AlignCaseColons);

  emscripten::class_<clang::format::FormatStyle>("FormatStyle")
      .property("InheritsParentConfig",
                &clang::format::FormatStyle::InheritsParentConfig)
      .property("AccessModifierOffset",
                &clang::format::FormatStyle::AccessModifierOffset)
      .property("AlignAfterOpenBracket",
                &clang::format::FormatStyle::AlignAfterOpenBracket)
      .property("AlignArrayOfStructures",
                &clang::format::FormatStyle::AlignArrayOfStructures)
      .property("AlignConsecutiveMacros",
                &clang::format::FormatStyle::AlignConsecutiveMacros,
                emscripten::return_value_policy::reference())
      .property("AlignConsecutiveAssignments",
                &clang::format::FormatStyle::AlignConsecutiveAssignments,
                emscripten::return_value_policy::reference())
      .property("AlignConsecutiveBitFields",
                &clang::format::FormatStyle::AlignConsecutiveBitFields,
                emscripten::return_value_policy::reference())
      .property("AlignConsecutiveDeclarations",
                &clang::format::FormatStyle::AlignConsecutiveDeclarations,
                emscripten::return_value_policy::reference())
      .property(
          "AlignConsecutiveShortCaseStatements",
          &clang::format::FormatStyle::AlignConsecutiveShortCaseStatements,
          emscripten::return_value_policy::reference());
}

} // namespace web_demangler

EMSCRIPTEN_BINDINGS(web_formatter) {
  emscripten::function("formatter", &web_demangler::Format);
  web_demangler::RegisterFormatStyle();
}
