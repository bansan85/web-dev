#include <iostream>
#include <string>

#include "../native/lighten.h"

int main() {
  std::string input;
  std::getline(std::cin, input);
  web_lighten::number(input);
  return 0;
}
