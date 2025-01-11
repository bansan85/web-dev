#include "3.3.0/Format.h"
#include "3.4.2/Format.h"
#include "3.5.2/Format.h"
#include "3.6.2/Format.h"
#include "3.7.1/Format.h"
#include "3.8.1/Format.h"
#include "3.9.1/Format.h"
#include "4.0.1/Format.h"
#include "5.0.2/Format.h"
#include "6.0.1/Format.h"

namespace clang_update_v3_4 {

clang_v3_4::FormatStyle update(clang_v3_3::FormatStyle &old,
                               const std::string &style);

} // namespace clang_update_v3_4

namespace clang_update_v3_5 {

clang_v3_5::FormatStyle update(clang_v3_4::FormatStyle &old,
                               const std::string &style);

} // namespace clang_update_v3_5

namespace clang_update_v3_6 {

clang_v3_6::FormatStyle update(clang_v3_5::FormatStyle &old,
                               const std::string &style);

} // namespace clang_update_v3_6

namespace clang_update_v3_7 {

clang_v3_7::FormatStyle update(clang_v3_6::FormatStyle &old,
                               const std::string &style);

} // namespace clang_update_v3_7

namespace clang_update_v3_8 {

clang_v3_8::FormatStyle update(clang_v3_7::FormatStyle &old,
                               const std::string &style);

} // namespace clang_update_v3_8

namespace clang_update_v3_9 {

clang_v3_9::FormatStyle update(clang_v3_8::FormatStyle &old,
                               const std::string &style);

} // namespace clang_update_v3_9

namespace clang_update_v4 {

clang_v4::FormatStyle update(clang_v3_9::FormatStyle &old,
                             const std::string &style);

} // namespace clang_update_v4

namespace clang_update_v5 {

clang_v5::FormatStyle update(clang_v4::FormatStyle &old,
                             const std::string &style);

} // namespace clang_update_v5

namespace clang_update_v6 {

clang_v6::FormatStyle update(clang_v5::FormatStyle &old,
                             const std::string &style);

} // namespace clang_update_v6