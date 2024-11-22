#include <fstream>
#include <map>
#include <vector>

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
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

using namespace clang;

namespace {
std::map<std::string_view, int> typeStrToSize{
    {"int", -32}, {"unsigned int", 32}, {"int8_t", -8}};
}

class FindNamedClassVisitor
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
  explicit FindNamedClassVisitor(ASTContext *Context, std::string_view name)
      : Context(Context), emscripten_file(name.data()) {}

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
        QualType fieldType = Field->getType();

        if (Field->getAccess() == clang::AccessSpecifier::AS_public) {
          emscripten_file << "\n.property(\"" << Field->getNameAsString()
                          << "\", &" << Field->getQualifiedNameAsString();
          QualType FieldType = Field->getType();
          if (FieldType->isRecordType()) {
            emscripten_file << ", emscripten::return_value_policy::reference()";
          }

          emscripten_file << ")";
          if (fieldType->isIntegralType(*Context) &&
              !fieldType->isBooleanType()) {
            emscripten_file << "\n";
            emscripten_file
                << ".function(\"get" << Field->getNameAsString()
                << "Type\", +[](const "
                << Declaration->getQualifiedNameAsString() << "&){return "
                << typeStrToSize.at(fieldType.getAsString()) << ";})";
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

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction {
public:
  FindNamedClassAction(std::string_view name) : filename(name) {}
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::make_unique<FindNamedClassConsumer>(&Compiler.getASTContext(),
                                                    filename);
  }

private:
  std::string_view filename;
};

int main(int argc, char **argv) {
  std::vector<std::string> args;
  std::string output_emscripten;

  args.push_back("-std=c++17");

  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-I") == 0) {
      args.push_back("-I");
      args.push_back(argv[i + 1]);
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
