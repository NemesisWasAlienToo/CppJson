# CppJson (V1)

This implementation of Json at least at this point is __ONLY__ for educational purposes.
The goal of this implementation is to implement json in c++ the simplest possible way and to demonstrate a few fundamental problems which will arise in the process.

## Dependencies:

This repository has dependency to ctre located at: https://github.com/hanickadot

## The problem im addressing here

Since in C++, all objects can use brace initialized parameters, and there is practically no way to differ a vector brace initialization from a json or any other object if they contain similar constructors, since each json value can either contain a json itself, a vector and etc, it is not yet possible to differentiate between for example a json and a list of lists both initialized with:

```cpp
 ... = {{"first", 1},{"second", 2}};
```

To address this problem, The default json only accepts the exact type mentioned not a type which can construct one of them instead and the type detection is delegated to the strategy the user provides. One can implement their own strategy, detecting the actual constructable type more suitable to the types passed to the constructor when the exact type is not passed or even when it is! For example one might decide that if the above syntax is observed, by default it is considered to be a json or a list of lists, unless explicitly mentioned otherwise.

This implementation also aims to provide a lightweight, future proof and flexible json implementation. The types that the json object contains can all be changed and even extra types can be added which are not part of the official definition and can be adjusted for the use case to for example contain std::string_view on an embedded system.

## The defaults

There is also a default strategy defined which should be sufficient for the daily uses. This default strategy is define in the Core::DefaultStrategy which will be explained how should be used. The default strategy needs the following types to be present:

- __nullptr_t__ corresponding to the __null__
- __bool__ corresponding to the __bool__
- __integer__ corresponding to the __integer__
- __floating point__ corresponding to the __floating point__
- __string__ corresponding to the __string__

Bello you can see the criteria for each type as defined in the default strategy:

```cpp
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
```
Note that if you define your own strategy there is no constrains on how, where or even either if you might want to define these, but there are defined clearly for the default strategy to make it more usable and understandable.

The default strategy is as follows:

- For the arguments exactly matching one of the types mentioned for the container, it is just forwarded. (This is the only way a Json value or an Array value is detected.)

- For the arguments matching one of the groups bellow, the type for that group is selected from the types passed to the container:
    - null
    - bool
    - integer
    - floating point
    - string
- For other any other arguments passed, the first type which is constructable from the types passed to the container is selected.

## How to use

To use the Core::Json we first need to construct a strategy. TO do so, you can either use your own one or just use the default one provided. We will use this strategy to to build a Recursive container type like bellow:

```cpp
template <typename J>
using type = Core::DefaultStrategy<J, std::string_view, float, int64_t, bool, std::nullptr_t>;
```
The first argument has to stay like this to preserve the recursive property of the type, and the rest of the types are the types you intent for you json value to be able to contain.

The nex step is to define the Json type like bellow:

```cpp
using Json = Core::Json<std::string_view, type>;
```

The first type passed to the Core::Json is the type for the json's __Key__ type and the second is the recursive container type we just constructed.

And now we can go ahead and use the type.

Note that in the default strategy, a value which itself is either Json or Array, has to be explicitly marked with their types respectively __Json__ and __Json::Array__. and for the rest of the types, they will go through the strategy and the proper type is selected.

```cpp
// The default strategy

Json Object{
        {"Parent", Json{{"Data1", 0}, {"Data2", 1}}}, // <-- must be explicitly marked as Json 
        {"List", Json::Array{1, 2, 3, 4}}, // <-- must be explicitly marked as Json::Array
        {"Boolean", true},
        ...
    };
```

## How to define my own strategy?

Strategy is defined in the class which is passed to Core::Json class. First you need to create a new class and to simplify the process, inherit from Core::Recursive class. Note that you can also implement your own from the ground up too if you really insist! Then you need to implement the constructor and embed your strategy in it. The way it is done in the default strategy is through a constexpr functions:

```cpp
template <typename... TArgs>
static constexpr auto Strategy() 
{
    ...
    return type_identity<detected_type>{};
    ...
}

template <typename... Types>
constexpr inline DefaultStrategy(Types &&...Args)
    : Base(typename decltype(Strategy<std::decay_t<Types>...>())::type(std::forward<Types>(Args)...))
{
}
```

And then you need to implement your own parser in the format of bellow:

```cpp
constexpr static DefaultStrategy From(std::string_view sv, std::size_t &Index)
```
sv is the string_view containing a string which has one or potentially multiple values. This function should parse one value which the string starts with and set the Index to point to the index of the rest of the string. Or you can just copy the default one and tweak it :)

## Compilation

After installing the dependencies to compile the example just do

```sh
mkdir build && cd build
cmake .. && make
```