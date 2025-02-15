#include <iostream>
#include <string>
#include <string_view>

int main() {
    std::string_view str = "123456";
    std::cout << str << std::endl;
    std::cout << str.substr(2, 3) << std::endl;  // "234"
    return 0;
}
