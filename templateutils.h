#pragma once
#include <vector>
#if __cplusplus > 201703L
#include <span>
#endif
#include <string>
#include <string_view>
#include <array>

/**  test if T is a container type */
template<typename T>
struct is_container : std::false_type {};

template<typename T>
struct is_container<std::vector<T> > : std::true_type {};
template<typename T>
struct is_container<const std::vector<T> > : std::true_type {};
#if __cplusplus > 201703L
template<typename T>
struct is_container<std::span<T> > : std::true_type {};
template<typename T>
struct is_container<const std::span<T> > : std::true_type {};
#endif
template<typename T>
struct is_container<std::basic_string<T> > : std::true_type {};
template<typename T>
struct is_container<const std::basic_string<T> > : std::true_type {};

template<typename T>
struct is_container<std::basic_string_view<T> > : std::true_type {};
template<typename T>
struct is_container<const std::basic_string_view<T> > : std::true_type {};

template<typename T, int N>
struct is_container<std::array<T,N> > : std::true_type {};
template<typename T, int N>
struct is_container<const std::array<T,N> > : std::true_type {};

template<typename T>
constexpr bool is_container_v = is_container<T>::value;

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



template<typename T>
struct is_callable_impl {
private:
    typedef char(&yes)[1];
    typedef char(&no)[2];

    struct Fallback { void operator()(); };
    struct Derived : T, Fallback { };

    template<typename U, U> struct Check;

    template<typename>
    static yes test(...);

    template<typename C>
    static no test(Check<void (Fallback::*)(), &C::operator()>*);

public:
    static const bool value = sizeof(test<Derived>(0)) == sizeof(yes);
};
template<typename T>
struct is_callable
    : std::conditional<
        std::is_class<T>::value,
        is_callable_impl<T>,
        std::false_type
    >::type
{ };
template<typename T> constexpr bool is_callable_v = is_callable<T>::value;


template<class T, class R = void>  
struct enable_if_type { typedef R type; };

template<class T, class Enable = void>
struct is_searchable : std::false_type {};
template<class T>
struct is_searchable<T, typename enable_if_type<typename T::key_type>::type> : std::true_type
{};
template<typename T> constexpr bool is_searchable_v = is_searchable<T>::value;


