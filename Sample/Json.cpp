#include <variant>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <Json.hpp>

int main(int, char const *[])
{
    {
        using Json = Core::Json<int>;

        // Must compile

        {
            std::cout << Json::type(1) << std::endl;

            std::cout << Json::type{1} << std::endl;

            std::cout << Json::type{1, 2, 3, 4} << std::endl;

            std::cout << Json::type{{1, 2, 3, 4}, {1, 2, 3, 4}} << std::endl;

            std::cout << Json{{"First", 1}, {"Second", 2}} << std::endl;

            std::cout << Json{{"First", {1, 2, 3, 4}}, {"Second", {1, 2, 3, 4}}} << std::endl;

            std::cout << Json{{"First", {{"First", 0}, {"Second", 1}}}} << std::endl;
        }

        // Must compile

        {
            Json json{
                {"Data1", 0},
                {"Parent", {{"Data1", 0}, {"Data2", 1}}},
                {"List", {1, 2, 3, 4}},
                {"MapList", {{{"Hello", 0}, {"Oi", 3}, {"Hallo", 4}}, {{"Hello", 2}, {"Oi", 3}, {"Hallo", 4}}}},
                {"ListList", {{1, 2, 3}, {4, 5, 6}}},
            };
        }

        // Must not compile
    }

    //

    {
        using Json = Core::Json<int, std::string>;

        // Must compile

        {
            std::cout << Json::type(1) << std::endl;
            std::cout << Json::type("Hello") << std::endl;

            std::cout << Json::type{1} << std::endl;
            std::cout << Json::type{"Hi"} << std::endl;

            std::cout << Json::type{1, "2", 3, "4/5"} << std::endl;

            std::cout << Json::type{{1, "2", 3, "4/5"}, {1, "2", 3, "4/5"}} << std::endl;

            std::cout << Json{{"First", 1}, {"Second", 2}} << std::endl;

            std::cout << Json{{"First", "1"}, {"Second", "2"}} << std::endl;

            std::cout << Json{{"MapList", {Json{{"Hello", "There"}}, Json{{"Hello", "There"}}}}} << std::endl;

            std::cout << Json{{"ListList", {{{"Hello", "There", "List"}}, {{"Hello", "There", "List2"}}}}} << std::endl;

            std::cout << Json{{"ListList", {{Json::Vector{"Hello", "There"}}, Json::Vector{{"Hello", "There"}}}}} << std::endl;

            std::cout << Json{{"First", {1, "2", 3, "4/5"}}, {"Second", {1, "2", 3, "4/5"}}} << std::endl;

            std::cout << Json{{"First", Json{{"First", 0}, {"Second", 1}}}} << std::endl;
        }

        // Must compile

        {
            Json json{
                {"Data1", "3"},
                {"Data2", 0},
                {"Parent", Json{{"Data1", 0}, {"Data2", 1}}},
                {"List", {1, 2, 3, 4}},
                {"MapList", {Json{{"Hello", "There"}, {"Oi", 3}, {"Hallo", 4}}, Json{{"Hello", "There"}, {"Oi", 3}, {"Hallo", 4}}}},
                {"ListList", {Json::Vector{"Hello", "There"}, Json::Vector{"Hello", "There"}}},
            };
        }

        // Must not compile

        {
            // std::cout << Json::type{{"First", 1}, {"Second", 2}} << std::endl;

            // std::cout << Json{{"MapList", {{{"Hello", "There"}}, {{"Hello", "There"}}}}} << std::endl;
        }
    }

    return 0;
}