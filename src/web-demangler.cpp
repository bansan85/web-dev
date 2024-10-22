#include <emscripten.h>
#include <emscripten/bind.h>
#include <llvm/Demangle/Demangle.h>
#include <string>

std::string demangle(const std::string &mangledName) {
  return llvm::demangle(mangledName.c_str());
}

EMSCRIPTEN_BINDINGS(web_demangler) {
  emscripten::function("web_demangle", &demangle);
}
