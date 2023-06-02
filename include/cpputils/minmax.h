#pragma once

#include <utility>

/*

This file provides fixedmin  and fixedmax, both support differently typed arguments.

This solves the problem where you get a compiler error for saying this:

   std::max(v.size(), 123)
or
   std::max(p - q, v.size())

   This would give you a compiler error:

error: no matching function for call to 'min(int, unsigned int)'
note:   deduced conflicting types for parameter 'const _Tp' ('int' and 'unsigned int')

You could pretend the error does not exist, and specify an explicit type, like this:

    std::min<unsigned>(-1, 2U)

This will even result in incorrect code, and return '2U', without the compiler warning for it.
So that is even worse.


So you would always need to pay close attention to the types of the values you are comparing.

A better solution:

   fixedmin(-1, 2U)  ->  -1

See https://godbolt.org/z/ohzc7jGPY for an example.

This table shows how the resulting type is determined:

   A         B          min             max
                          A               A   -- samesize(A,B) && samesignedness(A,B)
   s8        s8          s8              s8      
   u8        u8          u8              u8
  s16       s16         s16             s16
  u16       u16         u16             u16

                          B               B  --  largest_is_signed(A,B)
   s8       s16         s16             s16
   u8       s16         s16             s16
                          A               A  --  largest_is_signed(A,B)
  s16        s8         s16             s16
  s16        u8         s16             s16

                      signedof(A,B)    unsignedof(A,B)    -- sizeof(A)==sizeof(B) && different_signedness(A,B)
   s8        u8          s8              u8
   u8        s8          s8              u8
  s16       u16         s16             u16
  u16       s16         s16             u16

                       smallest(A,B)   largest(A,B)    -- largest_is_unsigned(A,B)
   s8       u16          s8             u16
  u16        s8          s8             u16
   u8       u16          u8             u16
  u16        u8          u8             u16

NOTE:
  gcc just says 'parse error'  on  fn<T>==fn<U>
  clang is more informative: a space is required between a right angle bracket and an equals sign.

 */
template<typename L, typename R>
using maxresult_t = 
    std::conditional_t< (std::is_signed_v<L> == std::is_signed_v<R> && sizeof(L)==sizeof(R)),
        L,  // both the same
        std::conditional_t< (std::is_signed_v<R> && sizeof(L)<sizeof(R)),  // largest type is signed
            R,  // max = largest type
            std::conditional_t< (std::is_signed_v<L> && sizeof(L)>sizeof(R)),  // largest type is signed
                L,  // max = largest type
                std::conditional_t< (std::is_signed_v<L> != std::is_signed_v<R> && sizeof(L)==sizeof(R)),  // different-signedness
                    std::make_unsigned_t<L>,    // use unsigned type
                    std::conditional_t< (std::is_unsigned_v<R> && sizeof(L)<sizeof(R)),  // largest is unsigned
                        R,  // use largest
                        std::conditional_t< (std::is_unsigned_v<L> && sizeof(L)>sizeof(R)),  // largest is unsigned
                            L,  // use largest
                            void   // never happens
                        >
                    >
                >
            >
        >
    >;

template<typename L, typename R>
constexpr bool equal_v = std::is_signed_v<L> == std::is_signed_v<R>; // && sizeof(L)==sizeof(R);

template<typename L, typename R>
using minresult_t =
    std::conditional_t< (std::is_signed_v<L> == std::is_signed_v<R> && sizeof(L)==sizeof(R)),
        L,  // both the same
        std::conditional_t< (std::is_signed_v<R> && sizeof(L)<sizeof(R)),  // largest type is signed
            R,  // min = largest type
            std::conditional_t< (std::is_signed_v<L> && sizeof(L)>sizeof(R)),  // largest type is signed
                L,  // min = largest type
                std::conditional_t< (std::is_signed_v<L> != std::is_signed_v<R> && sizeof(L)==sizeof(R)),  // different-signedness
                    std::make_signed_t<L>,    // use signed type
                    std::conditional_t< (std::is_unsigned_v<R> && sizeof(L)<sizeof(R)),  // largest is unsigned
                        L,  // use smallest
                        std::conditional_t< (std::is_unsigned_v<L> && sizeof(L)>sizeof(R)),  // largest is unsigned
                            R,  // use smallest
                            void   // never happens
                        >
                    >
                >
            >
        >
    >;

template<typename T, typename U>
minresult_t<T,U> fixedmin(T t, U u)
{
    if (std::cmp_less(t, u))
        return t;
    else
        return u;
}

template<typename T, typename U>
maxresult_t<T,U> fixedmax(T t, U u)
{
    if (std::cmp_less(u, t))
        return t;
    else
        return u;
}

