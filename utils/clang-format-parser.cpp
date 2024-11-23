#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Type.h>
#include <clang/Basic/Specifiers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>
#include <cstring>
#include <frozen/bits/elsa.h>
#include <frozen/bits/hash_string.h>
#include <frozen/unordered_map.h>
#include <fstream>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <memory>
#include <string>
#include <string_view>
#include <utility> // IWYU pragma: keep
#include <vector>

using clang::ASTContext;
using clang::CXXRecordDecl;
using clang::EnumDecl;
using clang::QualType;
using clang::RecursiveASTVisitor;

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

constexpr frozen::unordered_map<std::string_view, int, 3> typeStrToSize{
    {"int", -32}, {"unsigned int", 32}, {"int8_t", -8}};

} // namespace

class FindNamedClassVisitor
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
  explicit FindNamedClassVisitor(ASTContext *Context, std::string_view name)
      : Context(Context),
        emscripten_file(std::string{name.data(), name.size()}) {}

  bool VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
    if ((Declaration->getQualifiedNameAsString().starts_with(
             "clang::format::FormatStyle") ||
         Declaration->getQualifiedNameAsString().starts_with(
             "clang::tooling::IncludeStyle")) &&
        (Declaration->getDeclContext()->isNamespace() ||
         Declaration->getAccess() == clang::AccessSpecifier::AS_public)) {
      emscripten_file << "emscripten::class_<"
                      << Declaration->getQualifiedNameAsString() << ">(\""
                      << Declaration->getNameAsString() << "\")\n"
                      << ".constructor(+[]() {\n"
                      << "  return initialize<"
                      << Declaration->getQualifiedNameAsString() << ">();"
                      << "})";
      for (const auto *Field : Declaration->fields()) {
        if (Field->getAccess() == clang::AccessSpecifier::AS_public) {
          emscripten_file << "\n.property(\"" << Field->getNameAsString()
                          << "\", &" << Field->getQualifiedNameAsString();
          const QualType FieldType = Field->getType();
          if (FieldType->isRecordType()) {
            emscripten_file << ", emscripten::return_value_policy::reference()";
          }

          emscripten_file << ")";
          if (FieldType->isIntegralType(*Context) &&
              !FieldType->isBooleanType()) {
            emscripten_file << "\n";
            emscripten_file
                << ".function(\"get" << Field->getNameAsString()
                << "Type\", +[](const "
                << Declaration->getQualifiedNameAsString() << "&){return "
                << typeStrToSize.at(FieldType.getAsString()) << ";})";
          }
        }
      }
      emscripten_file << ";\n\n";
    }
    return true;
  }

  bool VisitEnumDecl(EnumDecl *Declaration) {
    if (Declaration->getQualifiedNameAsString().starts_with(
            "clang::format::FormatStyle") ||
        Declaration->getQualifiedNameAsString().starts_with(
            "clang::tooling::IncludeStyle") &&
            Declaration->getAccess() == clang::AccessSpecifier::AS_public) {
      emscripten_file << "emscripten::enum_<"
                      << Declaration->getQualifiedNameAsString() << ">(\""
                      << Declaration->getNameAsString() << "\")";
      for (const auto *Field : Declaration->enumerators()) {
        emscripten_file << "\n.value(\"" << Field->getNameAsString() << "\", "
                        << Field->getQualifiedNameAsString() << ")";
      }
      emscripten_file << ";\n\n";
    }
    return true;
  }

private:
  ASTContext *Context;
  std::ofstream emscripten_file;
};

class FindNamedClassConsumer : public clang::ASTConsumer {
public:
  explicit FindNamedClassConsumer(ASTContext *Context, std::string_view name)
      : Visitor(Context, name) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction {
public:
  explicit FindNamedClassAction(std::string_view name) : filename(name) {}
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler,
                    llvm::StringRef /*InFile*/) override {
    return std::make_unique<FindNamedClassConsumer>(&Compiler.getASTContext(),
                                                    filename);
  }

private:
  std::string_view filename;
};

int main(int argc, char **argv) {
  std::vector<std::string> args;
  std::string output_emscripten;

  args.emplace_back("-std=c++17");

  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-I") == 0) {
      args.emplace_back("-I");
      args.emplace_back(argv[i + 1]);
    }
    if (strcmp(argv[i], "-OCPP") == 0) {
      output_emscripten = argv[i + 1];
    }
  }

  if (argc > 1) {
    clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<FindNamedClassAction>(output_emscripten),
        llvm::MemoryBuffer::getFileOrSTDIN(argv[1]).get()->getBuffer(), args);
  }
}
