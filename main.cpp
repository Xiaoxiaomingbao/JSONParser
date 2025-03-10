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

    void do_print() const
    {
        printnl(inner);
    }
};

template <class T>
std::optional<T> try_parse_num(std::string_view str)
{
    // std::optional<T> 仅有两种可能：T 类型的值或 std::nullopt
    T value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value);
    // std::from_chars() 支持解析负数、科学计数法
    // res.ec 是一个枚举值，表示转换的错误代码
    // std::errc 是一个枚举类，定义了多个与错误相关的常量；std::errc() 是该枚举类的默认构造函数，它返回一个值，表示“没有错误”
    // res.ptr 是指向字符串中最后解析的字符的指针
    if (res.ec == std::errc() && res.ptr == str.data() + str.size())
    {
        return value;
    }
    return std::nullopt;
}

std::pair<JSONObject, size_t> parse(std::string_view json)
{
    if (json.empty()) {
        return {JSONObject{std::nullptr_t{}}, 0};
    }

    if ('0' <= json[0] && json[0] <= '9' || json[0] == '+' || json[0] == '-') {
        std::regex num_re{"[+-]?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?"};
        std::cmatch match;
        if (std::regex_search(json.data(), json.data() + json.size(), match, num_re)) {
            std::string str = match.str();
            if (auto num = try_parse_num<int>(str); num.has_value()) {
                return {JSONObject{num.value()}, str.size()};
            }
            if (auto num = try_parse_num<double>(str); num.has_value()) {
                return {JSONObject{num.value()}, str.size()};
            }
        }
    } else if ('"' == json[0]) {
        size_t next_quotation = json.find('"', 1);
        // substr() 第二个参数为长度，不是下标
        // 丢弃输入中自带的双引号
        std::string str{json.substr(1, next_quotation - 1)};
        return {JSONObject{std::move(str)}, next_quotation + 1};
    } else if (json[0] == '[') {
        std::vector<JSONObject> res;
        size_t i;
        for (i = 1; i < json.size();) {
            if (json[i] == ']') {
                i++;
                break;
            }
            // substr() 仅指定起始位置，直到末尾
            auto [obj, len] = parse(json.substr(i));
            if (len == 0) {
                print("Error: invalid json array");
                break;
            }
            res.push_back(std::move(obj));
            i += len;
            while (json[i] == ',' || json[i] == ' ') {
                i++;
            }
        }
        return {JSONObject{std::move(res)}, i};
    } else if (json[0] == '{') {
        std::unordered_map<std::string, JSONObject> res;
        size_t i;
        for (i = 1; i < json.size();) {
            if (json[i] == '}') {
                i++;
                break;
            }
            auto [key, len] = parse(json.substr(i));
            if (len == 0) {
                print("Error: invalid json object");
                break;
            }
            i += len;
            while (json[i] == ' ' || json[i] == ':') {
                i++;
            }
            auto [value, len2] = parse(json.substr(i));
            if (len2 == 0) {
                print("Error: invalid json object");
                break;
            }
            res[std::get<std::string>(key.inner)] = value;
            i += len2;
            while (json[i] == ',' || json[i] == ' ') {
                i++;
            }
        }
        return {JSONObject{std::move(res)}, i};
    }
    return {JSONObject{std::nullptr_t{}}, 0};
}

int main()
{
    std::string_view json_num = "-7";
    print(parse(json_num).first);
    json_num = "3.14";
    print(parse(json_num).first);
    json_num = "-2.5e2";
    print(parse(json_num).first);

    std::string_view json_str = "\"hello\"";
    print(parse(json_str).first);
    json_str = "\"Hello\nWorld!\"";
    print(parse(json_str).first);

    std::string_view json_array = "[1, 2, 3]";
    print(parse(json_array).first);
    json_array = "[1, [2, 3, 4], 5]";
    print(parse(json_array).first);
    json_array = "[123, 1.23, \"hello]world\", -5]";
    print(parse(json_array).first);

    std::string_view json_object = "{\"hello\": 23, \"world\": 24}";
    print(parse(json_object).first);
    json_object = "{\"hello\": 23, \"world\": [1, 2, 3]}";
    print(parse(json_object).first);
    json_object = "{\"hello\": 23, \"world\": {\"a\": 1, \"b\": 2}}";
    print(parse(json_object).first);
    return 0;
}
