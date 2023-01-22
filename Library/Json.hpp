#include <variant>
#include <map>
#include <vector>
#include <string>
#include <iostream>

template <typename... TArgs>
struct TypeList
{
    template <typename... T>
    struct FirstConstructible
    {
        using type = void;
    };

    template <typename T, typename... O>
    struct FirstConstructible<T, O...>
    {
        using type = std::conditional_t<
            std::is_constructible_v<T, TArgs...>,
            T,
            typename FirstConstructible<O...>::type>;
    };
};

template <typename...>
struct Search
{
    using type = void;
};

template <typename T, typename TFirst, typename... TRest>
struct Search<T, TFirst, TRest...>
{
    using type = typename Search<T, TRest...>::type;
};

template <typename T, typename... TRest>
struct Search<T, T, TRest...>
{
    using type = T;
};

namespace Core
{
    template <typename... Ts>
    struct Json
    {
        template <typename T, typename... TO>
        struct JsonValue
        {
            using Vector = std::vector<typename T::type>;
            using type = std::variant<T, Vector, TO...>;

            JsonValue() = default;

            JsonValue(std::initializer_list<typename T::Pair> Args)
            {
                Item.template emplace<T>(Args);
            }

            JsonValue(T Args)
            {
                Item.template emplace<T>(Args);
            }

            JsonValue(std::initializer_list<typename T::type> Args)
            {
                Item.template emplace<Vector>(Args);
            }

            JsonValue(Vector Args)
            {
                Item.template emplace<Vector>(Args);
            }

            template <typename... Types>
            JsonValue(Types &&...Args)
                requires(!std::is_same_v<typename TypeList<Types...>::template FirstConstructible<TO...>::type, void>)
            {
                using FCT = std::decay_t<typename TypeList<Types...>::template FirstConstructible<TO...>::type>;

                Item.template emplace<FCT>(std::forward<Types>(Args)...);
            }

            template <typename Types>
            JsonValue(Types &&Args)
                requires(!std::is_same_v<typename Search<std::decay_t<Types>, TO...>::type, void>)
            {
                using ET = typename Search<std::decay_t<Types>, TO...>::type;

                Item.template emplace<ET>(std::forward<Types>(Args));
            }

            friend std::ostream &operator<<(std::ostream &os, JsonValue const &jv)
            {
                return std::visit(
                    [&](auto &&arg) mutable -> std::ostream &
                    {
                        using TArg = std::decay_t<decltype(arg)>;

                        if constexpr (std::is_same_v<TArg, Vector>)
                        {
                            os << '[';

                            auto &Vec = std::get<Vector>(jv.Item);
                            size_t i = 0;

                            for (; i < Vec.size() - 1; i++)
                                os << Vec[i] << ", ";

                            return os << Vec[i] << ']';
                        }
                        else
                        {
                            return os << arg;
                        }
                    },
                    jv.Item);
            }

            JsonValue &operator[](std::string const &Key)
            {
                if (!std::holds_alternative<T>(Item))
                {
                    throw std::invalid_argument("Object is not json");
                }

                auto &cData = std::get<T>(Item).Data;

                auto Iterator = cData.find(Key);

                if (Iterator == cData.end())
                {
                    throw std::invalid_argument("Object does not contain this key");
                }

                return Iterator->second;
            }

            auto &GetItem()
            {
                return Item;
            }

        private:
            type Item;
        };

        using type = JsonValue<Json, Ts...>;
        using Map = std::map<std::string, type>;
        using Pair = std::pair<std::string, type>;
        using Vector = std::vector<type>;

        Json() = default;

        Json(std::initializer_list<Pair> list)
        {
            for (auto const &Item : list)
            {
                Data.emplace(Item);
            }
        }

        type &operator[](std::string const &Key)
        {
            auto Iterator = Data.find(Key);

            if (Iterator == Data.end())
            {
                throw std::invalid_argument("Object does not contain this key");
            }

            return Iterator->second;
        }

        friend std::ostream &operator<<(std::ostream &os, Json const &j)
        {
            os << "{\n";

            for (auto const &[Key, JValue] : j.Data)
            {
                os << "  \"" << Key + "\" : " << JValue << ",\n";
            }

            return os << "}";
        }

        auto &GetData()
        {
            return Data;
        }

    private:
        Map Data;
    };
}