#include <string>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <llvm/Demangle/Demangle.h>

std::string demangle(const std::string& mangledName) {
    return llvm::demangle(mangledName.c_str());
}

EMSCRIPTEN_BINDINGS(web_demangler) {
    emscripten::function("demangle", &demangle);
}

