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

using clang::AccessSpecifier;
using clang::ASTConsumer;
using clang::ASTContext;
using clang::ASTFrontendAction;
using clang::CompilerInstance;
using clang::CXXRecordDecl;
using clang::EnumDecl;
using clang::QualType;
using clang::RecursiveASTVisitor;
using llvm::StringRef;

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

constexpr frozen::unordered_map<std::string_view, int, 3> type_str_to_size{
    {"int", -32}, {"unsigned int", 32}, {"int8_t", -8}};

} // namespace

class FindNamedClassVisitor
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
  explicit FindNamedClassVisitor(ASTContext *context, std::string_view name)
      : _context(context),
        _emscripten_file(std::string{name.data(), name.size()}) {}

  bool VisitCXXRecordDecl(CXXRecordDecl *declaration) {
    if ((declaration->getQualifiedNameAsString().find("::FormatStyle") !=
             std::string::npos ||
         declaration->getQualifiedNameAsString().find("::IncludeStyle") !=
             std::string::npos) &&
        (declaration->getDeclContext()->isNamespace() ||
         declaration->getAccess() == AccessSpecifier::AS_public)) {
      _emscripten_file << "emscripten::class_<"
                       << declaration->getQualifiedNameAsString() << ">(\""
                       << declaration->getNameAsString()
                       << extractClangPostfix(
                              declaration->getQualifiedNameAsString())
                       << "\")\n"
                       << ".constructor(+[]() {\n"
                       << "  return initialize<"
                       << declaration->getQualifiedNameAsString() << ">();"
                       << "})";
      for (const auto *field : declaration->fields()) {
        if (field->getAccess() == AccessSpecifier::AS_public) {
          _emscripten_file << "\n.property(\"" << field->getNameAsString()
                           << "\", &" << field->getQualifiedNameAsString();
          const QualType field_type = field->getType();
          if (field_type->isRecordType()) {
            _emscripten_file
                << ", emscripten::return_value_policy::reference()";
          }

          _emscripten_file << ")";
          if (field_type->isIntegralType(*_context) &&
              !field_type->isBooleanType()) {
            _emscripten_file << "\n";
            _emscripten_file
                << ".function(\"get" << field->getNameAsString()
                << "Type\", +[](const "
                << declaration->getQualifiedNameAsString() << "&){return "
                << type_str_to_size.at(field_type.getAsString()) << ";})";
          }
        }
      }
      _emscripten_file << ";\n\n";
    }
    return true;
  }

  bool VisitEnumDecl(EnumDecl *declaration) {
    if (declaration->getQualifiedNameAsString().find("::FormatStyle") !=
            std::string::npos ||
        declaration->getQualifiedNameAsString().find("::IncludeStyle") !=
                std::string::npos &&
            declaration->getAccess() == AccessSpecifier::AS_public) {
      _emscripten_file << "emscripten::enum_<"
                       << declaration->getQualifiedNameAsString() << ">(\""
                       << declaration->getNameAsString()
                       << extractClangPostfix(
                              declaration->getQualifiedNameAsString())
                       << "\")";
      for (const auto *field : declaration->enumerators()) {
        _emscripten_file << "\n.value(\"" << field->getNameAsString() << "\", "
                         << field->getQualifiedNameAsString() << ")";
      }
      _emscripten_file << ";\n\n";
    }
    return true;
  }

private:
  ASTContext *_context;
  std::ofstream _emscripten_file;

  std::string extractClangPostfix(const std::string &input) {
    std::string prefix = "clang_v";
    std::size_t start = input.find(prefix);
    if (start != std::string::npos) {
      start += prefix.size();
      std::size_t end = input.find("::", start);
      if (end != std::string::npos) {
        std::string version = input.substr(start, end - start);
        return "V" + version;
      }
    }
    return "";
  }
};

class FindNamedClassConsumer : public ASTConsumer {
public:
  explicit FindNamedClassConsumer(ASTContext *context, std::string_view name)
      : _visitor(context, name) {}

  void HandleTranslationUnit(ASTContext &context) override {
    _visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  FindNamedClassVisitor _visitor;
};

class FindNamedClassAction : public ASTFrontendAction {
public:
  explicit FindNamedClassAction(std::string_view name) : _filename(name) {}
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &compiler, StringRef /*InFile*/) override {
    return std::make_unique<FindNamedClassConsumer>(&compiler.getASTContext(),
                                                    _filename);
  }

private:
  std::string_view _filename;
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
