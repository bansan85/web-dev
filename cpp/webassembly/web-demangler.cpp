#include <emscripten/bind.h>

#include "../native/demangler.h"

EMSCRIPTEN_BINDINGS(web_demangler) {
  emscripten::function("web_demangle", &web_demangler::demangle);
}
