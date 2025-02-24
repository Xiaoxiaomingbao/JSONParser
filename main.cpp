#include <string>
#include <string_view>
#include <charconv>
#include <regex>
#include <optional>
#include <variant>
#include <vector>
#include <unordered_map>

#include "print.h"

struct JSONObject {
    std::variant
    < std::monostate  // null
    , bool  // true
    , int  // 24
    , double  // 3.14
    , std::string  // "hello"
    , std::vector<JSONObject>  // [24, "hello"]
    , std::unordered_map<std::string, JSONObject>  // {"hello": 23, "world": 24}
    > inner;

    void do_print() const {
        printnl(inner);
    }
};

template <class T>
std::optional<T> try_parse_num(std::string_view str) {
    // std::optional<T> 仅有两种可能：T 类型的值或 std::nullopt
    T value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value);
    // std::from_chars() 支持解析负数、科学计数法
    // res.ec 是一个枚举值，表示转换的错误代码
    // std::errc 是一个枚举类，定义了多个与错误相关的常量；std::errc() 是该枚举类的默认构造函数，它返回一个值，表示“没有错误”
    // res.ptr 是指向字符串中最后解析的字符的指针
    if (res.ec == std::errc() && res.ptr == str.data() + str.size()) {
        return value;
    }
    return std::nullopt;
}

JSONObject parse(std::string_view json) {
   if (json.empty()) {
        return JSONObject{std::nullptr_t{}};
   }
   if ('0' <= json[0] && json[0] <= '9' || json[0] == '+' || json[0] == '-')
   {
        std::regex num_re{"[+-]?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?"};
        std::cmatch match;
        if (std::regex_search(json.data(), json.data() + json.size(), match, num_re))
        {
            std::string str = match.str();
            if (auto num = try_parse_num<int>(str); num.has_value())
            {
                return JSONObject{num.value()};
            }
            if (auto num = try_parse_num<double>(str); num.has_value())
            {
                return JSONObject{num.value()};
            }
        }
   }
   return JSONObject{std::nullptr_t{}};
}

int main() {
    std::string_view json1 = "3";
    print(parse(json1));
    std::string_view json2 = "3.14";
    print(parse(json2));
    return 0;
}
