#include <emscripten/bind.h>

#include "../native/lighten.h"

EMSCRIPTEN_BINDINGS(web_lighten) {
  emscripten::function("web_lighten_number", &web_lighten::number);
}
