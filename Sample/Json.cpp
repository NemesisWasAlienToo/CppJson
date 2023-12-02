#include <iostream>
#include <sstream>
#include <Core/Format/Json.hpp>

template <typename J>
using type = Core::DefaultStrategy<J, std::string_view, float, int64_t, bool, std::nullptr_t>;

using Json = Core::Json<std::string_view, type>;

int main(int, char const *[])
{
    // This uses the strategy defined in the the second argument
    // to the Json to detect the constructable type

    Json Object{
        {"Nullable", nullptr},
        {"Boolean", true},
        {"Other", 111},
        {"Double", 111.11},
        {"Data1", "3"},
        {"Data2", 0.001},
        {"Parent", Json{{"Data1", 0}, {"Data2", 1}}},
        {"List", Json::Array{1, 2, 3, 4}},
        {"MapList", Json::Array{Json{{"Hello", "There"}, {"Oi", 3}, {"Hallo", 4}}, Json{{"Hello", "There"}, {"Oi", 3}, {"Hallo", 4}}, "asdasd"}},
        {"ListList", Json::Array{Json::Array{"Hello", "There"}, Json::Array{"Hello", "There"}}},
        {"Count", 10},
    };

    std::cout << Object << std::endl;

    std::cout << Object["Nullable"].Is<std::nullptr_t>() << std::endl
              << Object["Other"].Is<int64_t>() << std::endl
              << Object["Double"].Is<float>() << std::endl
              << Object["Data1"].Is<std::string_view>() << std::endl
              << Object["Data2"].Is<float>() << std::endl
              << Object["Parent"].Is<Json>() << std::endl
              << Object["List"].Is<Json::Array>() << std::endl
              << Object["Count"].Is<int64_t>() << std::endl;

    Object = Json::From(STRINGIFY({
        "Nullable" : null,
        "Condition" : true,
        "Key1" : 1,
        "Message" : "Hello",
        "Value" : 2.2,
        "List" : [ "123", -22, 423.14 ],
        "Map" : {
            "Key2" : 15,
            "JsonList" : [ {"s" : 2}, -2123.2221, 3 ]
        }
    }));

    Object["Condition"] = false;

    std::cout << Object["Nullable"].Is<std::nullptr_t>() << std::endl
              << Object["Condition"].Is<bool>() << std::endl
              << Object["Message"].Is<std::string_view>() << std::endl
              << Object["Key1"].Is<int64_t>() << std::endl
              << Object["Value"].Is<float>() << std::endl
              << Object["List"].Is<Json::Array>() << std::endl
              << Object["List"][0].Is<std::string_view>() << std::endl
              << Object["List"][1].Is<int64_t>() << std::endl
              << Object["List"][2].Is<float>() << std::endl
              << Object["Map"].Is<Json>() << std::endl
              << Object["Map"]["Key2"].Is<int64_t>() << std::endl
              << Object["Map"]["JsonList"].Is<Json::Array>() << std::endl
              << Object["Map"]["JsonList"][0].Is<Json>() << std::endl
              << Object["Map"]["JsonList"][0]["s"].Is<int64_t>() << std::endl
              << Object["Map"]["JsonList"][1].Is<float>() << std::endl
              << Object["Map"]["JsonList"][2].Is<int64_t>() << std::endl;

    std::cout << Object << std::endl;

    // Serialize the json

    std::stringstream stream;

    stream << Object;

    std::cout << stream.str() << std::endl;

    return 0;
}
