#include <variant>
#include <optional>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <type_traits>
#include <ranges>
#include <charconv>

#define STRINGIFY(...) (#__VA_ARGS__)

template <template <typename> typename TCondition, typename T, typename... TR>
struct FirstWhere
{
    using type = typename FirstWhere<TCondition, TR...>::type;
};

template <template <typename> typename TCondition, typename T, typename... TR>
    requires(TCondition<T>::value)
struct FirstWhere<TCondition, T, TR...>
{
    using type = T;
};

template <typename T, typename... R>
struct FirstType
{
    using type = T;
};

template <typename T, typename... R>
struct FirstArg
{
    FirstArg(T &t, R &&...) : value(t) {}

    T &value;
};

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

template <typename T, typename... Args>
struct Contains
{
    static constexpr bool value{(std::is_same_v<T, Args> || ...)};
};

constexpr static void Skip(std::string_view sv, std::size_t &Index)
{
    while (Index < sv.length() && (sv[Index] == ' ' || sv[Index] == '\n'))
        Index++;
}

namespace Core
{
    template <typename T, typename... TO>
    struct Recursive
    {
    public:
        using Json = T;
        using Array = std::vector<typename T::Value>;
        using Value = std::variant<T, Array, TO...>;

        constexpr Recursive() = default;

        template <typename Types>
        constexpr inline Recursive(Types &&Args)
            requires(Contains<std::decay_t<Types>, Array, T, TO...>::value)
        {
            Item.template emplace<Types>(std::forward<Types>(Args));
        }

        constexpr Recursive &operator=(auto &&Other)
            requires(Contains<std::decay_t<decltype(Other)>, Array, T, TO...>::value)
        {
            Item = std::forward<decltype(Other)>(Other);

            return *this;
        }

        template <typename Types>
        constexpr bool operator==(Types const &Other) const
            requires(!std::is_same_v<Types, Recursive> && Contains<std::decay_t<Types>, Array, T, TO...>::value)
        {
            if (!std::holds_alternative<Types>(Item))
            {
                return false;
            }

            return std::get<Types>(Item) == Other;
        }

        constexpr bool operator==(Recursive const &Other) const
        {
            return Item == Other.Item;
        }

        constexpr Recursive &operator[](typename Json::Key const &Key)
        {
            if (!std::holds_alternative<T>(Item))
            {
                throw std::invalid_argument("Object is not json");
            }

            auto &cData = std::get<T>(Item).GetMap();

            return cData[Key];
        }

        constexpr Recursive &operator[](std::size_t Index)
        {
            if (!std::holds_alternative<Array>(Item))
            {
                throw std::invalid_argument("Object is not array");
            }

            auto &cData = std::get<Array>(Item);

            return cData[Index];
        }

        constexpr decltype(auto) Visit(auto &&Visitor)
        {
            return std::visit(std::forward<decltype(Visitor)>(Visitor), Item);
        }

        constexpr decltype(auto) Visit(auto &&Visitor) const
        {
            return std::visit(std::forward<decltype(Visitor)>(Visitor), Item);
        }

        constexpr auto &GetVariant()
        {
            return Item;
        }

        constexpr auto &GetVariant() const
        {
            return Item;
        }

        template <typename Target>
        constexpr inline bool Is() const
        {
            return std::holds_alternative<Target>(Item);
        }

        template <template <typename> typename TCondition>
        constexpr inline bool Is() const
        {
            using type = typename FirstWhere<TCondition, TO...>::type;

            static_assert(!std::is_same_v<void, type>, "Variant contains no such type");

            return std::holds_alternative<type>(Item);
        }

        template <typename Target>
        constexpr inline decltype(auto) As()
        {
            return std::get<Target>(Item);
        }

        template <typename Target>
        constexpr inline decltype(auto) As() const
        {
            return std::get<Target>(Item);
        }

        constexpr inline auto Index() const
        {
            return Item.index();
        }

    protected:
        Value Item;
    };

    template <typename TKey, template <typename> typename TValue>
        requires(std::is_constructible_v<TKey, char const *> && std::is_constructible_v<TKey, const char (&)[]> && std::is_constructible_v<TKey, std::string_view>)
    struct Json
    {
        using Key = TKey;
        using Value = TValue<Json>;
        using Map = std::map<TKey, Value>;
        using Pair = std::pair<TKey, Value>;
        using Array = std::vector<Value>;

        constexpr Json() = default;

        constexpr Json(std::initializer_list<Pair> list)
        {
            for (auto const &Item : list)
            {
                Data.emplace(Item);
            }
        }

        constexpr Value &Insert(TKey key, Value Val)
        {
            auto [it, success] = Data.insert(std::pair{std::move(key), std::move(Val)});

            return it->second;
        }

        template <typename Target>
        constexpr Value &InsertAs(TKey key, Value Val)
        {
            auto [it, success] = Data.insert(std::pair{std::move(key), Target{std::move(Val)}});

            return it->second;
        }

        constexpr inline Value &operator[](TKey const &key)
        {
            return Data[key];
        }

        constexpr auto &GetMap()
        {
            return Data;
        }

        constexpr auto &GetMap() const
        {
            return Data;
        }

        constexpr static Json From(std::string_view sv)
        {
            Json Result;

            From(Result, sv);

            return Result;
        }

        constexpr static std::string_view Trim(std::string_view str)
        {
            // Trim leading white spaces
            auto trimmed_start_view = str | std::ranges::views::drop_while([](char c)
                                                                           { return std::isspace(static_cast<unsigned char>(c)); });

            // Convert to string_view after trimming leading spaces
            std::string_view trimmed_start{trimmed_start_view.begin(), static_cast<std::string_view::size_type>(trimmed_start_view.size())};

            // Trim trailing white spaces
            auto reversed_view = std::ranges::views::reverse(trimmed_start);
            auto trimmed_reversed_view = reversed_view | std::ranges::views::drop_while([](char c)
                                                                                        { return std::isspace(static_cast<unsigned char>(c)); });

            // // Since these are reverse iterators, we need to get the base iterators and adjust
            return {trimmed_reversed_view.begin().base(), static_cast<std::string_view::size_type>(trimmed_reversed_view.size())};
        }

        inline constexpr static void From(Json &Object, std::string_view sv)
        {
            Pairs(Object, Trim(sv));
        }

        constexpr bool operator==(Json const &Other) const
        {
            return Data == Other.Data;
        }

        template <typename TSerializer>
        friend TSerializer &operator<<(TSerializer &os, Json const &json)
        {

            os << '{';

            bool firstPair = true;
            for (const auto &pair : json.GetMap())
            {
                if (!firstPair)
                {
                    os << ",";
                }
                os << "\"" << pair.first << "\"" << ":" << pair.second;

                firstPair = false;
            }

            os << '}';

            return os;
        }

    protected:
        Map Data;

        constexpr static std::string_view OneKey(std::string_view sv, std::size_t &Index)
        {
            if (Index >= sv.size())
                return "";

            std::size_t StartIndex = Index;
            std::size_t EndIndex = 0;

            while (StartIndex < sv.length() && sv[StartIndex] != '"')
            {
                StartIndex++;
            }

            StartIndex = Index;

            while (Index < sv.length() && sv[Index] != ':')
            {
                if (sv[Index] == '"')
                    EndIndex = Index;

                Index++;
            }

            if (Index >= sv.length())
                return "";

            ++Index;
            return sv.substr(StartIndex + 1, EndIndex - StartIndex - 1);
        }

        constexpr static void Pairs(Json &Object, std::string_view sv)
        {
            if (sv.size() == 0)
                return;

            std::size_t Index = 0;

            while (Index < sv.size())
            {
                std::string_view Key;

                Skip(sv, Index);

                if (Key = OneKey(sv, Index); !Index)
                    break;

                Skip(sv, Index);

                if (Object.Insert(TKey{Key}, Value::From(sv, Index)); !Index)
                    break;
            }
        }
    };

    template <typename T, typename... TO>
    struct DefaultStrategy : public Core::Recursive<T, TO...>
    {
    public:
        using Base = Core::Recursive<T, TO...>;

        template <typename Type>
        using is_null = std::is_same<std::nullptr_t, Type>;

        template <typename Type>
        using is_bool = std::is_same<bool, Type>;

        template <typename Type>
        using is_float = std::is_floating_point<Type>;

        template <typename Type>
        using is_int = std::is_integral<Type>;

        template <typename Type>
        using is_string = std::conjunction<std::negation<is_bool<Type>>, std::is_constructible<Type, char const *>, std::is_constructible<Type, const char (&)[]>>;

        template <typename... TArgs>
        static constexpr auto Strategy()
        {
            // Default constructor

            if constexpr (sizeof...(TArgs) == 0)
            {
                return std::type_identity<typename FirstWhere<is_null, TO...>::type>{};
            }

            // Single argument
            else if constexpr (sizeof...(TArgs) == 1)
            {
                using Arg = typename FirstType<TArgs...>::type;

                // Type match
                if constexpr (Contains<std::decay_t<Arg>, typename Base::Array, typename Base::Json, TO...>::value)
                    return std::type_identity<Arg>{};

                // Other compatible types
                else if constexpr (is_string<Arg>::value)
                    return std::type_identity<typename FirstWhere<is_string, TO...>::type>{};
                else if constexpr (is_float<Arg>::value)
                    return std::type_identity<typename FirstWhere<is_float, TO...>::type>{};
                else if constexpr (is_int<Arg>::value)
                    return std::type_identity<typename FirstWhere<is_int, TO...>::type>{};
                else if constexpr (is_bool<Arg>::value)
                    return std::type_identity<typename FirstWhere<is_bool, TO...>::type>{};
                else if constexpr (is_null<Arg>::value)
                    return std::type_identity<typename FirstWhere<is_null, TO...>::type>{};
                else
                    return std::type_identity<typename TypeList<Arg>::template FirstConstructible<TO...>::type>{};
            }
            else
            {
                using type = typename TypeList<TArgs...>::template FirstConstructible<TO...>::type;

                static_assert(
                    requires { requires !std::is_same_v<type, void>; }, "No constructable type was found");

                return std::type_identity<type>{};
            }
        }

        template <typename TNumber>
        constexpr static std::optional<TNumber> IsNumber(std::string_view str)
        {
            TNumber value;
            auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
            if (ec == std::errc() && ptr == str.data() + str.size()) return value;
            return std::nullopt;
        }

        constexpr static DefaultStrategy From(std::string_view sv, std::size_t &Index, char Stop = '}')
        {
            if (sv.length() == 0)
                return "";

            char OT = sv[Index];

            std::size_t StartIndex = 0;
            std::size_t EndIndex = 0;

            if (OT == '"')
            {
                StartIndex = Index;
                while (Index < sv.length() && ((sv[Index] != ',' && sv[Index] != Stop) || !EndIndex))
                {
                    if (sv[Index] == '"')
                        EndIndex = Index;

                    Index++;
                };

                if (Index >= sv.length())
                    return "";

                Index++;

                return typename FirstWhere<is_string, TO...>::type{sv.substr(StartIndex + 1, EndIndex - StartIndex - 1)};
            }
            else if (OT == '{')
            {
                std::size_t Count = 0;

                StartIndex = Index;
                while (Index < sv.length() && (Count | (sv[Index] != ',' && sv[Index] != Stop)))
                {
                    if (sv[Index] == '{')
                    {
                        Count++;
                    }
                    else if (sv[Index] == '}')
                    {
                        Count--;
                        EndIndex = Index;
                    }

                    Index++;
                };

                if (Index > sv.length())
                    return "";

                Index++;

                T Value;

                T::From(Value, sv.substr(StartIndex, EndIndex - StartIndex + 1));

                return Value;
            }
            else if (OT == '[')
            {
                std::size_t Count = 1;

                StartIndex = Index++;
                while (Index < sv.length() && (Count | (sv[Index] != ',' && sv[Index] != Stop)))
                {
                    if (sv[Index] == '[')
                    {
                        Count++;
                    }
                    else if (sv[Index] == ']')
                    {
                        Count--;
                        EndIndex = Index;
                    }

                    Index++;
                };

                if (Index > sv.length())
                    return "";

                // @todo Handle the case when the last ite4m in list also has "," after it like : [1,2,]

                auto ValueView = sv.substr(StartIndex + 1, EndIndex - StartIndex - 1);
                typename T::Array Value;

                std::size_t tmpIndex = 0;

                while (tmpIndex < ValueView.size())
                {
                    Skip(ValueView, tmpIndex);

                    if (Value.insert(Value.end(), From(ValueView, tmpIndex, ']')); !tmpIndex)
                        break;
                }

                Index++;

                return Value;
            }
            else
            {
                StartIndex = Index;
                while (Index < sv.length() && sv[Index] != ',' && sv[Index] != Stop)
                {
                    // if (sv[Index] != ' ')
                    //     EndIndex = Index;

                    Index++;
                }

                EndIndex = Index;

                if (Index > sv.length())
                    return "";

                Index++;

                auto Value = sv.substr(StartIndex, EndIndex - StartIndex);

                if (Value == "null")
                {
                    return typename FirstWhere<is_null, TO...>::type{nullptr};
                }
                else if (Value == "true")
                {
                    return typename FirstWhere<is_bool, TO...>::type{true};
                }
                else if (Value == "false")
                {
                    return typename FirstWhere<is_bool, TO...>::type{false};
                }
                else if (auto IntegerValue = IsNumber<std::int64_t>(Value))
                {
                    return IntegerValue.value();
                }
                else if (auto DoubleValue = IsNumber<double>(Value))
                {
                    return DoubleValue.value();
                }
                else
                {
                    return Value;
                }
            }
        }

        template <typename... Types>
        constexpr inline DefaultStrategy(Types &&...Args)
            : Base(typename decltype(Strategy<std::decay_t<Types>...>())::type(std::forward<Types>(Args)...))
        {
        }

        constexpr DefaultStrategy &operator=(auto Other)
        {
            Base::operator=(typename decltype(Strategy<std::decay_t<decltype(Other)>>())::type(std::forward<decltype(Other)>(Other)));

            return *this;
        }

        template <typename Types>
        constexpr bool operator==(Types &&Other) const
        {
            if constexpr (std::is_same_v<std::decay_t<Types>, DefaultStrategy>)
            {
                return Base::operator==(std::forward<Types>(Other));
            }
            else
            {
                using TResult = typename decltype(Strategy<std::decay_t<Types>>())::type;

                return Base::operator==(TResult(std::forward<Types>(Other)));
            }
        }

        template <typename TSerializer>
        friend TSerializer &operator<<(TSerializer &os, DefaultStrategy const &value)
        {
            return value.Visit(
                [&](auto &&arg) mutable -> TSerializer &
                {
                    using TArg = std::decay_t<decltype(arg)>;

                    if constexpr (std::is_same_v<TArg, typename T::Array>)
                    {
                        size_t i = 0;

                        os << '[';

                        for (; i < arg.size() - 1; i++)
                            os << arg[i] << ", ";

                        return os << arg[i] << ']';
                    }
                    else if constexpr (is_string<TArg>::value)
                    {
                        return os << '\"' << arg << '\"';
                    }
                    else if constexpr (is_bool<TArg>::value)
                    {
                        return os << (arg ? "true" : "false");
                    }
                    else if constexpr (is_null<TArg>::value)
                    {
                        return os << "null";
                    }
                    else
                    {
                        return os << arg;
                    }
                });
        }
    };
}
