#include "10.0.1/Format.h"
#include "11.1.0/Format.h"
#include "12.0.1/Format.h"
#include "13.0.1/Format.h"
#include "14.0.6/Format.h"
#include "15.0.7/Format.h"
#include "16.0.6/Format.h"
#include "17.0.6/Format.h"
#include "18.1.8/Format.h"
#include "19.1.6/Format.h"
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
#include "7.1.0/Format.h"
#include "8.0.1/Format.h"
#include "9.0.1/Format.h"

namespace clang_vx {
enum class Update { DOWNGRADE, UPGRADE };
}

namespace clang_update_v3_4 {

template <clang_vx::Update Upgrade>
void update(clang_v3_3::FormatStyle &prev, clang_v3_4::FormatStyle &next,
            const std::string &style);

} // namespace clang_update_v3_4

namespace clang_update_v3_5 {

template <clang_vx::Update Upgrade>
void update(clang_v3_4::FormatStyle &prev, clang_v3_5::FormatStyle &next,
            const std::string &style);

} // namespace clang_update_v3_5

namespace clang_update_v3_6 {

template <clang_vx::Update Upgrade>
void update(clang_v3_5::FormatStyle &prev, clang_v3_6::FormatStyle &next,
            const std::string &style);

} // namespace clang_update_v3_6

namespace clang_update_v3_7 {

template <clang_vx::Update Upgrade>
void update(clang_v3_6::FormatStyle &prev, clang_v3_7::FormatStyle &next,
            const std::string &style);

} // namespace clang_update_v3_7

namespace clang_update_v3_8 {

template <clang_vx::Update Upgrade>
void update(clang_v3_7::FormatStyle &prev, clang_v3_8::FormatStyle &next,
            const std::string &style);

} // namespace clang_update_v3_8

namespace clang_update_v3_9 {

template <clang_vx::Update Upgrade>
void update(clang_v3_8::FormatStyle &prev, clang_v3_9::FormatStyle &next,
            const std::string &style);

} // namespace clang_update_v3_9

namespace clang_update_v4 {

template <clang_vx::Update Upgrade>
void update(clang_v3_9::FormatStyle &prev, clang_v4::FormatStyle &next,
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

namespace clang_update_v7 {

clang_v7::FormatStyle update(clang_v6::FormatStyle &old,
                             const std::string &style);

} // namespace clang_update_v7

namespace clang_update_v8 {

clang_v8::FormatStyle update(clang_v7::FormatStyle &old,
                             const std::string &style);

} // namespace clang_update_v8

namespace clang_update_v9 {

clang_v9::FormatStyle update(clang_v8::FormatStyle &old,
                             const std::string &style);

} // namespace clang_update_v9

namespace clang_update_v10 {

clang_v10::FormatStyle update(clang_v9::FormatStyle &old,
                              const std::string &style);

} // namespace clang_update_v10

namespace clang_update_v11 {

clang_v11::FormatStyle update(clang_v10::FormatStyle &old,
                              const std::string &style);

} // namespace clang_update_v11

namespace clang_update_v12 {

clang_v12::FormatStyle update(clang_v11::FormatStyle &old,
                              const std::string &style);

} // namespace clang_update_v12

namespace clang_update_v13 {

clang_v13::FormatStyle update(clang_v12::FormatStyle &old,
                              const std::string &style);

} // namespace clang_update_v13

namespace clang_update_v14 {

clang_v14::FormatStyle update(clang_v13::FormatStyle &old,
                              const std::string &style);

} // namespace clang_update_v14

namespace clang_update_v15 {

clang_v15::FormatStyle update(clang_v14::FormatStyle &old,
                              const std::string &style);

} // namespace clang_update_v15

namespace clang_update_v16 {

clang_v16::FormatStyle update(clang_v15::FormatStyle &old,
                              const std::string &style);

} // namespace clang_update_v16

namespace clang_update_v17 {

clang_v17::FormatStyle update(clang_v16::FormatStyle &old,
                              const std::string &style);

} // namespace clang_update_v17

namespace clang_update_v18 {

clang_v18::FormatStyle update(clang_v17::FormatStyle &old,
                              const std::string &style);

} // namespace clang_update_v18

namespace clang_update_v19 {

clang_v19::FormatStyle update(clang_v18::FormatStyle &old,
                              const std::string &style);

} // namespace clang_update_v19
