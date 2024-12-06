#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang-tidy/ClangTidyCheck.h"
#include "clang-tidy/ClangTidyOptions.h"
#include "clang-tidy/Readability/IdentifierNamingCheck.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/YAMLParser.h"
#include <sstream>
#include <string>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tidy;

namespace web_clang_tidy {

namespace {

class ApplyFixAction : public ASTFrontendAction {
public:
    ApplyFixAction(const ClangTidyOptions &Options, Rewriter &Rewriter)
        : Options(Options), Rewriter(Rewriter) {}

    void EndSourceFileAction() override {
        auto &Context = getCompilerInstance().getASTContext();
        IdentifierNamingCheck Check("readability-identifier-naming", &Options);

        Check.registerMatchers(&Finder);
        Finder.matchAST(Context);

        for (const auto &Diag : Check.Diagnostics) {
            for (const auto &FixIt : Diag.FixIts) {
                auto Err = Rewriter.ReplaceText(FixIt.RemoveRange, FixIt.CodeToInsert);
                if (Err)
                    llvm::errs() << "Error applying fix: " << Err << "\n";
            }
        }
    }

private:
    MatchFinder Finder;
    ClangTidyOptions Options;
    Rewriter &Rewriter;
};

ClangTidyOptions loadOptions() {
    std::string yaml_options = R"(
Checks: -*,readability-identifier-naming
FormatStyle: file
CheckOptions:
    - key: readability-identifier-naming.ClassCase
      value: CamelCase
    - key: readability-identifier-naming.ClassConstantCase
      value: CamelCase
    - key: readability-identifier-naming.ClassConstantPrefix
      value: k
    - key: readability-identifier-naming.EnumCase
      value: CamelCase
    - key: readability-identifier-naming.EnumConstantCase
      value: CamelCase
    - key: readability-identifier-naming.EnumConstantPrefix
      value: k
    - key: readability-identifier-naming.FunctionCase
      value: CamelCase
    - key: readability-identifier-naming.GlobalConstantCase
      value: CamelCase
    - key: readability-identifier-naming.GlobalConstantPrefix
      value: k
    - key: readability-identifier-naming.GlobalConstantPointerCase
      value: CamelCase
    - key: readability-identifier-naming.GlobalConstantPointerPrefix
      value: k
    - key: readability-identifier-naming.MethodCase
      value: CamelCase
    - key: readability-identifier-naming.NamespaceCase
      value: lower_case
    - key: readability-identifier-naming.ParameterCase
      value: camelBack
    - key: readability-identifier-naming.PrivateMemberCase
      value: camelBack
    - key: readability-identifier-naming.PrivateMemberSuffix
      value: _
    - key: readability-identifier-naming.PublicMemberCase
      value: camelBack
    - key: readability-identifier-naming.StaticConstantCase
      value: CamelCase
    - key: readability-identifier-naming.StaticConstantPrefix
      value: k
    - key: readability-identifier-naming.TemplateParameterCase
      value: CamelCase
    - key: readability-identifier-naming.TypeAliasCase
      value: CamelCase
    - key: readability-identifier-naming.TypedefCase
      value: CamelCase
    - key: readability-identifier-naming.UnionCase
      value: CamelCase
    - key: readability-identifier-naming.VariableCase
      value: camelBack
)";
    llvm::yaml::Input YamlInput(yaml_options);
    ClangTidyOptions Options;
    YamlInput >> Options;
    return Options;
}


} // namespace

int clang_tidy_readability_identifier_naming() {
    // Charger les options depuis le fichier config.yaml
    auto Options = loadOptionsFromFile("config.yaml");

    // Code source à analyser
    std::string Code = R"(
        void my_function() {
            int myVar = 42;
        }
    )";

    // Configurer les composants nécessaires pour l'analyse
    CompilerInstance Compiler;
    Compiler.createDiagnostics();
    LangOptions LangOpts;
    LangOpts.CPlusPlus = true;
    Compiler.getLangOpts() = LangOpts;

    SourceManager Sources(Compiler.getDiagnostics(), Compiler.getFileManager());
    Rewriter Rewriter(Sources, LangOpts);

    // Charger le code en mémoire
    auto Buffer = llvm::MemoryBuffer::getMemBuffer(Code, "input.cc");
    auto FID = Sources.createFileID(std::move(Buffer));
    Sources.setMainFileID(FID);

    // Lancer l'analyse et les corrections
    ApplyFixAction Action(Options, Rewriter);
    Action.BeginSourceFile(Compiler.getLangOpts(), &Sources);
    Action.ExecuteAction();
    Action.EndSourceFile();

    // Afficher le code corrigé
    llvm::errs() << "--- Corrected Code ---\n";
    Rewriter.getEditBuffer(FID).write(llvm::outs());
    return 0;
}

} // namespace web_demangler

EMSCRIPTEN_BINDINGS(web_demangler) {
  emscripten::function("web_demangle", &web_demangler::demangle);
}
