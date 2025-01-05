#include "3.3.0/Format.h"
#include "3.4.2/Format.h"
#include "3.5.2/Format.h"

namespace clang_update_v3_4 {

clang_v3_4::FormatStyle update(clang_v3_3::FormatStyle &old,
                               const std::string &style);

} // namespace clang_update_v3_4
namespace clang_update_v3_5 {

clang_v3_5::FormatStyle update(clang_v3_4::FormatStyle &old,
                               const std::string &style);

} // namespace clang_update_v3_5
