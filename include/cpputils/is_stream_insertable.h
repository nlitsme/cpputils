#pragma once
// problem with clang: the operator<< functions must be declared -before-
// including this file.  with gcc the declaration order does not matter.

/** test if T can be inserted in a std::ostream */
template<typename T>
class is_stream_insertable {
    template<typename SS, typename TT>
    static auto test(int)
        -> decltype(std::declval<SS&>() << std::declval<TT>(), std::true_type());

    template<typename, typename>
    static auto test(...)->std::false_type;

public:
    static const bool value = decltype(test<std::ostream, const T&>(0))::value;
};
template<typename T>
constexpr bool is_stream_insertable_v = is_stream_insertable<T>::value;



