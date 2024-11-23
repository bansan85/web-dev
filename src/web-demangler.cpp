#include <emscripten/bind.h>
#include <llvm/Demangle/Demangle.h>
#include <string>

namespace web_demangler {

namespace {

std::string demangle(const std::string &mangledName) {
  std::string retval = llvm::demangle(mangledName);
  if (retval == mangledName) {
    // Make a second try by prefixing with _Z for Itanium.
    const std::string mangledNameZ = "_Z" + mangledName;
    const std::string retvalZ = llvm::demangle(mangledNameZ);
    if (retvalZ != mangledNameZ) {
      return retvalZ;
    }
  }
  return retval;
}

} // namespace

} // namespace web_demangler

EMSCRIPTEN_BINDINGS(web_demangler) {
  emscripten::function("web_demangle", &web_demangler::demangle);
}
