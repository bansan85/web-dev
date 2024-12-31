#include "demangler.h"

#include <llvm/Demangle/Demangle.h>
#include <string>

namespace web_demangler {

std::string demangle(const std::string &mangled_name) {
  std::string retval = llvm::demangle(mangled_name);
  if (retval == mangled_name) {
    // Make a second try by prefixing with _Z for Itanium.
    const std::string mangled_name_z = "_Z" + mangled_name;
    const std::string retval_z = llvm::demangle(mangled_name_z);
    if (retval_z != mangled_name_z) {
      return retval_z;
    }
  }
  return retval;
}

} // namespace web_demangler
