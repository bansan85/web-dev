#include <emscripten/bind.h>
#include <llvm/Demangle/Demangle.h>
#include <string>
#include <string_view>

std::string demangle(const std::string &mangledName) {
  std::string retval = llvm::demangle(mangledName.c_str());
  if (retval == mangledName) {
    // Make a second try by prefixing with _Z for Itanium.
    std::string mangledNameZ = "_Z" + mangledName;
    std::string retvalZ = llvm::demangle(mangledNameZ.c_str());
    if (retvalZ != mangledNameZ) {
      return retvalZ;
    }
  }
  return retval;
}

EMSCRIPTEN_BINDINGS(web_demangler) {
  emscripten::function("web_demangle", &demangle);
}
