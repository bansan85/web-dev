#include <emscripten/bind.h>
#include <string>

namespace web_lighten {

std::string number(std::string num) {
  // Remove decimal separator
  size_t pos = num.find('.');
  if (pos != std::string::npos) {
    num = num.substr(0, pos) + num.substr(pos + 1, num.length() - pos - 1);
  } else {
    pos = num.length();
  }

  // Truncate if 0000 is found.
  size_t pos0000 = num.find("0000");
  if (pos0000 != std::string::npos) {
    num = num.substr(0, pos0000);
  }

  // Truncate if 9999 is found.
  size_t pos9999 = num.find("9999");
  if (pos9999 != std::string::npos) {
    num = num.substr(0, pos9999);
    size_t old_length = num.length();
    long long number;
    if (old_length == 0) {
      number = 1;
    } else if (old_length == 1 && num[0] == '-') {
      number = -1;
    } else {
      number = stoll(num);
      if (number < 0) {
        number--;
      } else {
        number++;
      }
    }
    num = std::to_string(number);
    pos = pos - old_length + num.length();
  }

  if (pos != std::string::npos) {
    if (pos > num.length()) {
      num = num + std::string(pos - num.length(), '0');
    } else if (pos < num.length()) {
      num = num.substr(0, pos) + '.' + num.substr(pos, num.length() - pos);
    }
  }

  return num;
}

} // namespace web_lighten
